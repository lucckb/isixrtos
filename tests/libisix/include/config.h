/*
 * config.h
 *
 *  Created on: 4 gru 2013
 *      Author: lucck
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/* Enable shoutdown API */
#define ISIX_CONFIG_SHUTDOWN_API
/* Enable task stack checking */
#define ISIX_CONFIG_TASK_STACK_CHECK


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
#endif

#endif /* CONFIG_H_ */
