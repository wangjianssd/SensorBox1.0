/**
 * @brief       : this
 * @file        : flash.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-15
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-15  v0.0.1  gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"


void flash_write(uint8_t *flash_ptr,
                     const uint8_t *buffer,
                     uint16_t len)
{
	uint16_t i = 0;
	uint32_t uint_ptr =  (uint32_t)flash_ptr;
	uint8_t s;

	OSEL_ENTER_CRITICAL(s);
	if (uint_ptr == INFO_A_ADDR)
	{
		if (FCTL3 & LOCKA)   /* Test LOCKA, locked? */
		{
			FCTL3 = FWKEY + LOCKA;	/* unlock SegmentA	*/
		}
		else
		{
			FCTL3 = FWKEY;
		}
	}
	else
	{
		FCTL3 = FWKEY;                   /* Clear Lock bit */
	}

	FCTL1 = FWKEY + WRT;
	for (i = 0; i < len; i++)
	{
		*flash_ptr++ = *buffer++;    /* Write value to flash */
	}
	FCTL1 = FWKEY;                    /* Clear WRT bit  */

	if (uint_ptr == INFO_A_ADDR)
	{
		if (!(FCTL3 & LOCKA))   /* Test LOCKA, unlocked? */
		{
			FCTL3 = FWKEY + LOCKA + LOCK; /* lock SegmentA */
			return;
		}
	}
	FCTL3 = FWKEY + LOCK;
	OSEL_EXIT_CRITICAL(s);
}

void flash_read(const uint8_t *flash_ptr,
                    uint8_t *buffer,
                    uint16_t len)
{
	uint8_t i = 0;

	uint8_t s;
	OSEL_ENTER_CRITICAL(s);

	for (i = 0; i < len; i++)
	{
		*buffer++ = *flash_ptr++;
	}

	OSEL_EXIT_CRITICAL(s);
}

void flash_erase(uint8_t *const flash_ptr,
                     uint8_t erase_type)
{
	uint8_t s;
	OSEL_ENTER_CRITICAL(s);

	uint32_t uint_ptr =  (uint32_t)flash_ptr;

	if (uint_ptr == INFO_A_ADDR)
	{
		if (FCTL3 & LOCKA)   /* Test LOCKA, locked? */
		{
			FCTL3 = FWKEY + LOCKA;	/* unlock SegmentA	*/
		}
		else
		{
			FCTL3 = FWKEY;
		}
	}
	else
	{
		FCTL3 = FWKEY;
	}

	if (erase_type == FLASH_SEG_ERASE)
	{
		FCTL1 = FWKEY + ERASE;
	}

	if (erase_type == FLASH_BK_ERASE)
	{
		FCTL1 = FWKEY + MERAS;
	}

	*flash_ptr = 0;
	while (FCTL3 & BUSY);
	FCTL1 = FWKEY;
	if (uint_ptr == INFO_A_ADDR)
	{
		if (!(FCTL3 & LOCKA))   /* Test LOCKA, unlocked? */
		{
			FCTL3 = FWKEY + LOCKA + LOCK; /* lock SegmentA */
			OSEL_EXIT_CRITICAL(s);
			return;
		}
	}
	FCTL3 = FWKEY + LOCK;

	OSEL_EXIT_CRITICAL(s);
}






