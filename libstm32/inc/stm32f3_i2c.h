
#pragma once



/** @defgroup I2C_Exported_Constants
  * @{
  */


/** @defgroup I2C_Analog_Filter 
  * @{
  */

#define I2C_AnalogFilter_Enable         ((uint32_t)0x00000000)
#define I2C_AnalogFilter_Disable        I2C_CR1_ANFOFF

     

#define I2C_Mode_I2C                    ((uint32_t)0x00000000)
#define I2C_Mode_SMBusDevice            I2C_CR1_SMBDEN
#define I2C_Mode_SMBusHost              I2C_CR1_SMBHEN


/** @defgroup I2C_acknowledgement
  * @{
  */

#define I2C_Ack_Enable                  ((uint32_t)0x00000000)
#define I2C_Ack_Disable                 I2C_CR2_NACK


/** @defgroup I2C_acknowledged_address
  * @{
  */

#define I2C_AcknowledgedAddress_7bit    ((uint32_t)0x00000000)
#define I2C_AcknowledgedAddress_10bit   I2C_OAR1_OA1MODE



/** @defgroup I2C_transfer_direction 
  * @{
  */

#define I2C_Direction_Transmitter       ((uint16_t)0x0000)
#define I2C_Direction_Receiver          ((uint16_t)0x0400)


/** @defgroup I2C_DMA_transfer_requests 
  * @{
  */

#define I2C_DMAReq_Tx                   I2C_CR1_TXDMAEN
#define I2C_DMAReq_Rx                   I2C_CR1_RXDMAEN


/** @defgroup I2C_slave_address
  * @{
  */


/** @defgroup I2C_own_address2_mask
  * @{
  */

#define I2C_OA2_NoMask                  ((uint8_t)0x00)
#define I2C_OA2_Mask01                  ((uint8_t)0x01)                 
#define I2C_OA2_Mask02                  ((uint8_t)0x02)
#define I2C_OA2_Mask03                  ((uint8_t)0x03)
#define I2C_OA2_Mask04                  ((uint8_t)0x04)
#define I2C_OA2_Mask05                  ((uint8_t)0x05)
#define I2C_OA2_Mask06                  ((uint8_t)0x06)
#define I2C_OA2_Mask07                  ((uint8_t)0x07)

/** @defgroup I2C_registers 
  * @{
  */

#define I2C_Register_CR1                ((uint8_t)0x00)
#define I2C_Register_CR2                ((uint8_t)0x04)
#define I2C_Register_OAR1               ((uint8_t)0x08)
#define I2C_Register_OAR2               ((uint8_t)0x0C)
#define I2C_Register_TIMINGR            ((uint8_t)0x10)
#define I2C_Register_TIMEOUTR           ((uint8_t)0x14)
#define I2C_Register_ISR                ((uint8_t)0x18)
#define I2C_Register_ICR                ((uint8_t)0x1C)
#define I2C_Register_PECR               ((uint8_t)0x20)
#define I2C_Register_RXDR               ((uint8_t)0x24)
#define I2C_Register_TXDR               ((uint8_t)0x28)


/** @defgroup I2C_interrupts_definition 
  * @{
  */

#define I2C_IT_ERRI                     I2C_CR1_ERRIE
#define I2C_IT_TCI                      I2C_CR1_TCIE
#define I2C_IT_STOPI                    I2C_CR1_STOPIE
#define I2C_IT_NACKI                    I2C_CR1_NACKIE
#define I2C_IT_ADDRI                    I2C_CR1_ADDRIE
#define I2C_IT_RXI                      I2C_CR1_RXIE
#define I2C_IT_TXI                      I2C_CR1_TXIE


/** @defgroup I2C_flags_definition 
  * @{
  */

#define  I2C_FLAG_TXE                   I2C_ISR_TXE
#define  I2C_FLAG_TXIS                  I2C_ISR_TXIS
#define  I2C_FLAG_RXNE                  I2C_ISR_RXNE
#define  I2C_FLAG_ADDR                  I2C_ISR_ADDR
#define  I2C_FLAG_NACKF                 I2C_ISR_NACKF
#define  I2C_FLAG_STOPF                 I2C_ISR_STOPF
#define  I2C_FLAG_TC                    I2C_ISR_TC
#define  I2C_FLAG_TCR                   I2C_ISR_TCR
#define  I2C_FLAG_BERR                  I2C_ISR_BERR
#define  I2C_FLAG_ARLO                  I2C_ISR_ARLO
#define  I2C_FLAG_OVR                   I2C_ISR_OVR
#define  I2C_FLAG_PECERR                I2C_ISR_PECERR
#define  I2C_FLAG_TIMEOUT               I2C_ISR_TIMEOUT
#define  I2C_FLAG_ALERT                 I2C_ISR_ALERT
#define  I2C_FLAG_BUSY                  I2C_ISR_BUSY


/** @defgroup I2C_interrupts_definition 
  * @{
  */

#define  I2C_IT_TXIS                    I2C_ISR_TXIS
#define  I2C_IT_RXNE                    I2C_ISR_RXNE
#define  I2C_IT_ADDR                    I2C_ISR_ADDR
#define  I2C_IT_NACKF                   I2C_ISR_NACKF
#define  I2C_IT_STOPF                   I2C_ISR_STOPF
#define  I2C_IT_TC                      I2C_ISR_TC
#define  I2C_IT_TCR                     I2C_ISR_TCR
#define  I2C_IT_BERR                    I2C_ISR_BERR
#define  I2C_IT_ARLO                    I2C_ISR_ARLO
#define  I2C_IT_OVR                     I2C_ISR_OVR
#define  I2C_IT_PECERR                  I2C_ISR_PECERR
#define  I2C_IT_TIMEOUT                 I2C_ISR_TIMEOUT
#define  I2C_IT_ALERT                   I2C_ISR_ALERT


#define  I2C_Reload_Mode                I2C_CR2_RELOAD
#define  I2C_AutoEnd_Mode               I2C_CR2_AUTOEND
#define  I2C_SoftEnd_Mode               ((uint32_t)0x00000000)

                              

/** @defgroup I2C_StartStopMode_definition 
  * @{
  */

#define  I2C_No_StartStop                 ((uint32_t)0x00000000)
#define  I2C_Generate_Stop                I2C_CR2_STOP
#define  I2C_Generate_Start_Read          (uint32_t)(I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_Generate_Start_Write         I2C_CR2_START

                              

