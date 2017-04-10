#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* Port yield function for the RTOS */
void port_yield( void );

/* Port start first task */
void port_start_first_task( void );

/* Port set interrupt mask */
void port_set_interrupt_mask( void );

/* Port clear interrupt mask */
void port_clear_interrupt_mask( void );


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


