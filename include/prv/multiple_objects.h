/*--------------------------------------------------------------*/
/*
 * multiple_objects.h
 *
 *  Created on: 26-03-2011
 *      Author: lucck
 */
/*--------------------------------------------------------------*/
#ifndef ISIX_PRV_MULTIPLE_OBJECTS_H_
#define ISIX_PRV_MULTIPLE_OBJECTS_H_
/*--------------------------------------------------------------*/

#ifdef ISIX_CONFIG_USE_MULTIOBJECTS	//If multiobjects enable

/*--------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/scheduler.h>

/*--------------------------------------------------------------*/
struct multiple_obj
{
	const union ihandle *ihwnd;	//Ihandle pointer
	size_t ihwnd_len;		//Ihandle len
	task_t *task_id;		//Task owner
	list_t inode;			//Multiple objects inode
};

/*--------------------------------------------------------------*/
//Wait for multi object
typedef struct multiple_obj multiple_obj_t;


/*--------------------------------------------------------------*/
//Initialize the multiple objects waiting
void ixixp_multiple_objects_init( void );

/*--------------------------------------------------------------*/
//Wakeup multiple wating tasks
int isixp_wakeup_multiple_waiting_tasks( const sem_t *sem, int (*wkup_fun)(task_t *) );

/*--------------------------------------------------------------*/

void isixp_delete_from_multiple_wait_list(const task_t *task );

/*--------------------------------------------------------------*/
#else /*ISIX_CONFIG_USE_MULTIOBJECTS */

static inline void ixixp_multiple_objects_init( void ) {}

#endif /*ISIX_CONFIG_USE_MULTIOBJECTS */

#endif /* MULTIPLE_OBJECTS_H_ */
