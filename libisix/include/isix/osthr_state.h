/*
 * =====================================================================================
 *
 *       Filename:  osthr_state.h
 *
 *    Description: OSTHR state
 *
 *        Version:  1.0
 *        Created:  10.04.2015 17:53:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

//! Current thread state
enum osthr_state 
{
	OSTHR_STATE_READY		= 0,			//! Thread is on ready state
	OSTHR_STATE_RUNNING		= 1,			//! Thread is in running state
	OSTHR_STATE_CREATED		= 2,			//! Task already created but 
	OSTHR_STATE_SLEEPING	= 3,			//! Thread on sleeping state
	OSTHR_STATE_WTSEM		= 4,			//! Wait for semaphore state
	OSTHR_STATE_WTEXIT		= 5,			//! Wait for exit state
	OSTHR_STATE_ZOMBIE		= 6,			//! In zombie state just before exit
	OSTHR_STATE_SCHEDULE  	= 7,			//! Schedule only do nothing special
	OSTHR_STATE_WTEVT	    = 8, 			//! Scheduler on wait event state
	OSTHR_STATE_SUSPEND		= 9				//! Task is in suspend state
};

