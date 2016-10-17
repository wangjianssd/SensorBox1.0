#ifndef __AD5252_H__
#define __AD5252_H__

#include "data_type_def.h"

#define AB_RES              (100*1000.0)

#define VC_VALUE            (1)  // 1v
#define R169                (200*1000.0)
#define R171                (8.2*1000.0)

#define CONST_RES_OFFSET    (75)     
#define DIV_CONST           (256)

#define RES1_REG             (REG_EN | RDAC_EN | REG_RDAC1)
#define RES1_EEPROM          (REG_EN | EEPROM_EN | REG_RDAC1)

#define RES1_SOTRE           (CMD_EN | QUICK_STORE | REG_RDAC1)
#define RES1_RELOAD          (CMD_EN | QUICK_RELOAD | REG_RDAC1)
#define RES1_DEC             (CMD_EN | QUICK_RES1_DEC | REG_RDAC1)
#define RES1_INC             (CMD_EN | QUICK_RES1_INC | REG_RDAC1)
#define RES_RESET            (CMD_EN | QUICK_RES1_INC)

/* I2C address of the device */

#define CMD_EN              (0x01<<7)
#define REG_EN              (0x00<<7)

#define EEPROM_EN           (0x01<<5)
#define RDAC_EN             (0x00<<5)

#define REG_RDAC1           (0x01)      // 输出
#define REG_RDAC3           (0x03)

#define QUICK_STORE         (0x02<<3)
#define QUICK_RELOAD        (0x01<<3)
#define QUICK_RES1_DEC      (0x05<<3)
#define QUICK_RES1_INC      (0x0a<<3)
#define QUICK_RES_RESET     (0x07<<3)

#define RDAC1_INTERGER      (0x1A)
#define RDAC1_DECIMAL       (0x1B)


/*****************************************************************************/
/* AD5252                                                                    */
/*****************************************************************************/
#define AD5252_ADDRESS         (0x2D)   //AD5252的地址,AD1=0,AD0=1=>0101 101   

/* Instruction byte */
#define AD5252_CMD_REG       (1 << 7)
#define AD5252_EE_RDAC       (1 << 5)

/* Addresses for Writing Data Byte Contents to RDAC Registers */
#define AD5252_RDAC1         0x01 // For quad-channel device software 
#define AD5252_RDAC3         0x03 // compatibility, the dual potentiometers in 
                                  // the parts are designated as RDAC1 and RDAC3
/* EEMEM Registers */
#define AD5252_EEMEM1        0x01 // Store RDAC1 setting to EEMEM1
#define AD5252_EEMEM3        0x03 // Store RDAC3 setting to EEMEM3
#define AD5252_EEMEM4        0x04 // Store user data to EEMEM4
#define AD5252_EEMEM5        0x05 // Store user data to EEMEM5
#define AD5252_EEMEM6        0x06 // Store user data to EEMEM6
#define AD5252_EEMEM7        0x07 // Store user data to EEMEM7
#define AD5252_EEMEM8        0x08 // Store user data to EEMEM8
#define AD5252_EEMEM9        0x09 // Store user data to EEMEM9
#define AD5252_EEMEM10       0x0A // Store user data to EEMEM10
#define AD5252_EEMEM11       0x0B // Store user data to EEMEM11
#define AD5252_EEMEM12       0x0C // Store user data to EEMEM12
#define AD5252_EEMEM13       0x0D // Store user data to EEMEM13
#define AD5252_EEMEM14       0x0E // Store user data to EEMEM14
#define AD5252_EEMEM15       0x0F // Store user data to EEMEM15

/* Intructions */
#define AD5252_NOP			 0x00 // NOP
#define AD5252_RESTORE       0x01 // Restore EEMEM (A1, A0) to RDAC (A1, A0)
#define AD5252_STORE         0x02 // Store RDAC (A1, A0) to EEMEM (A1, A0)
#define AD5252_6DBDOWN	     0x03 // Decrement RDAC (A1, A0) 6 dB
#define AD5252_6DBDOWNALL	 0x04 // Decrement all RDACs 6 dB
#define AD5252_1STEPDOWN	 0x05 // Decrement RDAC (A1, A0) one step
#define AD5252_1STEPDOWNALL	 0x06 // Decrement all RDACs one step
#define AD5252_RESETALL		 0x07 // Reset: restore EEMEMs to all RDACs
#define AD5252_6DBUP	     0x08 // Increment RDACs (A1, A0) 6 dB
#define AD5252_6DBUPALL      0x09 // Increment all RDACs 6 dB
#define AD5252_1STEPUP       0x0A // Increment RDACs (A1, A0) one step
#define AD5252_1STEPUPALL    0x0B // Increment all RDACs one step
#define AD5252_INSTRUCTION(x) (((x) & 0x0F) << 3)

#define AD5252_TOLR1INT      0x1A // Sign,7bit integer values of RDAC1 tolerance
#define AD5252_TOLR1DEC      0x1B // 8-bit decimal value of RDAC1 tolerance
#define AD5252_TOLR2INT      0x1E // Sign,7bit integer values of RDAC3 tolerance
#define AD5252_TOLR2DEC      0x1F // 8-bit decimal value of RDAC3 tolerance

/**
* @brief 
*/
uint8_t ad5252_init(void);

void vout_set(uint8_t vccout);
#endif
