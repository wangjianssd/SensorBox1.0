
#include <driver.h>
#include <wsnos.h>

#define SDV1  1
#define SDV2  2
#define SDHC  3
#define CS_H()	P9OUT |= BIT0
#define CS_L()	P9OUT &= ~BIT0
#define TRY_TIME      250

typedef struct
{
	uint8_t sd_type;	//SD卡型号
	DSTATUS sd_state;	//SD卡状态
	sd_time_t sd_time;	//SD卡时间
}sd_info_t;

static sd_info_t sd_info;		//SD卡信息

static uint8_t spi_send(uint8_t byte)
{
	osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	while (!(UCB2IFG & UCTXIFG));			// Wait for USCI_B2 TX buffer ready
	UCB2TXBUF = byte;				// Send the header byte
	while (!(UCB2IFG & UCTXIFG));
	while (!(UCB2IFG & UCRXIFG));	// Wait for USCI_B2 RX buffer ready
	UCB2IFG &= ~UCRXIFG;
	OSEL_EXIT_CRITICAL(s);
	return UCB2RXBUF;

//	while (!(UCB2IFG & UCTXIFG));			// Wait for USCI_B2 TX buffer ready
//	UCB2TXBUF = byte;				// Send the header byte
//	while (!(UCB2IFG & UCRXIFG));	// Wait for USCI_B2 RX buffer ready
//
//	return UCB2RXBUF;
}

static uint8_t spi_recv(void)
{
	osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	while (!(UCB2IFG & UCTXIFG));			// Wait for USCI_B2 TX buffer ready
	UCB2TXBUF = 0xFF;				 // Send the header byte
	while (!(UCB2IFG & UCTXIFG));
	while (!(UCB2IFG & UCRXIFG));	// Wait for USCI_B2 RX buffer ready
	UCB2IFG &= ~UCRXIFG;
	OSEL_EXIT_CRITICAL(s);
	return UCB2RXBUF;


//	UCB2TXBUF = 0xFF;				 // Send the header byte
//	while (!(UCB2IFG & UCRXIFG));	// Wait for USCI_B2 RX buffer ready
//	return UCB2RXBUF;
}

static void spi_speed_h()
{
	if(sd_info.sd_type == CT_SD1)
	{
		UCB2BR0 = 0x04;
		UCB2BR1 = 0x00;
	}
	else
	{
		UCB2BR0 = 0x01;
		UCB2BR1 = 0x00;
	}
}

static void spi_speed_l()
{
	UCB2BR0 = 0x80;	 //SD复位为低速
	UCB2BR1 = 0x00;							// fBitClock = fBRCLK/UCBRx = SMCLK/0x80
}

static void spi_init()
{
	UCB2CTL1 |= UCSWRST;					// Put state machine in reset for reconfiguration
	UCB2CTL0 = UCCKPL + UCMSB + UCMST + UCSYNC;
	// The inactive state is High, MSB first, 8-bit data, Master mode, 3-pin SPI, Synchronous mode

	UCB2CTL1 = UCSSEL1 + UCSSEL0;			// Select SMCLK as clock source

	spi_speed_l();

	UCB2CTL1 &= ~UCSWRST;					// Initialize USCI state machine
	UCB2IE &= ~UCRXIE;						// Receive interrupt disabled

	P9SEL &= ~BIT0;
	P9SEL |= BIT1 + BIT2 + BIT3;			// P9.1 P9.2 P9.2 as Peripheral module function, P9.0 as GPIO
	P9DIR &= ~BIT2;
	P9DIR |= BIT0 + BIT1 + BIT3;			// P9.0 P9.1 P9.3 Output, P9.2 Input

	P9REN |= BIT2;
	P9OUT |= BIT2;
}

static uint8_t wait_ready (void)
{
	uint8_t res;
	uint16_t time = TRY_TIME*4;
	osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	spi_speed_l();
	do
	{
		res = spi_recv();
		time--;
	}
	while ((res != 0xFF) && (time > 0));
	spi_speed_h();
	OSEL_EXIT_CRITICAL(s);
	return res;
}

static uint8_t send_cmd(uint8_t cmd, uint32_t arg)
{
	int retry=0;
	uint8_t res = 0x00;
	uint8_t crc = 0x00;
	CS_L();
	if (wait_ready() != 0xFF) return 0xFF;
	if (cmd & 0x80)	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
	{
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	spi_send(cmd);
	spi_send((uint8_t)(arg >> 24));
	spi_send((uint8_t)(arg >> 16));
	spi_send((uint8_t)(arg >> 8));
	spi_send((uint8_t)arg);
	if (cmd == CMD0) crc= 0x95;			   /* CRC for CMD0(0) */
	if (cmd == CMD8) crc= 0x87;			   /* CRC for CMD8(0x1AA) */
	spi_send(crc);

	do
	{
		res=spi_send(0xFF);
		retry++;
	} while ((res==0xff) && (retry<400));
	return  res;
}

static bool_t sd_rest(void)
{
	uint8_t i,time=0;
	uint8_t temp = 0;

	CS_H();
	for (i=0;i<0x0f;i++)  //复位时发送至少74时钟
	{
		spi_send(0xff);	//提高兼容性，如果没有这里，有些SD卡不支持
	}
	CS_L();

	do
	{
		temp = send_cmd(CMD0,0);
		time++;
		if (time == TRY_TIME)
			return(FALSE);//复位失败
	}while (temp !=0x01);//判断SD卡回应是否正确，进入idle
	CS_H();

	spi_send(0xff);	//补偿8时钟

	return TRUE;
}

static void rcvr_spi_m (uint8_t *dst)
{
	*dst = spi_recv();
}

static bool_t rcvr_datablock (
							 uint8_t *buff,			/* Data buffer to store received data */
							 uint btr			 /* Byte count (must be even number) */
							 )
{
	uint8_t token;
	unsigned int time = TRY_TIME;
	do							  /* Wait for data packet in timeout of 100ms */
	{
		token = spi_recv();
	} while ((token == 0xFF) && (time --));
	if (token != 0xFE) return FALSE;	/* If not valid data token, retutn with error */

	do							  /* Receive the data block into buffer */
	{
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 2);
	spi_recv();  spi_recv();						 /* Discard CRC */
	spi_recv();  spi_recv();

	return TRUE;					/* Return with success */
}

static bool_t xmit_datablock (
							 const uint8_t *buff,	  /* 512 byte data block to be transmitted */
							 uint8_t token			   /* Data/Stop token */
							 )
{
	uint8_t resp, wc;
	int i = 40;
	if (wait_ready() != 0xFF) return FALSE;
	spi_send(token);					/* Xmit data token */
	if (token != 0xFD)	  /* Is data token */
	{
		wc = 0;
		do							  /* Xmit the 512 byte data block to MMC */
		{
			spi_send(*buff++);
			spi_send(*buff++);
		} while (--wc);

		spi_send(0xFF);					   /* CRC (Dummy) */
		spi_send(0xFF);
		do
		{
			resp = spi_recv();
		}while (((resp & 0x1F) != 0x05)&&i--);

		if ((resp & 0x1F) != 0x05)//                /* Reveive data response */
		{
			resp = spi_recv();resp = spi_recv();
			return FALSE;
		}		 /* If not accepted, return with error */
	}

	return TRUE;
}

DSTATUS disk_status (uint8_t pdrv)
{
	if (pdrv) return STA_NOINIT;		   /* Supports only single drive */
	return sd_info.sd_state;
}

DRESULT disk_write(                  uint8_t pdrv,		   /* Physical drive nmuber (0..) */
								   const uint8_t *buff,   /* Data to be written */
								   uint32_t sector,	   /* Sector address (LBA) */
								   uint8_t count		   /* Number of sectors to write (1..128) */)
{
	if (pdrv || !count)	  return RES_PARERR;
	if (sd_info.sd_state & STA_NOINIT) return RES_NOTRDY;
	if (sd_info.sd_state & STA_PROTECT)	  return RES_WRPRT;

	if (!(sd_info.sd_type & CT_SD2)) sector = sector<< 9;	 /* Convert to byte address if needed */

	if (count == 1)		 /* Single block write */
	{
		if ((send_cmd(CMD24, sector) == 0)	  /* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else			  /* Multiple block write */
	{
		if (sd_info.sd_type & 2)
		{
			send_cmd(CMD55, 0); send_cmd(CMD23, count);	   /* ACMD23 */
		}
		if (send_cmd(CMD25, sector) == 0)	 /* WRITE_MULTIPLE_BLOCK */
		{
			do
			{
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	 /* STOP_TRAN token */
				count = 1;
		}
	}

	CS_H();				 /* CS = H */

	return count ? RES_ERROR : RES_OK;
}

DRESULT disk_read (
				uint8_t pdrv,		 /* Physical drive nmuber (0..) */
				uint8_t *buff,		 /* Data buffer to store read data */
				uint32_t sector,	 /* Sector address (LBA) */
				uint8_t count		 /* Number of sectors to read (1..128) */
				)
{
	if (pdrv || !count)	  return RES_PARERR;
	if (sd_info.sd_state & STA_NOINIT) return RES_NOTRDY;

	if (!(sd_info.sd_type & CT_SD2)) sector *= 512;	 /* Convert to byte address if needed */
	if (count == 1)		 /* Single block read */
	{
		if ((send_cmd(CMD17, sector) == 0)	  /* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else				  /* Multiple block read */
	{
		if (send_cmd(CMD18, sector) == 0)	 /* READ_MULTIPLE_BLOCK */
		{
			do
			{
				if (!rcvr_datablock(buff, 512))	break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				   /* STOP_TRANSMISSION */
		}
	}
	CS_H();
	return count ? RES_ERROR : RES_OK;
}

DRESULT disk_ioctl (
				   uint8_t pdrv,	   /* Physical drive nmuber (0..) */
				   uint8_t cmd,	   /* Control code */
				   void *buff	   /* Buffer to send/receive control data */
				   )
{
	DRESULT res;
	uint8_t n, csd[16], *ptr = buff;
	uint16_t csize;


	if (pdrv) return RES_PARERR;

	res = RES_ERROR;

	if (cmd == CTRL_POWER)
	{
		switch (*ptr)
		{
		case 0:		   /* Sub control code == 0 (POWER_OFF) */
			res = RES_OK;
			break;
		case 1:		   /* Sub control code == 1 (POWER_ON) */
			res = RES_OK;
			break;
		case 2:		   /* Sub control code == 2 (POWER_GET) */
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	} else
	{
		if (sd_info.sd_state & STA_NOINIT) return RES_NOTRDY;

		switch (cmd)
		{
		case GET_SECTOR_COUNT :	   /* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
			{
				if ((csd[0] >> 6) == 1)	   /* SDC ver 2.00 */
				{
					csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
					*(uint32_t*)buff = (uint32_t)csize << 10;
				} else					  /* MMC or SDC ver 1.XX */
				{
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
					*(uint32_t*)buff = (uint32_t)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	  /* Get sectors on the disk (WORD) */
			*(uint16_t*)buff = 512;
			res = RES_OK;
			break;

		case CTRL_SYNC :	/* Make sure that data has been written */
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case MMC_GET_CSD :	  /* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		  /* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :	  /* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		   /* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :	  /* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0)	/* READ_OCR */
			{
				for (n = 0; n < 4; n++)
					*ptr++ = spi_recv();
				res = RES_OK;
			}

//        case MMC_GET_TYPE :    /* Get card type flags (1 byte) */
//            *ptr = CardType;
//            res = RES_OK;
//            break;

		default:
			res = RES_PARERR;
		}
		CS_H();
	}

	return res;
}

DSTATUS disk_initialize (uint8_t pdrv)
{
	int i=0;
	uint8_t ty = 0x00;
	uint8_t time =TRY_TIME;
	if (pdrv) return STA_NOINIT;
	if (sd_info.sd_state & STA_NODISK) return sd_info.sd_state;	   /* No card in the socket */
	uint8_t ocr[4];
	CS_H();
	for (i=0; i<0x08; i++)
	{
		spi_send(0xff);
	}

	if (send_cmd(CMD0,0)==1)
	{
		if (send_cmd(CMD1,0)==1)
		{
			if (send_cmd(CMD8, 0X1AA)==1)
			{
				for (i=0;i<4;i++)
				{
					ocr[i]=spi_recv();
				}
				if (ocr[2] == 0x01 && ocr[3] == 0xAA) /* The card can work at vdd range of 2.7-3.6V */
				{
					do
					{
						if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0)	   break;
					} while (time--);

					if ((time--) && send_cmd(CMD58, 0) == 0)	/* Check CCS bit */
					{
						for (i = 0; i < 4; i++)	ocr[i] = spi_recv();
						ty = (ocr[0] & 0x40) ? CT_SDHC : CT_SD1;
					}
				}
			} else
			{
				ty = (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) <= 1) ? CT_SD1 : CT_MMC;	  /* SDC : MMC */
				do
				{
					if (ty == CT_SD1)
					{
						if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0)	break;	  /* ACMD41 */
					} else
					{
						if (send_cmd(CMD1, 0) == 0)	break;								  /* CMD1 */
					}
				} while (time --);
				if ((time--) || send_cmd(CMD16, 512) != 0)	  /* Select R/W block length */
					ty = 0;
			}
		}

	}
	sd_info.sd_type = ty;
	CS_H();
	if (ty)
	{
		sd_info.sd_state &= ~STA_NOINIT;		/* Clear STA_NOINIT */
	}

	return sd_info.sd_state;
}

uint32_t get_fattime (void)
{
	return(((uint32_t)(sd_info.sd_time.year)-1980) << 25)
	| ((uint32_t)sd_info.sd_time.month << 21)
	| ((uint32_t)sd_info.sd_time.day << 16)
	| ((uint32_t)sd_info.sd_time.hour << 11)
	| ((uint32_t)sd_info.sd_time.min << 5)
	| ((uint32_t)sd_info.sd_time.sec >> 1)
	;
}

void set_fattime (sd_time_t *timestamp)
{
	sd_info.sd_time.year=timestamp->year;
	sd_info.sd_time.month=timestamp->month;
	sd_info.sd_time.day=timestamp->day;
	sd_info.sd_time.hour=timestamp->hour;
	sd_info.sd_time.min=timestamp->min;
	sd_info.sd_time.sec=timestamp->sec;
}

bool_t sd_init()
{
	sd_info.sd_state = STA_NOINIT;
	spi_init();
	if (!sd_rest())
	{
		return FALSE;
	}
	spi_speed_h();
	sd_time_t disk_time={2000,1,1,0,0,0};
	set_fattime(&disk_time);
	return TRUE;
}
