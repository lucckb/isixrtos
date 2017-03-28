/*
 * config.h
 *
 *  Created on: 4 gru 2013
 *      Author: lucck
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/* Enable shoutdown API */
#define CONFIG_ISIX_SHUTDOWN_API
/* Enable task stack checking */
#define CONFIG_ISIX_TASK_STACK_CHECK
#define CONFIG_ISIX_FIFO_EVENT_NOTIFY

#if defined(STM32MCU_MAJOR_TYPE_F1)
			#define CONFIG_XTAL_HZ  25000000U
			#define CONFIG_HCLK_HZ  75000000U
			#define CONFIG_PCLK1_HZ (CONFIG_HCLK_HZ/4)
			#define CONFIG_PCLK2_HZ (CONFIG_HCLK_HZ/2)
#elif defined(STM32MCU_MAJOR_TYPE_F4)
			//HCLK system speed
			#define CONFIG_XTAL_HZ 	 8000000U
			#define CONFIG_HCLK_HZ   120000000U
			#define CONFIG_PCLK1_HZ  (CONFIG_HCLK_HZ/4)
			#define CONFIG_PCLK2_HZ  (CONFIG_HCLK_HZ/2)

#elif defined(STM32MCU_MAJOR_TYPE_F2)
			//HCLK system speed
			#define CONFIG_XTAL_HZ  8000000U
			#define CONFIG_HCLK_HZ  120000000U
			#define CONFIG_PCLK1_HZ (CONFIG_HCLK_HZ/4)
			#define CONFIG_PCLK2_HZ (CONFIG_HCLK_HZ/2)
#elif defined(STM32MCU_MAJOR_TYPE_F37)
			//HCLK system speed
			#define CONFIG_XTAL_HZ 	 8000000U
			#define CONFIG_HCLK_HZ   72000000U
			#define CONFIG_PCLK1_HZ  (CONFIG_HCLK_HZ/4)
			#define CONFIG_PCLK2_HZ  (CONFIG_HCLK_HZ/2)
#endif

//! Enable CPU usage API
#define CONFIG_ISIX_CPU_USAGE_API
//! Define memory protection model
#define CONFIG_ISIX_LOGLEVEL ISIXLOG_CRIT
#define CONFIG_ISIX_MEMORY_PROTECTION_MODEL ISIX_MPROT_LITE
//#define CONFIG_ISIX_LOGLEVEL_SEMAPHORE ISIXLOG_DEBUG
//#define CONFIG_ISIX_LOGLEVEL_VTIMERS ISIXLOG_INFO

#endif /* CONFIG_H_ */
