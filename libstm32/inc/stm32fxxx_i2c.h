/*
 * stm32fxxx_i2c.h
 *
 *  Created on: 21 paź 2013
 *      Author: lucck
 */

#ifndef STM32FXXX_I2C_H_
#define STM32FXXX_I2C_H_



/** @defgroup I2C_mode
  * @{
  */

#define I2C_Mode_I2C                    ((uint16_t)0x0000)
#define I2C_Mode_SMBusDevice            ((uint16_t)0x0002)
#define I2C_Mode_SMBusHost              ((uint16_t)0x000A)

/**
  * @}
  */

/** @defgroup I2C_duty_cycle_in_fast_mode
  * @{
  */

#define I2C_DutyCycle_16_9              ((uint16_t)0x4000) /*!< I2C fast mode Tlow/Thigh = 16/9 */
#define I2C_DutyCycle_2                 ((uint16_t)0xBFFF) /*!< I2C fast mode Tlow/Thigh = 2 */

/**
  * @}
  */

/** @defgroup I2C_acknowledgement
  * @{
  */

#define I2C_Ack_Enable                  ((uint16_t)0x0400)
#define I2C_Ack_Disable                 ((uint16_t)0x0000)

/**
  * @}
  */

/** @defgroup I2C_transfer_direction
  * @{
  */

#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)

/**
  * @}
  */

/** @defgroup I2C_acknowledged_address
  * @{
  */

#define I2C_AcknowledgedAddress_7bit    ((uint16_t)0x4000)
#define I2C_AcknowledgedAddress_10bit   ((uint16_t)0xC000)

/**
  * @}
  */

/** @defgroup I2C_registers
  * @{
  */

#define I2C_Register_CR1                ((uint8_t)0x00)
#define I2C_Register_CR2                ((uint8_t)0x04)
#define I2C_Register_OAR1               ((uint8_t)0x08)
#define I2C_Register_OAR2               ((uint8_t)0x0C)
#define I2C_Register_DR                 ((uint8_t)0x10)
#define I2C_Register_SR1                ((uint8_t)0x14)
#define I2C_Register_SR2                ((uint8_t)0x18)
#define I2C_Register_CCR                ((uint8_t)0x1C)
#define I2C_Register_TRISE              ((uint8_t)0x20)

/**
  * @}
  */

/** @defgroup I2C_NACK_position
  * @{
  */

#define I2C_NACKPosition_Next           ((uint16_t)0x0800)
#define I2C_NACKPosition_Current        ((uint16_t)0xF7FF)

/**
  * @}
  */

/** @defgroup I2C_SMBus_alert_pin_level
  * @{
  */

#define I2C_SMBusAlert_Low              ((uint16_t)0x2000)
#define I2C_SMBusAlert_High             ((uint16_t)0xDFFF)

/**
  * @}
  */

/** @defgroup I2C_PEC_position
  * @{
  */

#define I2C_PECPosition_Next            ((uint16_t)0x0800)
#define I2C_PECPosition_Current         ((uint16_t)0xF7FF)

/**
  * @}
  */

/** @defgroup I2C_interrupts_definition
  * @{
  */

#define I2C_IT_BUF                      ((uint16_t)0x0400)
#define I2C_IT_EVT                      ((uint16_t)0x0200)
#define I2C_IT_ERR                      ((uint16_t)0x0100)

/**
  * @}
  */

/** @defgroup I2C_interrupts_definition
  * @{
  */

#define I2C_IT_SMBALERT                 ((uint32_t)0x01008000)
#define I2C_IT_TIMEOUT                  ((uint32_t)0x01004000)
#define I2C_IT_PECERR                   ((uint32_t)0x01001000)
#define I2C_IT_OVR                      ((uint32_t)0x01000800)
#define I2C_IT_AF                       ((uint32_t)0x01000400)
#define I2C_IT_ARLO                     ((uint32_t)0x01000200)
#define I2C_IT_BERR                     ((uint32_t)0x01000100)
#define I2C_IT_TXE                      ((uint32_t)0x06000080)
#define I2C_IT_RXNE                     ((uint32_t)0x06000040)
#define I2C_IT_STOPF                    ((uint32_t)0x02000010)
#define I2C_IT_ADD10                    ((uint32_t)0x02000008)
#define I2C_IT_BTF                      ((uint32_t)0x02000004)
#define I2C_IT_ADDR                     ((uint32_t)0x02000002)
#define I2C_IT_SB                       ((uint32_t)0x02000001)


/** @defgroup I2C_flags_definition
  * @{
  */

/**
  * @brief  SR2 register flags
  */

#define I2C_FLAG_DUALF                  ((uint32_t)0x00800000)
#define I2C_FLAG_SMBHOST                ((uint32_t)0x00400000)
#define I2C_FLAG_SMBDEFAULT             ((uint32_t)0x00200000)
#define I2C_FLAG_GENCALL                ((uint32_t)0x00100000)
#define I2C_FLAG_TRA                    ((uint32_t)0x00040000)
#define I2C_FLAG_BUSY                   ((uint32_t)0x00020000)
#define I2C_FLAG_MSL                    ((uint32_t)0x00010000)

/**
  * @brief  SR1 register flags
  */

#define I2C_FLAG_SMBALERT               ((uint32_t)0x10008000)
#define I2C_FLAG_TIMEOUT                ((uint32_t)0x10004000)
#define I2C_FLAG_PECERR                 ((uint32_t)0x10001000)
#define I2C_FLAG_OVR                    ((uint32_t)0x10000800)
#define I2C_FLAG_AF                     ((uint32_t)0x10000400)
#define I2C_FLAG_ARLO                   ((uint32_t)0x10000200)
#define I2C_FLAG_BERR                   ((uint32_t)0x10000100)
#define I2C_FLAG_TXE                    ((uint32_t)0x10000080)
#define I2C_FLAG_RXNE                   ((uint32_t)0x10000040)
#define I2C_FLAG_STOPF                  ((uint32_t)0x10000010)
#define I2C_FLAG_ADD10                  ((uint32_t)0x10000008)
#define I2C_FLAG_BTF                    ((uint32_t)0x10000004)
#define I2C_FLAG_ADDR                   ((uint32_t)0x10000002)
#define I2C_FLAG_SB                     ((uint32_t)0x10000001)

#define IS_I2C_CLEAR_FLAG(FLAG) ((((FLAG) & (uint16_t)0x20FF) == 0x00) && ((FLAG) != (uint16_t)0x00))


/** @defgroup I2C_Events
  * @{
  */

/**
 ===============================================================================
               I2C Master Events (Events grouped in order of communication)
 ===============================================================================
 */

/**
  * @brief  Communication start
  *
  * After sending the START condition (I2C_GenerateSTART() function) the master
  * has to wait for this event. It means that the Start condition has been correctly
  * released on the I2C bus (the bus is free, no other devices is communicating).
  *
  */
/* --EV5 */
#define  I2C_EVENT_MASTER_MODE_SELECT                      ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */

/**
  * @brief  Address Acknowledge
  *
  * After checking on EV5 (start condition correctly released on the bus), the
  * master sends the address of the slave(s) with which it will communicate
  * (I2C_Send7bitAddress() function, it also determines the direction of the communication:
  * Master transmitter or Receiver). Then the master has to wait that a slave acknowledges
  * his address. If an acknowledge is sent on the bus, one of the following events will
  * be set:
  *
  *  1) In case of Master Receiver (7-bit addressing): the I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED
  *     event is set.
  *
  *  2) In case of Master Transmitter (7-bit addressing): the I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED
  *     is set
  *
  *  3) In case of 10-Bit addressing mode, the master (just after generating the START
  *  and checking on EV5) has to send the header of 10-bit addressing mode (I2C_SendData()
  *  function). Then master should wait on EV9. It means that the 10-bit addressing
  *  header has been correctly sent on the bus. Then master should send the second part of
  *  the 10-bit address (LSB) using the function I2C_Send7bitAddress(). Then master
  *  should wait for event EV6.
  *
  */

/* --EV6 */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */
/* --EV9 */
#define  I2C_EVENT_MASTER_MODE_ADDRESS10                   ((uint32_t)0x00030008)  /* BUSY, MSL and ADD10 flags */

/**
  * @brief Communication events
  *
  * If a communication is established (START condition generated and slave address
  * acknowledged) then the master has to check on one of the following events for
  * communication procedures:
  *
  * 1) Master Receiver mode: The master has to wait on the event EV7 then to read
  *    the data received from the slave (I2C_ReceiveData() function).
  *
  * 2) Master Transmitter mode: The master has to send data (I2C_SendData()
  *    function) then to wait on event EV8 or EV8_2.
  *    These two events are similar:
  *     - EV8 means that the data has been written in the data register and is
  *       being shifted out.
  *     - EV8_2 means that the data has been physically shifted out and output
  *       on the bus.
  *     In most cases, using EV8 is sufficient for the application.
  *     Using EV8_2 leads to a slower communication but ensure more reliable test.
  *     EV8_2 is also more suitable than EV8 for testing on the last data transmission
  *     (before Stop condition generation).
  *
  *  @note In case the  user software does not guarantee that this event EV7 is
  *        managed before the current byte end of transfer, then user may check on EV7
  *        and BTF flag at the same time (ie. (I2C_EVENT_MASTER_BYTE_RECEIVED | I2C_FLAG_BTF)).
  *        In this case the communication may be slower.
  *
  */

/* Master RECEIVER mode -----------------------------*/
/* --EV7 */
#define  I2C_EVENT_MASTER_BYTE_RECEIVED                    ((uint32_t)0x00030040)  /* BUSY, MSL and RXNE flags */

/* Master TRANSMITTER mode --------------------------*/
/* --EV8 */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING                 ((uint32_t)0x00070080) /* TRA, BUSY, MSL, TXE flags */
/* --EV8_2 */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED                 ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */


/**
 ===============================================================================
               I2C Slave Events (Events grouped in order of communication)
 ===============================================================================
 */


/**
  * @brief  Communication start events
  *
  * Wait on one of these events at the start of the communication. It means that
  * the I2C peripheral detected a Start condition on the bus (generated by master
  * device) followed by the peripheral address. The peripheral generates an ACK
  * condition on the bus (if the acknowledge feature is enabled through function
  * I2C_AcknowledgeConfig()) and the events listed above are set :
  *
  * 1) In normal case (only one address managed by the slave), when the address
  *   sent by the master matches the own address of the peripheral (configured by
  *   I2C_OwnAddress1 field) the I2C_EVENT_SLAVE_XXX_ADDRESS_MATCHED event is set
  *   (where XXX could be TRANSMITTER or RECEIVER).
  *
  * 2) In case the address sent by the master matches the second address of the
  *   peripheral (configured by the function I2C_OwnAddress2Config() and enabled
  *   by the function I2C_DualAddressCmd()) the events I2C_EVENT_SLAVE_XXX_SECONDADDRESS_MATCHED
  *   (where XXX could be TRANSMITTER or RECEIVER) are set.
  *
  * 3) In case the address sent by the master is General Call (address 0x00) and
  *   if the General Call is enabled for the peripheral (using function I2C_GeneralCallCmd())
  *   the following event is set I2C_EVENT_SLAVE_GENERALCALLADDRESS_MATCHED.
  *
  */

/* --EV1  (all the events below are variants of EV1) */
/* 1) Case of One Single Address managed by the slave */
#define  I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED          ((uint32_t)0x00020002) /* BUSY and ADDR flags */
#define  I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED       ((uint32_t)0x00060082) /* TRA, BUSY, TXE and ADDR flags */

/* 2) Case of Dual address managed by the slave */
#define  I2C_EVENT_SLAVE_RECEIVER_SECONDADDRESS_MATCHED    ((uint32_t)0x00820000)  /* DUALF and BUSY flags */
#define  I2C_EVENT_SLAVE_TRANSMITTER_SECONDADDRESS_MATCHED ((uint32_t)0x00860080)  /* DUALF, TRA, BUSY and TXE flags */

/* 3) Case of General Call enabled for the slave */
#define  I2C_EVENT_SLAVE_GENERALCALLADDRESS_MATCHED        ((uint32_t)0x00120000)  /* GENCALL and BUSY flags */

/**
  * @brief  Communication events
  *
  * Wait on one of these events when EV1 has already been checked and:
  *
  * - Slave RECEIVER mode:
  *     - EV2: When the application is expecting a data byte to be received.
  *     - EV4: When the application is expecting the end of the communication: master
  *       sends a stop condition and data transmission is stopped.
  *
  * - Slave Transmitter mode:
  *    - EV3: When a byte has been transmitted by the slave and the application is expecting
  *      the end of the byte transmission. The two events I2C_EVENT_SLAVE_BYTE_TRANSMITTED and
  *      I2C_EVENT_SLAVE_BYTE_TRANSMITTING are similar. The second one can optionally be
  *      used when the user software doesn't guarantee the EV3 is managed before the
  *      current byte end of transfer.
  *    - EV3_2: When the master sends a NACK in order to tell slave that data transmission
  *      shall end (before sending the STOP condition). In this case slave has to stop sending
  *      data bytes and expect a Stop condition on the bus.
  *
  *  @note In case the  user software does not guarantee that the event EV2 is
  *        managed before the current byte end of transfer, then user may check on EV2
  *        and BTF flag at the same time (ie. (I2C_EVENT_SLAVE_BYTE_RECEIVED | I2C_FLAG_BTF)).
  *        In this case the communication may be slower.
  *
  */

/* Slave RECEIVER mode --------------------------*/
/* --EV2 */
#define  I2C_EVENT_SLAVE_BYTE_RECEIVED                     ((uint32_t)0x00020040)  /* BUSY and RXNE flags */
/* --EV4  */
#define  I2C_EVENT_SLAVE_STOP_DETECTED                     ((uint32_t)0x00000010)  /* STOPF flag */

/* Slave TRANSMITTER mode -----------------------*/
/* --EV3 */
#define  I2C_EVENT_SLAVE_BYTE_TRANSMITTED                  ((uint32_t)0x00060084)  /* TRA, BUSY, TXE and BTF flags */
#define  I2C_EVENT_SLAVE_BYTE_TRANSMITTING                 ((uint32_t)0x00060080)  /* TRA, BUSY and TXE flags */
/* --EV3_2 */
#define  I2C_EVENT_SLAVE_ACK_FAILURE                       ((uint32_t)0x00000400)  /* AF flag */



#endif /* STM32FXXX_I2C_H_ */
