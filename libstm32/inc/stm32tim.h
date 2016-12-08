/*
 * stm32tim.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#ifndef STM32TIM_H_
#define STM32TIM_H_



#include <stm32lib.h>
#include <stddef.h>

#ifdef __cplusplus
 namespace stm32 {
#endif


enum tim_cc_chns { tim_cc_chn1, tim_cc_chn2, 
				   tim_cc_chn3, tim_cc_chn4,
				   tim_cc_chn5, tim_cc_chn6 };



//Configure the time base in timer
static inline void tim_timebase_init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t cnt_mode,
		uint16_t period, uint16_t clkdiv, uint16_t rptcounter )
{
	 uint16_t tmpcr1 = 0;

	 tmpcr1 = TIMx->CR1;

	  if( _TIM_IS_ON_LIST_1(TIMx) )
	  {
	    /* Select the Counter Mode */
	    tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS)));
	    tmpcr1 |= (uint32_t)cnt_mode;
	  }

	  if( _TIM_IS_ON_LIST_2(TIMx) )
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

	  if ( _TIM_IS_ON_LIST_3( TIMx ) )
	  {
	    /* Set the Repetition Counter value */
	    TIMx->RCR = rptcounter;
	  }

	  /* Generate an update event to reload the Prescaler and the Repetition counter
	     values immediately */
	  TIMx->EGR = TIM_PSCReloadMode_Immediate;

}

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
#ifdef TIM_CCER_CC5E
	  case tim_cc_chn5:  TIMx->CCR5 = val; break;
#endif
#ifdef TIM_CCER_CC6E
	  case tim_cc_chn6:  TIMx->CCR6 = val; break;
#endif
	  default: return;
	  }
}

//Configure output as output compare
static inline void tim_oc_init(TIM_TypeDef* TIMx, enum tim_cc_chns chn, stm32tim_oc_t oc_mode,
		uint16_t oc_value, uint16_t output_state, uint16_t output_n_state, uint16_t polarity,
		uint16_t n_polarity, uint16_t idle_state, uint16_t n_idle_state)
{
	 stm32tim_oc_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;

	 /* Disable the Channel 1: Reset the CC1E Bit */
	 switch(chn)
	 {
		 case tim_cc_chn1: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC1E); break;
		 case tim_cc_chn2: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC2E); break;
		 case tim_cc_chn3: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC3E); break;
		 case tim_cc_chn4: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC4E); break;
#ifdef TIM_CCER_CC6E
		 case tim_cc_chn5: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC5E); break;
		 case tim_cc_chn6: TIMx->CCER &= (stm32tim_oc_t)(~(stm32tim_oc_t)TIM_CCER_CC6E); break;
#endif
	  default: return;
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
#ifdef TIM_CCER_CC6E
		 case tim_cc_chn5: case tim_cc_chn6: tmpccmrx = TIMx->CCMR3; break;
#endif
		 default: return;
	  }
	  /* Reset the Output Compare Mode Bits */
	  switch(chn)
	  {
	  case tim_cc_chn1:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR1_OC1M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR1_CC1S));
		  break;
	  case tim_cc_chn2:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR1_OC2M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR1_CC2S));
		  break;
	  case tim_cc_chn3:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_OC3M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_CC3S));
		  break;
	  case tim_cc_chn4:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_OC4M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_CC4S));
		  break;
#ifdef TIM_CCER_CC6E
	  case tim_cc_chn5:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_OC5M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_CC5S));
		  break;
	  case tim_cc_chn6:
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_OC6M));
		  tmpccmrx &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CCMR2_CC6S));
		  break;
#endif
		 default: return;
	  }
	  /* Select the Output Compare Mode */
	  switch(chn)
	  {
	 	case tim_cc_chn1: case tim_cc_chn3: tmpccmrx |= oc_mode; break;
	 	case tim_cc_chn2: case tim_cc_chn4: tmpccmrx |= oc_mode<<8; break;
#ifdef TIM_CCER_CC6E
		case tim_cc_chn5: tmpccmrx |= oc_mode; break;
	 	case tim_cc_chn6: tmpccmrx |= oc_mode<<8; break;
#endif
		default: return;
	  }
	  /* Reset the Output Polarity level */
	  tmpccer &= (stm32tim_oc_t)(~((stm32tim_oc_t)(TIM_CCER_CC1P<<(4*chn))));
	  /* Set the Output Compare Polarity */
	  tmpccer |= polarity << (4*chn);

	  /* Set the Output State */
	  tmpccer |= output_state << (4*chn);

	  if( _TIM_IS_ON_LIST_3(TIMx) )
	  {

	    /* Reset the Output N Polarity level */
	    tmpccer &= (stm32tim_oc_t)(~((stm32tim_oc_t)(TIM_CCER_CC1NP<<(4*chn))));
	    /* Set the Output N Polarity */
	    tmpccer |= n_polarity << (4*chn);

	    /* Reset the Output N State */
	    tmpccer &= (stm32tim_oc_t)(~((stm32tim_oc_t)(TIM_CCER_CC1NE<<(4*chn))));
	    /* Set the Output N State */
	    tmpccer |= output_n_state << (4*chn);

	    /* Reset the Output Compare and Output Compare N IDLE State */
	    switch(chn)
	    {
	    case tim_cc_chn1:
	    	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS1));
	    	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS1N));
	    	break;
	    case tim_cc_chn2:
	 	    tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS2));
	 	    tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS2N));
	 	    break;
	    case tim_cc_chn3:
	   	 	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS3));
	   	 	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS3N));
	   	 	break;
	    case tim_cc_chn4:
	 	   	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS4));
	 	   	break;
#ifdef TIM_CCER_CC6E
	    case tim_cc_chn5:
	 	   	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS5));
	 	   	break;
	    case tim_cc_chn6:
	 	   	tmpcr2 &= (stm32tim_oc_t)(~((stm32tim_oc_t)TIM_CR2_OIS6));
	 	   	break;
#endif
		 default: return;
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
#ifdef TIM_CCER_CC6E
	  	  case tim_cc_chn5: case tim_cc_chn6: TIMx->CCMR3 = tmpccmrx; break;
#endif
		  default: return;
	  }

	  /* Set the Capture Compare Register value */
	  tim_set_ccr( TIMx, chn, oc_value );

	  /* Write to TIMx CCER */
	  TIMx->CCER = tmpccer;

}

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

//Enable or disable timer
static inline void tim_cmd(TIM_TypeDef* TIMx, int enable )
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

//If timer is enabled
static inline bool tim_is_enabled( TIM_TypeDef* TIMx )
{
	return TIMx->CR1 & TIM_CR1_CEN;
}

#if defined(STM32MCU_MAJOR_TYPE_F3)
static inline void tim_break1_config(TIM_TypeDef* TIMx, 
		uint32_t TIM_Break1Polarity, uint8_t TIM_Break1Filter)
{   
  /* Reset the BKP and BKF Bits */
  TIMx->BDTR &= (uint32_t)~ (TIM_BDTR_BKP | TIM_BDTR_BKF);
  /* Configure the Break1 polarity and filter */
  TIMx->BDTR |=	TIM_Break1Polarity |((uint32_t)TIM_Break1Filter << 16);
}
static inline void tim_break2_config(TIM_TypeDef* TIMx, 
		uint32_t TIM_Break2Polarity, uint8_t TIM_Break2Filter)
{
  /* Reset the BKP and BKF Bits */
  TIMx->BDTR &= (uint32_t)~ (TIM_BDTR_BK2P | TIM_BDTR_BK2F);
  /* Configure the Break1 polarity and filter */
  TIMx->BDTR |=	TIM_Break2Polarity |((uint32_t)TIM_Break2Filter << 20);
}

static inline void tim_break1_cmd(TIM_TypeDef* timx, bool enable)
{
  if (enable)
  {
    /* enable the break1 */
    timx->BDTR |= TIM_BDTR_BKE;
  }
  else
  {
    /* disable the break1 */
    timx->BDTR &= (uint32_t)~TIM_BDTR_BKE;
  } 
}

static inline void tim_break2_cmd(TIM_TypeDef* TIMx, bool enable )
{
  if (enable)
  {
    /* Enable the Break1 */
    TIMx->BDTR |= TIM_BDTR_BK2E;
  }
  else
  {
    /* Disable the Break1 */
    TIMx->BDTR &= (uint32_t)~TIM_BDTR_BK2E;
  }
}
#else
//Configure BDTR register
static inline void tim_bdtr_config(TIM_TypeDef* TIMx, uint8_t dead_time, uint16_t config)
{
	TIMx->BDTR = (uint16_t)dead_time | config;
}
#endif

//Preload enable timer CCR will be updated only on new cycle

//Set compare capture reg
static inline void tim_oc_preload_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn, int enabled)
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
#ifdef TIM_CCER_CC6E
		case tim_cc_chn5:
			if(enabled) TIMx->CCMR3 |= TIM_CCMR3_OC5PE;
			else TIMx->CCMR3 &= ~TIM_CCMR3_OC5PE;
			break;
		case tim_cc_chn6:
			if(enabled) TIMx->CCMR3 |= TIM_CCMR3_OC6PE;
			else TIMx->CCMR3 &= ~TIM_CCMR3_OC6PE;
			break;
#endif
		default: break;
	}

}

/** Enable or disable ARR shadow register in the time base,
 * required only if the time base will be changed
 */
static inline void tim_arr_preload_enable(TIM_TypeDef* TIMx, int enable)
{
	/* Check the parameters */
	  if (enable)
	  {
	    /* Set the ARR Preload Bit */
	    TIMx->CR1 |= TIM_CR1_ARPE;
	  }
	  else
	  {
	    /* Reset the ARR Preload Bit */
	    TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_ARPE);
	  }
}

//Internal used by ic_init
#ifdef __cplusplus
namespace _internal {
namespace tim {

static const uint16_t  SMCR_ETR_Mask      =       0x00FF;
static const uint16_t  CCER_CCE_Set       =       0x0001;
static const uint16_t  CCER_CCNE_Set      =       0x0004;
static const uint16_t  CCMR_Offset        =       0x0018;
#else
#define  SMCR_ETR_Mask             (uint16_t)0x00FF
#define  CCER_CCE_Set              (uint16_t)0x0001
#define  CCER_CCNE_Set             (uint16_t)0x0004
#define  CCMR_Offset               (uint16_t)0x0018
#endif

static inline void TI1_Config(TIM_TypeDef* TIMx, uint16_t TIM_ICPolarity, uint16_t TIM_ICSelection,
                       uint16_t TIM_ICFilter)
{
  uint16_t tmpccmr1 = 0, tmpccer = 0;
  /* Disable the Channel 1: Reset the CC1E Bit */
  TIMx->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC1E);
  tmpccmr1 = TIMx->CCMR1;
  tmpccer = TIMx->CCER;
  /* Select the Input and set the filter */
  tmpccmr1 &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCMR1_CC1S)) & ((uint16_t)~((uint16_t)TIM_CCMR1_IC1F)));
  tmpccmr1 |= (uint16_t)(TIM_ICSelection | (uint16_t)(TIM_ICFilter << (uint16_t)4));

  if( _TIM_IS_ON_LIST_4(TIMx) )
  {
    /* Select the Polarity and set the CC1E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC1P));
    tmpccer |= (uint16_t)(TIM_ICPolarity | (uint16_t)TIM_CCER_CC1E);
  }
  else
  {
    /* Select the Polarity and set the CC1E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC1P | TIM_CCER_CC1NP));
    tmpccer |= (uint16_t)(TIM_ICPolarity | (uint16_t)TIM_CCER_CC1E);
  }

  /* Write to TIMx CCMR1 and CCER registers */
  TIMx->CCMR1 = tmpccmr1;
  TIMx->CCER = tmpccer;
}


static inline void TIM_SetIC1Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{
  /* Reset the IC1PSC Bits */
  TIMx->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_IC1PSC);
  /* Set the IC1PSC value */
  TIMx->CCMR1 |= TIM_ICPSC;
}

static inline void TI2_Config(TIM_TypeDef* TIMx, uint16_t TIM_ICPolarity, uint16_t TIM_ICSelection,
                       uint16_t TIM_ICFilter)
{
  stm32tim_oc_t tmpccmr1 = 0, tmpccer = 0, tmp = 0;
  /* Disable the Channel 2: Reset the CC2E Bit */
  TIMx->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC2E);
  tmpccmr1 = TIMx->CCMR1;
  tmpccer = TIMx->CCER;
  tmp = (uint16_t)(TIM_ICPolarity << 4);
  /* Select the Input and set the filter */
  tmpccmr1 &= (stm32tim_oc_t)(((stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_CC2S)) 
		  & ((stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_IC2F)));
  tmpccmr1 |= (stm32tim_oc_t)(TIM_ICFilter << 12);
  tmpccmr1 |= (stm32tim_oc_t)(TIM_ICSelection << 8);

  if(_TIM_IS_ON_LIST_4(TIMx) )
  {
    /* Select the Polarity and set the CC2E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC2P));
    tmpccer |=  (uint16_t)(tmp | (uint16_t)TIM_CCER_CC2E);
  }
  else
  {
    /* Select the Polarity and set the CC2E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC2P | TIM_CCER_CC2NP));
    tmpccer |= (uint16_t)(TIM_ICPolarity | (uint16_t)TIM_CCER_CC2E);
  }

  /* Write to TIMx CCMR1 and CCER registers */
  TIMx->CCMR1 = tmpccmr1 ;
  TIMx->CCER = tmpccer;
}

static inline void TIM_SetIC2Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{
  /* Reset the IC2PSC Bits */
  TIMx->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_IC2PSC);
  /* Set the IC2PSC value */
  TIMx->CCMR1 |= (uint16_t)(TIM_ICPSC << 8);
}

static inline void TI3_Config(TIM_TypeDef* TIMx, uint16_t TIM_ICPolarity, uint16_t TIM_ICSelection,
                       uint16_t TIM_ICFilter)
{
  uint16_t tmpccmr2 = 0, tmpccer = 0, tmp = 0;
  /* Disable the Channel 3: Reset the CC3E Bit */
  TIMx->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC3E);
  tmpccmr2 = TIMx->CCMR2;
  tmpccer = TIMx->CCER;
  tmp = (uint16_t)(TIM_ICPolarity << 8);
  /* Select the Input and set the filter */
  tmpccmr2 &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCMR2_CC3S)) & ((uint16_t)~((uint16_t)TIM_CCMR2_IC3F)));
  tmpccmr2 |= (uint16_t)(TIM_ICSelection | (uint16_t)(TIM_ICFilter << (uint16_t)4));

  if( _TIM_IS_ON_LIST_4(TIMx) )
  {
    /* Select the Polarity and set the CC3E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC3P));
    tmpccer |= (uint16_t)(tmp | (uint16_t)TIM_CCER_CC3E);
  }
  else
  {
    /* Select the Polarity and set the CC3E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC3P | TIM_CCER_CC3NP));
    tmpccer |= (uint16_t)(TIM_ICPolarity | (uint16_t)TIM_CCER_CC3E);
  }

  /* Write to TIMx CCMR2 and CCER registers */
  TIMx->CCMR2 = tmpccmr2;
  TIMx->CCER = tmpccer;
}

static inline void TIM_SetIC3Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{
  /* Reset the IC3PSC Bits */
  TIMx->CCMR2 &= (uint16_t)~((uint16_t)TIM_CCMR2_IC3PSC);
  /* Set the IC3PSC value */
  TIMx->CCMR2 |= TIM_ICPSC;
}

static inline void TI4_Config(TIM_TypeDef* TIMx, uint16_t TIM_ICPolarity, uint16_t TIM_ICSelection,
                       uint16_t TIM_ICFilter)
{
  uint16_t tmpccmr2 = 0, tmpccer = 0, tmp = 0;

   /* Disable the Channel 4: Reset the CC4E Bit */
  TIMx->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC4E);
  tmpccmr2 = TIMx->CCMR2;
  tmpccer = TIMx->CCER;
  tmp = (uint16_t)(TIM_ICPolarity << 12);
  /* Select the Input and set the filter */
  tmpccmr2 &= (uint16_t)((uint16_t)(~(uint16_t)TIM_CCMR2_CC4S) & ((uint16_t)~((uint16_t)TIM_CCMR2_IC4F)));
  tmpccmr2 |= (uint16_t)(TIM_ICSelection << 8);
  tmpccmr2 |= (uint16_t)(TIM_ICFilter << 12);

  if( _TIM_IS_ON_LIST_4(TIMx) )
  {
    /* Select the Polarity and set the CC4E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC4P));
    tmpccer |= (uint16_t)(tmp | (uint16_t)TIM_CCER_CC4E);
  }
  else
  {
    /* Select the Polarity and set the CC4E Bit */
    tmpccer &= (uint16_t)~((uint16_t)(TIM_CCER_CC3P | TIM_CCER_CC4NP));
    tmpccer |= (uint16_t)(TIM_ICPolarity | (uint16_t)TIM_CCER_CC4E);
  }
  /* Write to TIMx CCMR2 and CCER registers */
  TIMx->CCMR2 = tmpccmr2;
  TIMx->CCER = tmpccer;
}

static inline void TIM_SetIC4Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{
  /* Reset the IC4PSC Bits */
  TIMx->CCMR2 &= (uint16_t)~((uint16_t)TIM_CCMR2_IC4PSC);
  /* Set the IC4PSC value */
  TIMx->CCMR2 |= (uint16_t)(TIM_ICPSC << 8);
}

static inline void ForcedOC1Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr1 = 0;
  tmpccmr1 = TIMx->CCMR1;
  /* Reset the OC1M Bits */
  tmpccmr1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_OC1M);
  /* Configure The Forced output Mode */
  tmpccmr1 |= TIM_ForcedAction;
  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}

static inline void ForcedOC2Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr1 = 0;
  tmpccmr1 = TIMx->CCMR1;
  /* Reset the OC2M Bits */
  tmpccmr1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_OC2M);
  /* Configure The Forced output Mode */
  tmpccmr1 |= (stm32tim_oc_t)(TIM_ForcedAction << 8);
  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}


static inline void ForcedOC3Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr2 = 0;
  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC1M Bits */
  tmpccmr2 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR2_OC3M);
  /* Configure The Forced output Mode */
  tmpccmr2 |= TIM_ForcedAction;
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}


static inline void ForcedOC4Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr2 = 0;
  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC2M Bits */
  tmpccmr2 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR2_OC4M);
  /* Configure The Forced output Mode */
  tmpccmr2 |= (stm32tim_oc_t)(TIM_ForcedAction << 8);
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}

#ifdef TIM_CCER_CC6E
static inline void ForcedOC5Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr3 = 0;
  tmpccmr3 = TIMx->CCMR3;
  /* Reset the OC1M Bits */
  tmpccmr3 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR3_OC5M);
  /* Configure The Forced output Mode */
  tmpccmr3 |= TIM_ForcedAction;
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR3 = tmpccmr2;
}


static inline void ForcedOC6Config(TIM_TypeDef* TIMx, uint16_t TIM_ForcedAction)
{
  stm32tim_oc_t tmpccmr3 = 0;
  tmpccmr3 = TIMx->CCMR3;
  /* Reset the OC2M Bits */
  tmpccmr3 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR3_OC6M);
  /* Configure The Forced output Mode */
  tmpccmr3 |= (stm32tim_oc_t)(TIM_ForcedAction << 8);
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR3 = tmpccmr3;
}
#endif

static inline  void OC1FastConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCFast)
{
  stm32tim_oc_t tmpccmr1 = 0;
  /* Get the TIMx CCMR1 register value */
  tmpccmr1 = TIMx->CCMR1;
  /* Reset the OC1FE Bit */
  tmpccmr1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_OC1FE);
  /* Enable or Disable the Output Compare Fast Bit */
  tmpccmr1 |= TIM_OCFast;
  /* Write to TIMx CCMR1 */
  TIMx->CCMR1 = tmpccmr1;
}


static inline void OC2FastConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCFast)
{
  stm32tim_oc_t tmpccmr1 = 0;
  /* Get the TIMx CCMR1 register value */
  tmpccmr1 = TIMx->CCMR1;
  /* Reset the OC2FE Bit */
  tmpccmr1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_OC2FE);
  /* Enable or Disable the Output Compare Fast Bit */
  tmpccmr1 |= (stm32tim_oc_t)(TIM_OCFast << 8);
  /* Write to TIMx CCMR1 */
  TIMx->CCMR1 = tmpccmr1;
}

static inline void OC3FastConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCFast)
{
  stm32tim_oc_t tmpccmr2 = 0;
  /* Get the TIMx CCMR2 register value */
  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC3FE Bit */
  tmpccmr2 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR2_OC3FE);
  /* Enable or Disable the Output Compare Fast Bit */
  tmpccmr2 |= TIM_OCFast;
  /* Write to TIMx CCMR2 */
  TIMx->CCMR2 = tmpccmr2;
}


static inline void OC4FastConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCFast)
{
  stm32tim_oc_t tmpccmr2 = 0;
  /* Get the TIMx CCMR2 register value */
  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC4FE Bit */
  tmpccmr2 &= (stm32tim_oc_t)~((uint16_t)TIM_CCMR2_OC4FE);
  /* Enable or Disable the Output Compare Fast Bit */
  tmpccmr2 |= (stm32tim_oc_t)(TIM_OCFast << 8);
  /* Write to TIMx CCMR2 */
  TIMx->CCMR2 = tmpccmr2;
}


static inline void ClearOC1Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr1 = 0;

  tmpccmr1 = TIMx->CCMR1;

  /* Reset the OC1CE Bit */
  tmpccmr1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_OC1CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr1 |= TIM_OCClear;
  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}

static inline void ClearOC2Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr1 = 0;

  tmpccmr1 = TIMx->CCMR1;
  /* Reset the OC2CE Bit */
  tmpccmr1 &= (stm32tim_oc_t)~((uint16_t)TIM_CCMR1_OC2CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr1 |= (stm32tim_oc_t)(TIM_OCClear << 8);
  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}


static inline void ClearOC3Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr2 = 0;

  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC3CE Bit */
  tmpccmr2 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR2_OC3CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr2 |= TIM_OCClear;
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}


static inline void ClearOC4Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr2 = 0;
  tmpccmr2 = TIMx->CCMR2;
  /* Reset the OC4CE Bit */
  tmpccmr2 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR2_OC4CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr2 |= (stm32tim_oc_t)(TIM_OCClear << 8);
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}
#ifdef TIM_CCER_CC6E
static inline void ClearOC5Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr3 = 0;

  tmpccmr3 = TIMx->CCMR3;
  /* Reset the OC3CE Bit */
  tmpccmr3 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR3_OC5CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr3 |= TIM_OCClear;
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR3 = tmpccmr3;
}


static inline void ClearOC6Ref(TIM_TypeDef* TIMx, uint16_t TIM_OCClear)
{
  stm32tim_oc_t tmpccmr3 = 0;
  tmpccmr3 = TIMx->CCMR3;
  /* Reset the OC4CE Bit */
  tmpccmr3 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR3_OC6CE);
  /* Enable or Disable the Output Compare Clear Bit */
  tmpccmr3 |= (stm32tim_oc_t)(TIM_OCClear << 8);
  /* Write to TIMx CCMR2 register */
  TIMx->CCMR3 = tmpccmr3;
}
#endif

static inline void OC1PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC1P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC1P);
  tmpccer |= TIM_OCPolarity;
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


static inline void OC1NPolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCNPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC1NP Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC1NP);
  tmpccer |= TIM_OCNPolarity;
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


static inline void OC2PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC2P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC2P);
  tmpccer |= (stm32tim_oc_t)(TIM_OCPolarity << 4);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}

static inline void OC2NPolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCNPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC2NP Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC2NP);
  tmpccer |= (stm32tim_oc_t)(TIM_OCNPolarity << 4);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


static inline void OC3PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC3P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC3P);
  tmpccer |= (stm32tim_oc_t)(TIM_OCPolarity << 8);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


static inline void OC3NPolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCNPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC3NP Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC3NP);
  tmpccer |= (stm32tim_oc_t)(TIM_OCNPolarity << 8);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


static inline void OC4PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC4P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC4P);
  tmpccer |= (stm32tim_oc_t)(TIM_OCPolarity << 12);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}


#ifdef TIM_CCER_CC6E
static inline void OC5PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC4P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC4P);
  tmpccer |= (stm32tim_oc_t)(TIM_OCPolarity << 16);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}
static inline void OC6PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  stm32tim_oc_t tmpccer = 0;

  tmpccer = TIMx->CCER;
  /* Set or Reset the CC4P Bit */
  tmpccer &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCER_CC4P);
  tmpccer |= (stm32tim_oc_t)(TIM_OCPolarity << 20);
  /* Write to TIMx CCER register */
  TIMx->CCER = tmpccer;
}
#endif

static inline void SetIC1Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{
  /* Reset the IC1PSC Bits */
  TIMx->CCMR1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_IC1PSC);
  /* Set the IC1PSC value */
  TIMx->CCMR1 |= TIM_ICPSC;
}

static inline void SetIC2Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{

  /* Reset the IC2PSC Bits */
  TIMx->CCMR1 &= (stm32tim_oc_t)~((stm32tim_oc_t)TIM_CCMR1_IC2PSC);
  /* Set the IC2PSC value */
  TIMx->CCMR1 |= (stm32tim_oc_t)(TIM_ICPSC << 8);
}


static inline void SetIC3Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{

  /* Reset the IC3PSC Bits */
  TIMx->CCMR2 &= (uint16_t)~((uint16_t)TIM_CCMR2_IC3PSC);
  /* Set the IC3PSC value */
  TIMx->CCMR2 |= TIM_ICPSC;
}


static inline void SetIC4Prescaler(TIM_TypeDef* TIMx, uint16_t TIM_ICPSC)
{

  /* Reset the IC4PSC Bits */
  TIMx->CCMR2 &= (uint16_t)~((uint16_t)TIM_CCMR2_IC4PSC);
  /* Set the IC4PSC value */
  TIMx->CCMR2 |= (uint16_t)(TIM_ICPSC << 8);
}



#ifdef __cplusplus
}
}
#endif

/**
  * @brief  Initializes the TIM peripheral according to the specified
  *         parameters in the TIM_ICInitStruct.
  * @param  TIMx: where x can be  1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  channel Specifies the TIM channel
  * @param  ic_polarity Specifies the active edge of the input signal.
  * @param  ic_selection Specifies the input.
  * @param  ic_prescaler Specifies the Input Capture Prescaler.
  * @param  ic_filter Specifies the input capture filter.
  * @retval None
  */
static inline void tim_ic_init(TIM_TypeDef* TIMx, enum tim_cc_chns chn,  uint16_t ic_polarity,
		uint16_t ic_selection, uint16_t ic_prescaler, uint16_t ic_filter)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  if (chn == tim_cc_chn1)
  {
    /* TI1 Configuration */
    TI1_Config(TIMx, ic_polarity, ic_selection, ic_filter );
    /* Set the Input Capture Prescaler value */
    TIM_SetIC1Prescaler(TIMx, ic_prescaler );
  }
  else if (chn == tim_cc_chn2)
  {
    /* TI2 Configuration */
    TI2_Config(TIMx, ic_polarity, ic_selection, ic_filter );
    /* Set the Input Capture Prescaler value */
    TIM_SetIC2Prescaler(TIMx, ic_prescaler);
  }
  else if (chn == tim_cc_chn3)
  {
    /* TI3 Configuration */
    TI3_Config(TIMx,  ic_polarity, ic_selection, ic_filter );
    /* Set the Input Capture Prescaler value */
    TIM_SetIC3Prescaler(TIMx, ic_prescaler );
  }
  else if (chn == tim_cc_chn4)
  {
    /* TI4 Configuration */
    TI4_Config(TIMx, ic_polarity, ic_selection, ic_filter );
    /* Set the Input Capture Prescaler value */
    TIM_SetIC4Prescaler(TIMx, ic_prescaler );
  }
}

/**
  * @brief  Configures the TIM peripheral according to the specified
  *         parameters in the TIM_ICInitStruct to measure an external PWM signal.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_ICInitStruct: pointer to a TIM_ICInitTypeDef structure
  *         that contains the configuration information for the specified TIM peripheral.
  * @retval None
  */
static inline void tim_pwmi_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn,  uint16_t ic_polarity,
		uint16_t ic_selection, uint16_t ic_prescaler, uint16_t ic_filter)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  uint16_t icoppositepolarity = TIM_ICPolarity_Rising;
  uint16_t icoppositeselection = TIM_ICSelection_DirectTI;
  /* Select the Opposite Input Polarity */
  if (ic_polarity == TIM_ICPolarity_Rising)
  {
    icoppositepolarity = TIM_ICPolarity_Falling;
  }
  else
  {
    icoppositepolarity = TIM_ICPolarity_Rising;
  }
  /* Select the Opposite Input */
  if (ic_selection == TIM_ICSelection_DirectTI)
  {
    icoppositeselection = TIM_ICSelection_IndirectTI;
  }
  else
  {
    icoppositeselection = TIM_ICSelection_DirectTI;
  }
  if (chn == tim_cc_chn1)
  {
    /* TI1 Configuration */
    TI1_Config( TIMx, ic_polarity, ic_selection, ic_filter );
    /* Set the Input Capture Prescaler value */
    TIM_SetIC1Prescaler(TIMx, ic_prescaler);
    /* TI2 Configuration */
    TI2_Config(TIMx, icoppositepolarity, icoppositeselection, ic_filter);
    /* Set the Input Capture Prescaler value */
    TIM_SetIC2Prescaler(TIMx, ic_prescaler);
  }
  else
  {
    /* TI2 Configuration */
    TI2_Config(TIMx, ic_polarity, ic_selection, ic_filter);
    /* Set the Input Capture Prescaler value */
    TIM_SetIC2Prescaler(TIMx, ic_prescaler);
    /* TI1 Configuration */
    TI1_Config(TIMx, icoppositepolarity, icoppositeselection, ic_filter);
    /* Set the Input Capture Prescaler value */
    TIM_SetIC1Prescaler(TIMx, ic_prescaler);
  }
}


/**
  * @brief  Enables or disables the specified TIM interrupts.
  * @param  TIMx: where x can be 1 to 17 to select the TIMx peripheral.
  * @param  TIM_IT: specifies the TIM interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg TIM_IT_Update: TIM update Interrupt source
  *     @arg TIM_IT_CC1: TIM Capture Compare 1 Interrupt source
  *     @arg TIM_IT_CC2: TIM Capture Compare 2 Interrupt source
  *     @arg TIM_IT_CC3: TIM Capture Compare 3 Interrupt source
  *     @arg TIM_IT_CC4: TIM Capture Compare 4 Interrupt source
  *     @arg TIM_IT_COM: TIM Commutation Interrupt source
  *     @arg TIM_IT_Trigger: TIM Trigger Interrupt source
  *     @arg TIM_IT_Break: TIM Break Interrupt source
  * @note
  *   - TIM6 and TIM7 can only generate an update interrupt.
  *   - TIM9, TIM12 and TIM15 can have only TIM_IT_Update, TIM_IT_CC1,
  *      TIM_IT_CC2 or TIM_IT_Trigger.
  *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_IT_Update or TIM_IT_CC1.
  *   - TIM_IT_Break is used only with TIM1, TIM8 and TIM15.
  *   - TIM_IT_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
  * @param  NewState: new state of the TIM interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_it_config(TIM_TypeDef* TIMx, uint16_t TIM_IT, bool enable)
{
  if (enable)
  {
    /* Enable the Interrupt sources */
    TIMx->DIER |= TIM_IT;
  }
  else
  {
    /* Disable the Interrupt sources */
    TIMx->DIER &= (uint16_t)~TIM_IT;
  }
}

/**
  * @brief  Configures the TIMx event to be generate by software.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_EventSource: specifies the event source.
  *   This parameter can be one or more of the following values:
  *     @arg TIM_EventSource_Update: Timer update Event source
  *     @arg TIM_EventSource_CC1: Timer Capture Compare 1 Event source
  *     @arg TIM_EventSource_CC2: Timer Capture Compare 2 Event source
  *     @arg TIM_EventSource_CC3: Timer Capture Compare 3 Event source
  *     @arg TIM_EventSource_CC4: Timer Capture Compare 4 Event source
  *     @arg TIM_EventSource_COM: Timer COM event source
  *     @arg TIM_EventSource_Trigger: Timer Trigger Event source
  *     @arg TIM_EventSource_Break: Timer Break event source
  * @note
  *   - TIM6 and TIM7 can only generate an update event.
  *   - TIM_EventSource_COM and TIM_EventSource_Break are used only with TIM1 and TIM8.
  * @retval None
  */
static inline void tim_generate_event(TIM_TypeDef* TIMx, uint16_t TIM_EventSource)
{
  /* Set the event sources */
  TIMx->EGR = TIM_EventSource;
}


/**
  * @brief  Configures the TIMx's DMA interface.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 15, 16 or 17 to select
  *   the TIM peripheral.
  * @param  TIM_DMABase: DMA Base address.
  *   This parameter can be one of the following values:
  *     @arg TIM_DMABase_CR, TIM_DMABase_CR2, TIM_DMABase_SMCR,
  *          TIM_DMABase_DIER, TIM1_DMABase_SR, TIM_DMABase_EGR,
  *          TIM_DMABase_CCMR1, TIM_DMABase_CCMR2, TIM_DMABase_CCER,
  *          TIM_DMABase_CNT, TIM_DMABase_PSC, TIM_DMABase_ARR,
  *          TIM_DMABase_RCR, TIM_DMABase_CCR1, TIM_DMABase_CCR2,
  *          TIM_DMABase_CCR3, TIM_DMABase_CCR4, TIM_DMABase_BDTR,
  *          TIM_DMABase_DCR.
  * @param  TIM_DMABurstLength: DMA Burst length.
  *   This parameter can be one value between:
  *   TIM_DMABurstLength_1Transfer and TIM_DMABurstLength_18Transfers.
  * @retval None
  */
static inline void tim_dma_config(TIM_TypeDef* TIMx, uint16_t TIM_DMABase, uint16_t TIM_DMABurstLength)
{
  /* Set the DMA Base and the DMA Burst Length */
  TIMx->DCR = TIM_DMABase | TIM_DMABurstLength;
}

/**
  * @brief  Enables or disables the TIMx's DMA Requests.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 6, 7, 8, 15, 16 or 17
  *   to select the TIM peripheral.
  * @param  TIM_DMASource: specifies the DMA Request sources.
  *   This parameter can be any combination of the following values:
  *     @arg TIM_DMA_Update: TIM update Interrupt source
  *     @arg TIM_DMA_CC1: TIM Capture Compare 1 DMA source
  *     @arg TIM_DMA_CC2: TIM Capture Compare 2 DMA source
  *     @arg TIM_DMA_CC3: TIM Capture Compare 3 DMA source
  *     @arg TIM_DMA_CC4: TIM Capture Compare 4 DMA source
  *     @arg TIM_DMA_COM: TIM Commutation DMA source
  *     @arg TIM_DMA_Trigger: TIM Trigger DMA source
  * @param  NewState: new state of the DMA Request sources.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_dma_cmd(TIM_TypeDef* TIMx, uint16_t TIM_DMASource, bool enable)
{

  if (enable)
  {
    /* Enable the DMA sources */
    TIMx->DIER |= TIM_DMASource;
  }
  else
  {
    /* Disable the DMA sources */
    TIMx->DIER &= (uint16_t)~TIM_DMASource;
  }
}

/**
  * @brief  Configures the TIMx internal Clock
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 9, 12 or 15
  *         to select the TIM peripheral.
  * @retval None
  */
static inline void tim_internal_clock_config(TIM_TypeDef* TIMx)
{
  /* Disable slave mode to clock the prescaler directly with the internal clock */
  TIMx->SMCR &=  (uint16_t)(~((uint16_t)TIM_SMCR_SMS));
}

/**
  * @brief  Selects the Input Trigger source
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_InputTriggerSource: The Input Trigger source.
  *   This parameter can be one of the following values:
  *     @arg TIM_TS_ITR0: Internal Trigger 0
  *     @arg TIM_TS_ITR1: Internal Trigger 1
  *     @arg TIM_TS_ITR2: Internal Trigger 2
  *     @arg TIM_TS_ITR3: Internal Trigger 3
  *     @arg TIM_TS_TI1F_ED: TI1 Edge Detector
  *     @arg TIM_TS_TI1FP1: Filtered Timer Input 1
  *     @arg TIM_TS_TI2FP2: Filtered Timer Input 2
  *     @arg TIM_TS_ETRF: External Trigger input
  * @retval None
  */
static inline void tim_select_input_trigger(TIM_TypeDef* TIMx, uint16_t TIM_InputTriggerSource)
{
  uint16_t tmpsmcr = 0;
  /* Get the TIMx SMCR register value */
  tmpsmcr = TIMx->SMCR;
  /* Reset the TS Bits */
  tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_TS));
  /* Set the Input Trigger source */
  tmpsmcr |= TIM_InputTriggerSource;
  /* Write to TIMx SMCR */
  TIMx->SMCR = tmpsmcr;
}


/**
  * @brief  Configures the TIMx Internal Trigger as External Clock
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_ITRSource: Trigger source.
  *   This parameter can be one of the following values:
  * @param  TIM_TS_ITR0: Internal Trigger 0
  * @param  TIM_TS_ITR1: Internal Trigger 1
  * @param  TIM_TS_ITR2: Internal Trigger 2
  * @param  TIM_TS_ITR3: Internal Trigger 3
  * @retval None
  */
static inline void tim_itrx_external_clock_config(TIM_TypeDef* TIMx, uint16_t TIM_InputTriggerSource)
{
  /* Select the Internal Trigger */
  tim_select_input_trigger(TIMx, TIM_InputTriggerSource);
  /* Select the External clock mode1 */
  TIMx->SMCR |= TIM_SlaveMode_External1;
}

/**
  * @brief  Configures the TIMx Trigger as External Clock
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_TIxExternalCLKSource: Trigger source.
  *   This parameter can be one of the following values:
  *     @arg TIM_TIxExternalCLK1Source_TI1ED: TI1 Edge Detector
  *     @arg TIM_TIxExternalCLK1Source_TI1: Filtered Timer Input 1
  *     @arg TIM_TIxExternalCLK1Source_TI2: Filtered Timer Input 2
  * @param  TIM_ICPolarity: specifies the TIx Polarity.
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPolarity_Rising
  *     @arg TIM_ICPolarity_Falling
  * @param  ICFilter : specifies the filter value.
  *   This parameter must be a value between 0x0 and 0xF.
  * @retval None
  */
static inline void tim_tix_external_clock_config(TIM_TypeDef* TIMx, uint16_t TIM_TIxExternalCLKSource,
              uint16_t TIM_ICPolarity, uint16_t ICFilter)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  /* Configure the Timer Input Clock Source */
  if (TIM_TIxExternalCLKSource == TIM_TIxExternalCLK1Source_TI2)
  {
    TI2_Config(TIMx, TIM_ICPolarity, TIM_ICSelection_DirectTI, ICFilter);
  }
  else
  {
    TI1_Config(TIMx, TIM_ICPolarity, TIM_ICSelection_DirectTI, ICFilter);
  }
  /* Select the Trigger source */
  tim_select_input_trigger(TIMx, TIM_TIxExternalCLKSource);
  /* Select the External clock mode1 */
  TIMx->SMCR |= TIM_SlaveMode_External1;
}


/**
  * @brief  Configures the TIMx External Trigger (ETR).
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_ExtTRGPrescaler: The external Trigger Prescaler.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPSC_OFF: ETRP Prescaler OFF.
  *     @arg TIM_ExtTRGPSC_DIV2: ETRP frequency divided by 2.
  *     @arg TIM_ExtTRGPSC_DIV4: ETRP frequency divided by 4.
  *     @arg TIM_ExtTRGPSC_DIV8: ETRP frequency divided by 8.
  * @param  TIM_ExtTRGPolarity: The external Trigger Polarity.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPolarity_Inverted: active low or falling edge active.
  *     @arg TIM_ExtTRGPolarity_NonInverted: active high or rising edge active.
  * @param  ExtTRGFilter: External Trigger Filter.
  *   This parameter must be a value between 0x00 and 0x0F
  * @retval None
  */
static inline void tim_etr_config(TIM_TypeDef* TIMx, uint16_t TIM_ExtTRGPrescaler, uint16_t TIM_ExtTRGPolarity,
                   uint16_t ExtTRGFilter)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  uint16_t tmpsmcr = 0;
  tmpsmcr = TIMx->SMCR;
  /* Reset the ETR Bits */
  tmpsmcr &= SMCR_ETR_Mask;
  /* Set the Prescaler, the Filter value and the Polarity */
  tmpsmcr |= (uint16_t)(TIM_ExtTRGPrescaler | (uint16_t)(TIM_ExtTRGPolarity | (uint16_t)(ExtTRGFilter << (uint16_t)8)));
  /* Write to TIMx SMCR */
  TIMx->SMCR = tmpsmcr;
}

/**
  * @brief  Configures the External clock Mode1
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_ExtTRGPrescaler: The external Trigger Prescaler.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPSC_OFF: ETRP Prescaler OFF.
  *     @arg TIM_ExtTRGPSC_DIV2: ETRP frequency divided by 2.
  *     @arg TIM_ExtTRGPSC_DIV4: ETRP frequency divided by 4.
  *     @arg TIM_ExtTRGPSC_DIV8: ETRP frequency divided by 8.
  * @param  TIM_ExtTRGPolarity: The external Trigger Polarity.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPolarity_Inverted: active low or falling edge active.
  *     @arg TIM_ExtTRGPolarity_NonInverted: active high or rising edge active.
  * @param  ExtTRGFilter: External Trigger Filter.
  *   This parameter must be a value between 0x00 and 0x0F
  * @retval None
  */
static inline void tim_etr_clock_mode1_config(TIM_TypeDef* TIMx,
		uint16_t TIM_ExtTRGPrescaler, uint16_t TIM_ExtTRGPolarity, uint16_t ExtTRGFilter)
{
  uint16_t tmpsmcr = 0;

  /* Configure the ETR Clock source */
  tim_etr_config(TIMx, TIM_ExtTRGPrescaler, TIM_ExtTRGPolarity, ExtTRGFilter);

  /* Get the TIMx SMCR register value */
  tmpsmcr = TIMx->SMCR;
  /* Reset the SMS Bits */
  tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_SMS));
  /* Select the External clock mode1 */
  tmpsmcr |= TIM_SlaveMode_External1;
  /* Select the Trigger selection : ETRF */
  tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_TS));
  tmpsmcr |= TIM_TS_ETRF;
  /* Write to TIMx SMCR */
  TIMx->SMCR = tmpsmcr;
}

/**
  * @brief  Configures the External clock Mode2
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_ExtTRGPrescaler: The external Trigger Prescaler.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPSC_OFF: ETRP Prescaler OFF.
  *     @arg TIM_ExtTRGPSC_DIV2: ETRP frequency divided by 2.
  *     @arg TIM_ExtTRGPSC_DIV4: ETRP frequency divided by 4.
  *     @arg TIM_ExtTRGPSC_DIV8: ETRP frequency divided by 8.
  * @param  TIM_ExtTRGPolarity: The external Trigger Polarity.
  *   This parameter can be one of the following values:
  *     @arg TIM_ExtTRGPolarity_Inverted: active low or falling edge active.
  *     @arg TIM_ExtTRGPolarity_NonInverted: active high or rising edge active.
  * @param  ExtTRGFilter: External Trigger Filter.
  *   This parameter must be a value between 0x00 and 0x0F
  * @retval None
  */
static inline void tim_etr_clock_mode2_config(TIM_TypeDef* TIMx, uint16_t TIM_ExtTRGPrescaler,
             uint16_t TIM_ExtTRGPolarity, uint16_t ExtTRGFilter)
{
  /* Configure the ETR Clock source */
  tim_etr_config(TIMx, TIM_ExtTRGPrescaler, TIM_ExtTRGPolarity, ExtTRGFilter);
  /* Enable the External clock mode2 */
  TIMx->SMCR |= TIM_SMCR_ECE;
}

/**
  * @brief  Configures the TIMx Prescaler.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  Prescaler: specifies the Prescaler Register value
  * @param  TIM_PSCReloadMode: specifies the TIM Prescaler Reload mode
  *   This parameter can be one of the following values:
  *     @arg TIM_PSCReloadMode_Update: The Prescaler is loaded at the update event.
  *     @arg TIM_PSCReloadMode_Immediate: The Prescaler is loaded immediately.
  * @retval None
  */
static inline void tim_prescaler_config(TIM_TypeDef* TIMx, uint16_t Prescaler, uint16_t TIM_PSCReloadMode)
{
  /* Set the Prescaler value */
  TIMx->PSC = Prescaler;
  /* Set or reset the UG Bit */
  TIMx->EGR = TIM_PSCReloadMode;
}

/**
  * @brief  Specifies the TIMx Counter Mode to be used.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_CounterMode: specifies the Counter Mode to be used
  *   This parameter can be one of the following values:
  *     @arg TIM_CounterMode_Up: TIM Up Counting Mode
  *     @arg TIM_CounterMode_Down: TIM Down Counting Mode
  *     @arg TIM_CounterMode_CenterAligned1: TIM Center Aligned Mode1
  *     @arg TIM_CounterMode_CenterAligned2: TIM Center Aligned Mode2
  *     @arg TIM_CounterMode_CenterAligned3: TIM Center Aligned Mode3
  * @retval None
  */
static inline void tim_counter_mode_config(TIM_TypeDef* TIMx, uint16_t TIM_CounterMode)
{
  uint16_t tmpcr1 = 0;
  tmpcr1 = TIMx->CR1;
  /* Reset the CMS and DIR Bits */
  tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS)));
  /* Set the Counter Mode */
  tmpcr1 |= TIM_CounterMode;
  /* Write to TIMx CR1 register */
  TIMx->CR1 = tmpcr1;
}

/**
  * @brief  Configures the TIMx Encoder Interface.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_EncoderMode: specifies the TIMx Encoder Mode.
  *   This parameter can be one of the following values:
  *     @arg TIM_EncoderMode_TI1: Counter counts on TI1FP1 edge depending on TI2FP2 level.
  *     @arg TIM_EncoderMode_TI2: Counter counts on TI2FP2 edge depending on TI1FP1 level.
  *     @arg TIM_EncoderMode_TI12: Counter counts on both TI1FP1 and TI2FP2 edges depending
  *                                on the level of the other input.
  * @param  TIM_IC1Polarity: specifies the IC1 Polarity
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPolarity_Falling: IC Falling edge.
  *     @arg TIM_ICPolarity_Rising: IC Rising edge.
  * @param  TIM_IC2Polarity: specifies the IC2 Polarity
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPolarity_Falling: IC Falling edge.
  *     @arg TIM_ICPolarity_Rising: IC Rising edge.
  * @retval None
  */
static inline void tim_encoder_interface_config(TIM_TypeDef* TIMx, uint16_t TIM_EncoderMode,
                                uint16_t TIM_IC1Polarity, uint16_t TIM_IC2Polarity)
{
  uint16_t tmpsmcr = 0;
  uint16_t tmpccmr1 = 0;
  uint16_t tmpccer = 0;


  /* Get the TIMx SMCR register value */
  tmpsmcr = TIMx->SMCR;

  /* Get the TIMx CCMR1 register value */
  tmpccmr1 = TIMx->CCMR1;

  /* Get the TIMx CCER register value */
  tmpccer = TIMx->CCER;

  /* Set the encoder Mode */
  tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_SMS));
  tmpsmcr |= TIM_EncoderMode;

  /* Select the Capture Compare 1 and the Capture Compare 2 as input */
  tmpccmr1 &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCMR1_CC1S)) & (uint16_t)(~((uint16_t)TIM_CCMR1_CC2S)));
  tmpccmr1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

  /* Set the TI1 and the TI2 Polarities */
  tmpccer &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCER_CC1P)) & ((uint16_t)~((uint16_t)TIM_CCER_CC2P)));
  tmpccer |= (uint16_t)(TIM_IC1Polarity | (uint16_t)(TIM_IC2Polarity << (uint16_t)4));

  /* Write to TIMx SMCR */
  TIMx->SMCR = tmpsmcr;
  /* Write to TIMx CCMR1 */
  TIMx->CCMR1 = tmpccmr1;
  /* Write to TIMx CCER */
  TIMx->CCER = tmpccer;
}

/**
  * @brief  Forces the TIMx output 1 waveform to active or inactive level.
  * @param  TIMx: where x can be  1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  TIM_ForcedAction: specifies the forced Action to be set to the output waveform.
  *   This parameter can be one of the following values:
  *     @arg TIM_ForcedAction_Active: Force active level on OC1REF
  *     @arg TIM_ForcedAction_InActive: Force inactive level on OC1REF.
  * @retval None
  */
static inline void tim_forced_oc_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn,uint16_t TIM_ForcedAction)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	ForcedOC1Config( TIMx, TIM_ForcedAction ); break;
		case tim_cc_chn2:	ForcedOC2Config( TIMx, TIM_ForcedAction ); break;
		case tim_cc_chn3:	ForcedOC3Config( TIMx, TIM_ForcedAction ); break;
		case tim_cc_chn4:	ForcedOC4Config( TIMx, TIM_ForcedAction ); break;
#ifdef TIM_CCER_CC6E
		case tim_cc_chn5:	ForcedOC5Config( TIMx, TIM_ForcedAction ); break;
		case tim_cc_chn6:	ForcedOC6Config( TIMx, TIM_ForcedAction ); break;
#endif
		default: break;
	}
}

/**
  * @brief  Enables or disables TIMx peripheral Preload register on ARR.
  * @param  TIMx: where x can be  1 to 17 to select the TIM peripheral.
  * @param  NewState: new state of the TIMx peripheral Preload register
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_arrp_reload_config(TIM_TypeDef* TIMx, bool enable)
{
  if (enable)
  {
    /* Set the ARR Preload Bit */
    TIMx->CR1 |= TIM_CR1_ARPE;
  }
  else
  {
    /* Reset the ARR Preload Bit */
    TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_ARPE);
  }
}

/**
  * @brief  Selects the TIM peripheral Commutation event.
  * @param  TIMx: where x can be  1, 8, 15, 16 or 17 to select the TIMx peripheral
  * @param  NewState: new state of the Commutation event.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_select_com(TIM_TypeDef* TIMx, bool enable)
{
  if (enable)
  {
    /* Set the COM Bit */
    TIMx->CR2 |= TIM_CR2_CCUS;
  }
  else
  {
    /* Reset the COM Bit */
    TIMx->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_CCUS);
  }
}

/**
  * @brief  Selects the TIMx peripheral Capture Compare DMA source.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 15, 16 or 17 to select
  *         the TIM peripheral.
  * @param  NewState: new state of the Capture Compare DMA source
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_select_ccdma(TIM_TypeDef* TIMx, bool enable)
{
  if ( enable )
  {
    /* Set the CCDS Bit */
    TIMx->CR2 |= TIM_CR2_CCDS;
  }
  else
  {
    /* Reset the CCDS Bit */
    TIMx->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_CCDS);
  }
}

/**
  * @brief  Sets or Resets the TIM peripheral Capture Compare Preload Control bit.
  * @param  TIMx: where x can be   1, 2, 3, 4, 5, 8 or 15
  *         to select the TIMx peripheral
  * @param  NewState: new state of the Capture Compare Preload Control bit
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_cc_preload_control(TIM_TypeDef* TIMx, bool enable)
{
  if ( enable )
  {
    /* Set the CCPC Bit */
    TIMx->CR2 |= TIM_CR2_CCPC;
  }
  else
  {
    /* Reset the CCPC Bit */
    TIMx->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_CCPC);
  }
}

/**
  * @brief  Configures the TIMx Output Compare 1 Fast feature.
  * @param  TIMx: where x can be  1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  TIM_OCFast: new state of the Output Compare Fast Enable Bit.
  *   This parameter can be one of the following values:
  *     @arg TIM_OCFast_Enable: TIM output compare fast enable
  *     @arg TIM_OCFast_Disable: TIM output compare fast disable
  * @retval None
  */
static inline void tim_oc_fast_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_OCFast)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	OC1FastConfig( TIMx, TIM_OCFast ); break;
		case tim_cc_chn2:	OC2FastConfig( TIMx, TIM_OCFast ); break;
		case tim_cc_chn3:	OC3FastConfig( TIMx, TIM_OCFast ); break;
		case tim_cc_chn4:	OC4FastConfig( TIMx, TIM_OCFast ); break;
		default: break;
	}
}

/**
  * @brief  Clears or safeguards the OCREF1 signal on an external event
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_OCClear: new state of the Output Compare Clear Enable Bit.
  *   This parameter can be one of the following values:
  *     @arg TIM_OCClear_Enable: TIM Output clear enable
  *     @arg TIM_OCClear_Disable: TIM Output clear disable
  * @retval None
  */
static inline void tim_clear_oc_ref(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_OCClear)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	ClearOC1Ref( TIMx, TIM_OCClear ); break;
		case tim_cc_chn2:	ClearOC2Ref( TIMx, TIM_OCClear ); break;
		case tim_cc_chn3:	ClearOC3Ref( TIMx, TIM_OCClear ); break;
		case tim_cc_chn4:	ClearOC4Ref( TIMx, TIM_OCClear ); break;
#ifdef TIM_CCER_CC6E
		case tim_cc_chn5:	ClearOC5Ref( TIMx, TIM_OCClear ); break;
		case tim_cc_chn6:	ClearOC6Ref( TIMx, TIM_OCClear ); break;
#endif
		default: break;
	}
}

/* @brief  Configures the TIMx channel x polarity.
* @param  TIMx: where x can be 1 to 17 except 6 and 7 to select the TIM peripheral.
* @param  TIM_OCPolarity: specifies the OC1 Polarity
*   This parameter can be one of the following values:
*     @arg TIM_OCPolarity_High: Output Compare active high
*     @arg TIM_OCPolarity_Low: Output Compare active low
* @retval None
*/
static inline void tim_oc_polarity_config(TIM_TypeDef* TIMx,enum tim_cc_chns chn, uint16_t TIM_OCPolarity)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	OC1PolarityConfig( TIMx, TIM_OCPolarity ); break;
		case tim_cc_chn2:	OC2PolarityConfig( TIMx, TIM_OCPolarity ); break;
		case tim_cc_chn3:	OC3PolarityConfig( TIMx, TIM_OCPolarity ); break;
		case tim_cc_chn4:	OC4PolarityConfig( TIMx, TIM_OCPolarity ); break;
#ifdef TIM_CCER_CC6E
		case tim_cc_chn5:	OC5PolarityConfig( TIMx, TIM_OCPolarity ); break;
		case tim_cc_chn6:	OC6PolarityConfig( TIMx, TIM_OCPolarity ); break;
#endif
		default: break;
	}
}

/**
  * @brief  Configures the TIMx Channel 1N polarity.
  * @param  TIMx: where x can be 1, 8, 15, 16 or 17 to select the TIM peripheral.
  * @param  TIM_OCNPolarity: specifies the OC1N Polarity
  *   This parameter can be one of the following values:
  *     @arg TIM_OCNPolarity_High: Output Compare active high
  *     @arg TIM_OCNPolarity_Low: Output Compare active low
  * @retval None
  */
static inline void tim_ocn_polarity_config(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_OCNPolarity)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	OC1NPolarityConfig( TIMx, TIM_OCNPolarity ); break;
		case tim_cc_chn2:	OC2NPolarityConfig( TIMx, TIM_OCNPolarity ); break;
		case tim_cc_chn3:	OC3NPolarityConfig( TIMx, TIM_OCNPolarity ); break;
		default:	break;
	}
}

/**
  * @brief  Enables or disables the TIM Capture Compare Channel x.
  * @param  TIMx: where x can be 1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  TIM_Channel: specifies the TIM Channel
  *   This parameter can be one of the following values:
  *     @arg TIM_Channel_1: TIM Channel 1
  *     @arg TIM_Channel_2: TIM Channel 2
  *     @arg TIM_Channel_3: TIM Channel 3
  *     @arg TIM_Channel_4: TIM Channel 4
  * @param  TIM_CCx: specifies the TIM Channel CCxE bit new state.
  *   This parameter can be: TIM_CCx_Enable or TIM_CCx_Disable.
  * @retval None
  */
static inline void tim_ccx_cmd(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_CCx)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  stm32tim_oc_t tmp;
  uint16_t TIM_Channel;
  switch( chn )
  {
  		case tim_cc_chn1:	TIM_Channel = TIM_Channel_1; break;
  		case tim_cc_chn2:	TIM_Channel = TIM_Channel_2; break;
  		case tim_cc_chn3:	TIM_Channel = TIM_Channel_3; break;
  		case tim_cc_chn4:	TIM_Channel = TIM_Channel_4; break;
#ifdef TIM_Channel_6
  		case tim_cc_chn5:	TIM_Channel = TIM_Channel_5; break;
  		case tim_cc_chn6:	TIM_Channel = TIM_Channel_6; break;
#endif
		default: return;
  }
  tmp = CCER_CCE_Set << TIM_Channel;

  /* Reset the CCxE Bit */
  TIMx->CCER &= (stm32tim_oc_t)~ tmp;

  /* Set or reset the CCxE Bit */
  TIMx->CCER |=  (stm32tim_oc_t)(TIM_CCx << TIM_Channel);
}

/**
  * @brief  Enables or disables the TIM Capture Compare Channel xN.
  * @param  TIMx: where x can be 1, 8, 15, 16 or 17 to select the TIM peripheral.
  * @param  TIM_Channel: specifies the TIM Channel
  *   This parameter can be one of the following values:
  *     @arg TIM_Channel_1: TIM Channel 1
  *     @arg TIM_Channel_2: TIM Channel 2
  *     @arg TIM_Channel_3: TIM Channel 3
  * @param  TIM_CCxN: specifies the TIM Channel CCxNE bit new state.
  *   This parameter can be: TIM_CCxN_Enable or TIM_CCxN_Disable.
  * @retval None
  */
static inline void tim_ccx_n_cmd(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_CCxN)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  stm32tim_oc_t tmp = 0;
  uint16_t TIM_Channel;
   switch( chn )
   {
    	case tim_cc_chn1:	TIM_Channel = TIM_Channel_1; break;
    	case tim_cc_chn2:	TIM_Channel = TIM_Channel_2; break;
    	case tim_cc_chn3:	TIM_Channel = TIM_Channel_3; break;
		default: return;
   }
  tmp = CCER_CCNE_Set << TIM_Channel;

  /* Reset the CCxNE Bit */
  TIMx->CCER &= (stm32tim_oc_t) ~tmp;

  /* Set or reset the CCxNE Bit */
  TIMx->CCER |=  (stm32tim_oc_t)(TIM_CCxN << TIM_Channel);
}


/**
  * @brief  Selects the TIM Output Compare Mode.
  * @note   This function disables the selected channel before changing the Output
  *         Compare Mode.
  *         User has to enable this channel using TIM_CCxCmd and TIM_CCxNCmd functions.
  * @param  TIMx: where x can be 1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  TIM_Channel: specifies the TIM Channel
  *   This parameter can be one of the following values:
  *     @arg TIM_Channel_1: TIM Channel 1
  *     @arg TIM_Channel_2: TIM Channel 2
  *     @arg TIM_Channel_3: TIM Channel 3
  *     @arg TIM_Channel_4: TIM Channel 4
  * @param  TIM_OCMode: specifies the TIM Output Compare Mode.
  *   This parameter can be one of the following values:
  *     @arg TIM_OCMode_Timing
  *     @arg TIM_OCMode_Active
  *     @arg TIM_OCMode_Toggle
  *     @arg TIM_OCMode_PWM1
  *     @arg TIM_OCMode_PWM2
  *     @arg TIM_ForcedAction_Active
  *     @arg TIM_ForcedAction_InActive
  * @retval None
  */
static inline void tim_select_ocxm(TIM_TypeDef* TIMx,enum tim_cc_chns chn, uint32_t TIM_OCMode)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
  uint16_t TIM_Channel;
  uint32_t tmp = 0;
  uint16_t tmp1 = 0;
  switch( chn )
  {
    		case tim_cc_chn1:	TIM_Channel = TIM_Channel_1; break;
    		case tim_cc_chn2:	TIM_Channel = TIM_Channel_2; break;
    		case tim_cc_chn3:	TIM_Channel = TIM_Channel_3; break;
    		case tim_cc_chn4:	TIM_Channel = TIM_Channel_4; break;
			default: return;
   }
  tmp = (uint32_t) TIMx;
  tmp += CCMR_Offset;

  tmp1 = CCER_CCE_Set << (uint16_t)TIM_Channel;

  /* Disable the Channel: Reset the CCxE Bit */
  TIMx->CCER &= (uint16_t) ~tmp1;

  if((TIM_Channel == TIM_Channel_1) ||(TIM_Channel == TIM_Channel_3))
  {
    tmp += (TIM_Channel>>1);

    /* Reset the OCxM bits in the CCMRx register */
    *(__IO uint32_t *) tmp &= (uint32_t)~((uint32_t)TIM_CCMR1_OC1M);

    /* Configure the OCxM bits in the CCMRx register */
    *(__IO uint32_t *) tmp |= TIM_OCMode;
  }
  else
  {
    tmp += (uint16_t)(TIM_Channel - (uint16_t)4)>> (uint16_t)1;

    /* Reset the OCxM bits in the CCMRx register */
    *(__IO uint32_t *) tmp &= (uint32_t)~((uint32_t)TIM_CCMR1_OC2M);

    /* Configure the OCxM bits in the CCMRx register */
    *(__IO uint32_t *) tmp |= (uint32_t)(TIM_OCMode << 8);
  }
}



/**
  * @brief  Enables or Disables the TIMx Update event.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  NewState: new state of the TIMx UDIS bit
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_update_disable_config(TIM_TypeDef* TIMx, bool enable)
{
  if (enable)
  {
    /* Set the Update Disable Bit */
    TIMx->CR1 |= TIM_CR1_UDIS;
  }
  else
  {
    /* Reset the Update Disable Bit */
    TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_UDIS);
  }
}

/**
  * @brief  Configures the TIMx Update Request Interrupt source.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_UpdateSource: specifies the Update source.
  *   This parameter can be one of the following values:
  *     @arg TIM_UpdateSource_Regular: Source of update is the counter overflow/underflow
                                       or the setting of UG bit, or an update generation
                                       through the slave mode controller.
  *     @arg TIM_UpdateSource_Global: Source of update is counter overflow/underflow.
  * @retval None
  */
static inline void tim_update_request_config(TIM_TypeDef* TIMx, uint16_t TIM_UpdateSource)
{

  if (TIM_UpdateSource != TIM_UpdateSource_Global)
  {
    /* Set the URS Bit */
    TIMx->CR1 |= TIM_CR1_URS;
  }
  else
  {
    /* Reset the URS Bit */
    TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_URS);
  }
}

/**
  * @brief  Enables or disables the TIMx's Hall sensor interface.
  * @param  TIMx: where x can be 1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  NewState: new state of the TIMx Hall sensor interface.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_select_hall_sensor(TIM_TypeDef* TIMx, bool enable)
{
  if (enable)
  {
    /* Set the TI1S Bit */
    TIMx->CR2 |= TIM_CR2_TI1S;
  }
  else
  {
    /* Reset the TI1S Bit */
    TIMx->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_TI1S);
  }
}


/**
  * @brief  Selects the TIMx's One Pulse Mode.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_OPMode: specifies the OPM Mode to be used.
  *   This parameter can be one of the following values:
  *     @arg TIM_OPMode_Single
  *     @arg TIM_OPMode_Repetitive
  * @retval None
  */
static inline void tim_select_one_pulse_mode(TIM_TypeDef* TIMx, uint16_t TIM_OPMode)
{
  /* Reset the OPM Bit */
  TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_OPM);
  /* Configure the OPM Mode */
  TIMx->CR1 |= TIM_OPMode;
}

/**
  * @brief  Selects the TIMx Trigger Output Mode.
  * @param  TIMx: where x can be 1, 2, 3, 4, 5, 6, 7, 8, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_TRGOSource: specifies the Trigger Output source.
  *   This paramter can be one of the following values:
  *
  *  - For all TIMx
  *     @arg TIM_TRGOSource_Reset:  The UG bit in the TIM_EGR register is used as the trigger output (TRGO).
  *     @arg TIM_TRGOSource_Enable: The Counter Enable CEN is used as the trigger output (TRGO).
  *     @arg TIM_TRGOSource_Update: The update event is selected as the trigger output (TRGO).
  *
  *  - For all TIMx except TIM6 and TIM7
  *     @arg TIM_TRGOSource_OC1: The trigger output sends a positive pulse when the CC1IF flag
  *                              is to be set, as soon as a capture or compare match occurs (TRGO).
  *     @arg TIM_TRGOSource_OC1Ref: OC1REF signal is used as the trigger output (TRGO).
  *     @arg TIM_TRGOSource_OC2Ref: OC2REF signal is used as the trigger output (TRGO).
  *     @arg TIM_TRGOSource_OC3Ref: OC3REF signal is used as the trigger output (TRGO).
  *     @arg TIM_TRGOSource_OC4Ref: OC4REF signal is used as the trigger output (TRGO).
  *
  * @retval None
  */
static inline void tim_select_output_trigger(TIM_TypeDef* TIMx, uint16_t TIM_TRGOSource)
{
  /* Reset the MMS Bits */
  TIMx->CR2 &= (uint16_t)~((uint16_t)TIM_CR2_MMS);
  /* Select the TRGO source */
  TIMx->CR2 |=  TIM_TRGOSource;
}

#if defined(STM32MCU_MAJOR_TYPE_F3)
static inline void tim_select_output_trigger2(TIM_TypeDef* TIMx, uint32_t TIM_TRGO2Source)
{
  /* Reset the MMS Bits */
  TIMx->CR2 &= (uint32_t)~TIM_CR2_MMS2;
  /* Select the TRGO source */
  TIMx->CR2 |=  TIM_TRGO2Source;
}
#endif

/**
  * @brief  Selects the TIMx Slave Mode.
  * @param  TIMx: where x can be 1, 2, 3, 4, 5, 8, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_SlaveMode: specifies the Timer Slave Mode.
  *   This parameter can be one of the following values:
  *     @arg TIM_SlaveMode_Reset: Rising edge of the selected trigger signal (TRGI) re-initializes
  *                               the counter and triggers an update of the registers.
  *     @arg TIM_SlaveMode_Gated:     The counter clock is enabled when the trigger signal (TRGI) is high.
  *     @arg TIM_SlaveMode_Trigger:   The counter starts at a rising edge of the trigger TRGI.
  *     @arg TIM_SlaveMode_External1: Rising edges of the selected trigger (TRGI) clock the counter.
  * @retval None
  */
static inline void tim_select_slave_mode(TIM_TypeDef* TIMx, uint16_t TIM_SlaveMode)
{
 /* Reset the SMS Bits */
  TIMx->SMCR &= (uint16_t)~((uint16_t)TIM_SMCR_SMS);
  /* Select the Slave Mode */
  TIMx->SMCR |= TIM_SlaveMode;
}

/**
  * @brief  Sets or Resets the TIMx Master/Slave Mode.
  * @param  TIMx: where x can be 1, 2, 3, 4, 5, 8, 9, 12 or 15 to select the TIM peripheral.
  * @param  TIM_MasterSlaveMode: specifies the Timer Master Slave Mode.
  *   This parameter can be one of the following values:
  *     @arg TIM_MasterSlaveMode_Enable: synchronization between the current timer
  *                                      and its slaves (through TRGO).
  *     @arg TIM_MasterSlaveMode_Disable: No action
  * @retval None
  */
static inline void tim_select_master_slave_mode(TIM_TypeDef* TIMx, uint16_t TIM_MasterSlaveMode)
{
  /* Reset the MSM Bit */
  TIMx->SMCR &= (uint16_t)~((uint16_t)TIM_SMCR_MSM);

  /* Set or Reset the MSM Bit */
  TIMx->SMCR |= TIM_MasterSlaveMode;
}

/**
  * @brief  Sets the TIMx Counter Register value
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  Counter: specifies the Counter register new value.
  * @retval None
  */
static inline void tim_set_counter(TIM_TypeDef* TIMx, uint16_t Counter)
{
  /* Set the Counter Register value */
  TIMx->CNT = Counter;
}

/**
  * @brief  Sets the TIMx Autoreload Register value
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  Autoreload: specifies the Autoreload register new value.
  * @retval None
  */
static inline void tim_set_auto_reload(TIM_TypeDef* TIMx, uint16_t Autoreload)
{
  /* Set the Autoreload Register value */
  TIMx->ARR = Autoreload;
}

/**
  * @brief  Sets the TIMx Input Capture 1 prescaler.
  * @param  TIMx: where x can be 1 to 17 except 6 and 7 to select the TIM peripheral.
  * @param  TIM_ICPSC: specifies the Input Capture1 prescaler new value.
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPSC_DIV1: no prescaler
  *     @arg TIM_ICPSC_DIV2: capture is done once every 2 events
  *     @arg TIM_ICPSC_DIV4: capture is done once every 4 events
  *     @arg TIM_ICPSC_DIV8: capture is done once every 8 events
  * @retval None
  */
static inline void tim_set_ic_prescaler(TIM_TypeDef* TIMx, enum tim_cc_chns chn, uint16_t TIM_ICPSC)
{
#ifdef __cplusplus
	using namespace _internal::tim;
#endif
	switch( chn )
	{
		case tim_cc_chn1:	SetIC1Prescaler( TIMx, TIM_ICPSC); break;
		case tim_cc_chn2:	SetIC2Prescaler( TIMx, TIM_ICPSC); break;
		case tim_cc_chn3:	SetIC3Prescaler( TIMx, TIM_ICPSC); break;
		case tim_cc_chn4:	SetIC4Prescaler( TIMx, TIM_ICPSC); break;
		default: return;
	}
}

/**
  * @brief  Sets the TIMx Clock Division value.
  * @param  TIMx: where x can be  1 to 17 except 6 and 7 to select
  *   the TIM peripheral.
  * @param  TIM_CKD: specifies the clock division value.
  *   This parameter can be one of the following value:
  *     @arg TIM_CKD_DIV1: TDTS = Tck_tim
  *     @arg TIM_CKD_DIV2: TDTS = 2*Tck_tim
  *     @arg TIM_CKD_DIV4: TDTS = 4*Tck_tim
  * @retval None
  */
static inline void tim_set_clock_division(TIM_TypeDef* TIMx, uint16_t TIM_CKD)
{
  /* Reset the CKD Bits */
  TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_CKD);
  /* Set the CKD value */
  TIMx->CR1 |= TIM_CKD;
}


/**
  * @brief  Gets the TIMx Input Capture 1 value.
  * @param  TIMx: where x can be 1 to 17 except 6 and 7 to select the TIM peripheral.
  * @retval Capture Compare 1 Register value.
*/
static inline uint16_t tim_get_ccr(TIM_TypeDef* TIMx, enum tim_cc_chns chn)
{
	switch( chn )
	{
		case tim_cc_chn1:	return TIMx->CCR1;
		case tim_cc_chn2:	return TIMx->CCR2;
		case tim_cc_chn3:	return TIMx->CCR3;
		case tim_cc_chn4:	return TIMx->CCR4;
		default: 			return 0;
	}
}

/**
  * @brief  Gets the TIMx Counter value.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @retval Counter Register value.
  */
static inline uint16_t tim_get_counter(TIM_TypeDef* TIMx)
{
  /* Get the Counter Register value */
  return TIMx->CNT;
}

/**
  * @brief  Gets the TIMx Prescaler value.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @retval Prescaler Register value.
  */
static inline uint16_t tim_get_prescaler(TIM_TypeDef* TIMx)
{
  /* Get the Prescaler Register value */
  return TIMx->PSC;
}

/**
  * @brief  Checks whether the specified TIM flag is set or not.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg TIM_FLAG_Update: TIM update Flag
  *     @arg TIM_FLAG_CC1: TIM Capture Compare 1 Flag
  *     @arg TIM_FLAG_CC2: TIM Capture Compare 2 Flag
  *     @arg TIM_FLAG_CC3: TIM Capture Compare 3 Flag
  *     @arg TIM_FLAG_CC4: TIM Capture Compare 4 Flag
  *     @arg TIM_FLAG_COM: TIM Commutation Flag
  *     @arg TIM_FLAG_Trigger: TIM Trigger Flag
  *     @arg TIM_FLAG_Break: TIM Break Flag
  *     @arg TIM_FLAG_CC1OF: TIM Capture Compare 1 overcapture Flag
  *     @arg TIM_FLAG_CC2OF: TIM Capture Compare 2 overcapture Flag
  *     @arg TIM_FLAG_CC3OF: TIM Capture Compare 3 overcapture Flag
  *     @arg TIM_FLAG_CC4OF: TIM Capture Compare 4 overcapture Flag
  * @note
  *   - TIM6 and TIM7 can have only one update flag.
  *   - TIM9, TIM12 and TIM15 can have only TIM_FLAG_Update, TIM_FLAG_CC1,
  *      TIM_FLAG_CC2 or TIM_FLAG_Trigger.
  *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_FLAG_Update or TIM_FLAG_CC1.
  *   - TIM_FLAG_Break is used only with TIM1, TIM8 and TIM15.
  *   - TIM_FLAG_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
  * @retval The new state of TIM_FLAG (SET or RESET).
  */
static inline bool tim_get_flag_status(TIM_TypeDef* TIMx, uint16_t TIM_FLAG)
{
	return (TIMx->SR & TIM_FLAG)?true:false;
}

/**
  * @brief  Clears the TIMx's pending flags.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_FLAG: specifies the flag bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg TIM_FLAG_Update: TIM update Flag
  *     @arg TIM_FLAG_CC1: TIM Capture Compare 1 Flag
  *     @arg TIM_FLAG_CC2: TIM Capture Compare 2 Flag
  *     @arg TIM_FLAG_CC3: TIM Capture Compare 3 Flag
  *     @arg TIM_FLAG_CC4: TIM Capture Compare 4 Flag
  *     @arg TIM_FLAG_COM: TIM Commutation Flag
  *     @arg TIM_FLAG_Trigger: TIM Trigger Flag
  *     @arg TIM_FLAG_Break: TIM Break Flag
  *     @arg TIM_FLAG_CC1OF: TIM Capture Compare 1 overcapture Flag
  *     @arg TIM_FLAG_CC2OF: TIM Capture Compare 2 overcapture Flag
  *     @arg TIM_FLAG_CC3OF: TIM Capture Compare 3 overcapture Flag
  *     @arg TIM_FLAG_CC4OF: TIM Capture Compare 4 overcapture Flag
  * @note
  *   - TIM6 and TIM7 can have only one update flag.
  *   - TIM9, TIM12 and TIM15 can have only TIM_FLAG_Update, TIM_FLAG_CC1,
  *      TIM_FLAG_CC2 or TIM_FLAG_Trigger.
  *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_FLAG_Update or TIM_FLAG_CC1.
  *   - TIM_FLAG_Break is used only with TIM1, TIM8 and TIM15.
  *   - TIM_FLAG_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
  * @retval None
  */
static inline void tim_clear_flag(TIM_TypeDef* TIMx, uint16_t TIM_FLAG)
{
  /* Clear the flags */
  TIMx->SR = (uint16_t)~TIM_FLAG;
}

/**
  * @brief  Checks whether the TIM interrupt has occurred or not.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_IT: specifies the TIM interrupt source to check.
  *   This parameter can be one of the following values:
  *     @arg TIM_IT_Update: TIM update Interrupt source
  *     @arg TIM_IT_CC1: TIM Capture Compare 1 Interrupt source
  *     @arg TIM_IT_CC2: TIM Capture Compare 2 Interrupt source
  *     @arg TIM_IT_CC3: TIM Capture Compare 3 Interrupt source
  *     @arg TIM_IT_CC4: TIM Capture Compare 4 Interrupt source
  *     @arg TIM_IT_COM: TIM Commutation Interrupt source
  *     @arg TIM_IT_Trigger: TIM Trigger Interrupt source
  *     @arg TIM_IT_Break: TIM Break Interrupt source
  * @note
  *   - TIM6 and TIM7 can generate only an update interrupt.
  *   - TIM9, TIM12 and TIM15 can have only TIM_IT_Update, TIM_IT_CC1,
  *      TIM_IT_CC2 or TIM_IT_Trigger.
  *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_IT_Update or TIM_IT_CC1.
  *   - TIM_IT_Break is used only with TIM1, TIM8 and TIM15.
  *   - TIM_IT_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
  * @retval The new state of the TIM_IT(SET or RESET).
  */
static inline bool tim_get_it_status(TIM_TypeDef* TIMx, uint16_t TIM_IT)
{
  uint16_t itstatus, itenable;

  itstatus = TIMx->SR & TIM_IT;
  itenable = TIMx->DIER & TIM_IT;
  return itstatus !=0 && itenable !=0;
}


/**
  * @brief  Clears the TIMx's interrupt pending bits.
  * @param  TIMx: where x can be 1 to 17 to select the TIM peripheral.
  * @param  TIM_IT: specifies the pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg TIM_IT_Update: TIM1 update Interrupt source
  *     @arg TIM_IT_CC1: TIM Capture Compare 1 Interrupt source
  *     @arg TIM_IT_CC2: TIM Capture Compare 2 Interrupt source
  *     @arg TIM_IT_CC3: TIM Capture Compare 3 Interrupt source
  *     @arg TIM_IT_CC4: TIM Capture Compare 4 Interrupt source
  *     @arg TIM_IT_COM: TIM Commutation Interrupt source
  *     @arg TIM_IT_Trigger: TIM Trigger Interrupt source
  *     @arg TIM_IT_Break: TIM Break Interrupt source
  * @note
  *   - TIM6 and TIM7 can generate only an update interrupt.
  *   - TIM9, TIM12 and TIM15 can have only TIM_IT_Update, TIM_IT_CC1,
  *      TIM_IT_CC2 or TIM_IT_Trigger.
  *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_IT_Update or TIM_IT_CC1.
  *   - TIM_IT_Break is used only with TIM1, TIM8 and TIM15.
  *   - TIM_IT_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
  * @retval None
  */
static inline void tim_clear_it_pending_bit(TIM_TypeDef* TIMx, uint16_t TIM_IT)
{
  /* Clear the IT pending Bit */
  TIMx->SR = (uint16_t)~TIM_IT;
}


#if defined(STM32MCU_MAJOR_TYPE_F4)
/**
  * @brief  Configures the TIM2, TIM5 and TIM11 Remapping input capabilities.
  * @param  TIMx: where x can be 2, 5 or 11 to select the TIM peripheral.
  * @param  TIM_Remap: specifies the TIM input remapping source.
  *          This parameter can be one of the following values:
  *            @arg TIM2_TIM8_TRGO: TIM2 ITR1 input is connected to TIM8 Trigger output(default)
  *            @arg TIM2_ETH_PTP:   TIM2 ITR1 input is connected to ETH PTP trogger output.
  *            @arg TIM2_USBFS_SOF: TIM2 ITR1 input is connected to USB FS SOF.
  *            @arg TIM2_USBHS_SOF: TIM2 ITR1 input is connected to USB HS SOF.
  *            @arg TIM5_GPIO:      TIM5 CH4 input is connected to dedicated Timer pin(default)
  *            @arg TIM5_LSI:       TIM5 CH4 input is connected to LSI clock.
  *            @arg TIM5_LSE:       TIM5 CH4 input is connected to LSE clock.
  *            @arg TIM5_RTC:       TIM5 CH4 input is connected to RTC Output event.
  *            @arg TIM11_GPIO:     TIM11 CH4 input is connected to dedicated Timer pin(default)
  *            @arg TIM11_HSE:      TIM11 CH4 input is connected to HSE_RTC clock
  *                                 (HSE divided by a programmable prescaler)
  * @retval None
  */
static inline void tim_remap_config(TIM_TypeDef* TIMx, uint16_t TIM_Remap)
{
  /* Set the Timer remapping configuration */
  TIMx->OR =  TIM_Remap;
}

#endif

#if defined(STM32MCU_MAJOR_TYPE_F3)
/**
  * @brief  Sets or resets the update interrupt flag (UIF)status bit Remapping.
  *         when sets, reading TIMx_CNT register returns UIF bit instead of CNT[31]  
  * @param  TIMx: where x can be 1, 2, 3, 4, 6 ,7 ,8, 15, 16 or 17 to select the TIM peripheral.
  * @param  NewState: new state of the UIFREMAP bit.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_uif_remap(TIM_TypeDef* TIMx, bool enable )
{
  
  if (enable)
  {
    /* Enable the TIM Counter */
    TIMx->CR1 |= TIM_CR1_UIFREMAP;
  }
  else
  {
    /* Disable the TIM Counter */
    TIMx->CR1 &= (uint16_t)~TIM_CR1_UIFREMAP;
  }  
}

/**
  * @brief  Selects the TIM Group Channel 5 and Channel 1, 
            OC1REFC is the logical AND of OC1REFC and OC5REF.
  * @param  TIMx: where x can be  1 or 8 to select the TIMx peripheral
  * @param  NewState: new state of the Commutation event.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_select_GC5C1(TIM_TypeDef* TIMx, bool enable )
{

  if (enable)
  {
    /* Set the GC5C1 Bit */
    TIMx->CCR5 |= TIM_CCR5_GC5C1;
  }
  else
  {
    /* Reset the GC5C1 Bit */
    TIMx->CCR5 &= (uint32_t)~TIM_CCR5_GC5C1;
  }
}

/**
  * @brief  Selects the TIM Group Channel 5 and Channel 2, 
            OC2REFC is the logical AND of OC2REFC and OC5REF.
  * @param  TIMx: where x can be  1 or 8 to select the TIMx peripheral
  * @param  NewState: new state of the Commutation event.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void tim_select_GC5C2(TIM_TypeDef* TIMx, bool enable )
{

  if (enable)
  {
    /* Set the GC5C2 Bit */
    TIMx->CCR5 |= TIM_CCR5_GC5C2;
  }
  else
  {
    /* Reset the GC5C2 Bit */
    TIMx->CCR5 &= (uint32_t)~TIM_CCR5_GC5C2;
  }
}


#endif /*  defined(STM32MCU_MAJOR_TYPE_F3) */



#ifdef __cplusplus
 }
#endif


#ifndef __cplusplus
#undef  SMCR_ETR_Mask
#undef  CCER_CCE_Set
#undef  CCER_CCNE_Set
#undef  CCMR_Offset
#endif




#endif /* STM32TIM_H_ */
