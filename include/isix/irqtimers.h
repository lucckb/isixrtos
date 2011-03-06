/*
 * irq_timers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/

#ifndef ISIX_IRQTIMERS_H_
#define ISIX_IRQTIMERS_H_
#ifdef ISIX_CONFIG_USE_TIMERS
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
/* Structure definition for hidding the type */
struct vtimer_struct;
typedef struct vtimer_struct vtimer_t;

/*-----------------------------------------------------------------------*/
/** Create virtual timer called from interrupt context for light events
 * consume much power cyclic tasks
 * @param[in] func Function to be called in context - cyclic timeout
 * @param[in] arg Frgument passed to the function
 * @return NULL if timer create was unsucess, else return vtimer pointer
 */
vtimer_t* isix_vtimer_create(void (*func)(void*),void *arg );
/*-----------------------------------------------------------------------*/
/** Start the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @param[in] timeout Timeout for the next start
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_start(vtimer_t* timer, tick_t timeout);
/*-----------------------------------------------------------------------*/
/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
static inline int isix_vtimer_stop(vtimer_t* timer)
{
	return isix_vtimer_start( timer, 0 );
}
/*-----------------------------------------------------------------------*/
/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy(vtimer_t* timer);

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------------------*/
/****************************** C++ API ***********************************/
#ifdef __cplusplus
#include <cstddef>

namespace isix {
/*-----------------------------------------------------------------------*/
//! C++ wrapper for the vtime
class virtual_timer
{
public:
	//! Create virtual timer object
	virtual_timer()
	{
		timer = isix_vtimer_create( vtimer_func, this );
	}
	//! Destroy the virtual timer object
	~virtual_timer()
	{
		isix_vtimer_destroy( timer );
	}
	//! Check that object is valid
	bool is_valid() { return timer!=0; }
	//! Start the timer on selected period
	int start(tick_t timeout) { return isix_vtimer_start( timer, timeout ); }
	int stop() { return isix_vtimer_stop( timer ); }
protected:
	//! Virtual function called on time
	virtual void handle_timer() = 0;
private:
	static void vtimer_func(void *ptr)
	{
		static_cast<virtual_timer*>(ptr)->handle_timer();
	}
private:
	//Noncopyable
	virtual_timer(const task_base&);
	virtual_timer& operator=(const task_base&);
private:
	vtimer_t *timer;
};
/*-----------------------------------------------------------------------*/
}
/*-----------------------------------------------------------------------*/
#endif /*  __cplusplus */
/*-----------------------------------------------------------------------*/

#endif /*ISIX_CONFIG_USE_TIMERS */
#endif /* IRQ_TIMERS_H_ */
/*-----------------------------------------------------------------------*/
