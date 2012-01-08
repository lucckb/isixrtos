/*
 * generic_tim.hpp
 *
 *  Created on: 06-01-2012
 *      Author: lucck
 */

#ifndef GENERIC_TIM_H_
#define GENERIC_TIM_H_
/* ---------------------------------------------------------------------------- */
#include <stm32f10x_lib.h>
#include <stm32f10x_tim.h>
#include <stddef.h>
/* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
 namespace stm32 {
#endif


/* ---------------------------------------------------------------------------- */
enum tim_cc_chns { tim_cc_chn1, tim_cc_chn2, tim_cc_chn3, tim_cc_chn4 };


/* ---------------------------------------------------------------------------- */
//Configure the time base in timer
static inline void tim_timebase_config(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t cnt_mode,
		uint16_t period, uint16_t clkdiv, uint8_t rptcounter )
{
	 uint16_t tmpcr1 = 0;

	 tmpcr1 = TIMx->CR1;

	  if((TIMx == TIM1) || (TIMx == TIM8)|| (TIMx == TIM2) || (TIMx == TIM3)||
	     (TIMx == TIM4) || (TIMx == TIM5))
	  {
	    /* Select the Counter Mode */
	    tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS)));
	    tmpcr1 |= (uint32_t)cnt_mode;
	  }

	  if((TIMx != TIM6) && (TIMx != TIM7))
	  {
	    /* Set the clock division */
	    tmpcr1 &= (uint16_t)(~((uint16_t)TIM_CR1_CKD));
	    tmpcr1 |= (uint32_t)clkdiv;
	  }

	  TIMx->CR1 = tmpcr1;

	  /* Set the Autoreload value */
	  TIMx->ARR = period ;

	  /* Set the Prescaler value */
	  TIMx->PSC = prescaler;

	  if ((TIMx == TIM1) || (TIMx == TIM8)|| (TIMx == TIM15)|| (TIMx == TIM16) || (TIMx == TIM17))
	  {
	    /* Set the Repetition Counter value */
	    TIMx->RCR = rptcounter;
	  }

	  /* Generate an update event to reload the Prescaler and the Repetition counter
	     values immediately */
	  TIMx->EGR = TIM_PSCReloadMode_Immediate;

}
/* ---------------------------------------------------------------------------- */
//Set compare capture reg
static inline void tim_set_ccr(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t val)
{
	  /* Set the Capture Compare Register value */
	  switch( chn )
	  {
	  case tim_cc_chn1:  TIMx->CCR1 = val; break;
	  case tim_cc_chn2:  TIMx->CCR2 = val; break;
	  case tim_cc_chn3:  TIMx->CCR3 = val; break;
	  case tim_cc_chn4:  TIMx->CCR4 = val; break;
	  }
}
/* ---------------------------------------------------------------------------- */
//Configure output as output compare
static inline void tim_oc_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t oc_mode,
		uint16_t oc_value, uint16_t output_state, uint16_t output_n_state, uint16_t polarity,
		uint16_t n_polarity, uint16_t idle_state, uint16_t n_idle_state)
{
	 uint16_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;

	 /* Disable the Channel 1: Reset the CC1E Bit */
	 switch(chn)
	 {
		 case tim_cc_chn1: TIMx->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC1E); break;
		 case tim_cc_chn2: TIMx->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC2E); break;
		 case tim_cc_chn3: TIMx->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC3E); break;
		 case tim_cc_chn4: TIMx->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC4E); break;

	 }
	  /* Get the TIMx CCER register value */
	  tmpccer = TIMx->CCER;
	  /* Get the TIMx CR2 register value */
	  tmpcr2 =  TIMx->CR2;

	  /* Get the TIMx CCMR1 register value */
	  switch(chn)
	  {
		 case tim_cc_chn1: case tim_cc_chn2: tmpccmrx = TIMx->CCMR1; break;
		 case tim_cc_chn3: case tim_cc_chn4: tmpccmrx = TIMx->CCMR2; break;
	  }
	  /* Reset the Output Compare Mode Bits */
	  switch(chn)
	  {
	  case tim_cc_chn1:
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC1M));
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC1S));
		  break;
	  case tim_cc_chn2:
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC2M));
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC2S));
		  break;
	  case tim_cc_chn3:
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR2_OC3M));
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR2_CC3S));
		  break;
	  case tim_cc_chn4:
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR2_OC4M));
		  tmpccmrx &= (uint16_t)(~((uint16_t)TIM_CCMR2_CC4S));
		  break;
	  }
	  /* Select the Output Compare Mode */
	  switch(chn)
	  {
	 	case tim_cc_chn1: case tim_cc_chn3: tmpccmrx |= oc_mode; break;
	 	case tim_cc_chn2: case tim_cc_chn4: tmpccmrx |= oc_mode<<8; break;
	  }
	  /* Reset the Output Polarity level */
	  tmpccer &= (uint16_t)(~((uint16_t)(TIM_CCER_CC1P<<(4*chn))));
	  /* Set the Output Compare Polarity */
	  tmpccer |= polarity << (4*chn);

	  /* Set the Output State */
	  tmpccer |= output_state << (4*chn);

	  if((TIMx == TIM1) || (TIMx == TIM8)|| (TIMx == TIM15)||
	     (TIMx == TIM16)|| (TIMx == TIM17))
	  {

	    /* Reset the Output N Polarity level */
	    tmpccer &= (uint16_t)(~((uint16_t)(TIM_CCER_CC1NP<<(4*chn))));
	    /* Set the Output N Polarity */
	    tmpccer |= n_polarity << (4*chn);

	    /* Reset the Output N State */
	    tmpccer &= (uint16_t)(~((uint16_t)(TIM_CCER_CC1NE<<(4*chn))));
	    /* Set the Output N State */
	    tmpccer |= output_n_state << (4*chn);

	    /* Reset the Output Compare and Output Compare N IDLE State */
	    switch(chn)
	    {
	    case tim_cc_chn1:
	    	tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS1));
	    	tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS1N));
	    	break;
	    case tim_cc_chn2:
	 	    tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS2));
	 	    tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS2N));
	 	    break;
	    case tim_cc_chn3:
	   	 	tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS3));
	   	 	tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS3N));
	   	 	break;
	    case tim_cc_chn4:
	 	   	tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS4));
	 	   	//tmpcr2 &= (uint16_t)(~((uint16_t)TIM_CR2_OIS4N));
	 	   	break;
	    }


	    /* Set the Output Idle state */
	    tmpcr2 |= idle_state << (2*chn);
	    /* Set the Output N Idle state */
	    tmpcr2 |= n_idle_state << (2*chn);
	  }
	  /* Write to TIMx CR2 */
	  TIMx->CR2 = tmpcr2;

	  /* Write to TIMx CCMR1 */
	  switch(chn)
	  {
	  	  case tim_cc_chn1: case tim_cc_chn2: TIMx->CCMR1 = tmpccmrx; break;
	  	  case tim_cc_chn3: case tim_cc_chn4: TIMx->CCMR2 = tmpccmrx; break;
	  }

	  /* Set the Capture Compare Register value */
	  tim_set_ccr( TIMx, chn, oc_value );

	  /* Write to TIMx CCER */
	  TIMx->CCER = tmpccer;

}
/* ---------------------------------------------------------------------------- */
//Enable CTRL advanced output only on advanced timers like TIM1
static inline void tim_ctrl_pwm_outputs( TIM_TypeDef* TIMx, int enable )
{
	/* Check the parameters */
	if ( enable )
	{
	    /* Enable the TIM Main Output */
	    TIMx->BDTR |= TIM_BDTR_MOE;
	}
	else
	{
	    /* Disable the TIM Main Output */
	    TIMx->BDTR &= (uint16_t)(~((uint16_t)TIM_BDTR_MOE));
	}
}
/* ---------------------------------------------------------------------------- */
//Enable or disable timer
static inline void tim_enable(TIM_TypeDef* TIMx, int enable )
{

	  if ( enable )
	  {
	    /* Enable the TIM Counter */
	    TIMx->CR1 |= TIM_CR1_CEN;
	  }
	  else
	  {
	    /* Disable the TIM Counter */
	    TIMx->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
	  }
}
/* ---------------------------------------------------------------------------- */
//Configure BDTR register
static inline void tim_bdtr_config(TIM_TypeDef* TIMx, uint8_t dead_time, uint16_t config)
{
	TIMx->BDTR = (uint16_t)dead_time | config;
}
/* ---------------------------------------------------------------------------- */
//Preload enable timer CCR will be updated only on new cycle
/* ---------------------------------------------------------------------------- */
//Set compare capture reg
static inline void tim_preload_enable(TIM_TypeDef* TIMx, enum tim_cc_chns chn, int enabled)
{
	switch( chn )
	{
		case tim_cc_chn1:
			if(enabled) TIMx->CCMR1 |= TIM_CCMR1_OC1PE;
			else TIMx->CCMR1 &= ~TIM_CCMR1_OC1PE;
			break;
		case tim_cc_chn2:
			if(enabled) TIMx->CCMR1 |= TIM_CCMR1_OC2PE;
			else TIMx->CCMR1 &= ~TIM_CCMR1_OC2PE;
			break;
		case tim_cc_chn3:
			if(enabled) TIMx->CCMR2 |= TIM_CCMR2_OC3PE;
			else TIMx->CCMR2 &= ~TIM_CCMR2_OC3PE;
			break;
		case tim_cc_chn4:
			if(enabled) TIMx->CCMR2 |= TIM_CCMR2_OC4PE;
			else TIMx->CCMR2 &= ~TIM_CCMR2_OC4PE;
			break;
	}

}

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 }
#endif
#endif /* GENERIC_TIM_HPP_ */
