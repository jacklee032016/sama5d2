
#include "extFsm.h"

void	extFsmHandle(ext_fsm_t *fsm)
{
	unsigned char	i;
	unsigned char newState = EXT_STATE_CONTINUE;
	const statemachine_t *_states = fsm->states;

//	EXT_ASSERT(("stats is null in state machine"), _states != NULL);

	if(fsm->currentEvent == EXT_EVENT_NONE || _states == NULL)
	{
		EXT_ERRORF(("Invalidate params in state Machine"));
		return;
	}

//	EXT_DEBUGF(EXT_DBG_ON, ("states %p ", _states)) ;
//	EXT_DEBUGF(EXT_DBG_ON, ("state %d in FSM, current state is %d", _states->state, fsm->currentState)) ;
	while(_states->state != EXT_STATE_CONTINUE && _states->eventHandlers )
	{
//		EXT_DEBUGF(EXT_DBG_ON, ("state %d in FSM, current state is %d", _states->state, fsm->currentState)) ;
		if( _states->state == fsm->currentState )
		{
			const transition_t *handle = _states->eventHandlers;

			for(i=0; i < _states->size; i++)
			{
				if(fsm->currentEvent == handle->event )
				{
					newState = (handle->handle)(fsm);
					fsm->currentEvent = EXT_EVENT_NONE;
					
					if(newState!= EXT_STATE_CONTINUE && newState != fsm->currentState )
					{
						fsm->currentState = newState;
					}
					
					return;
				}
				
				handle++;
			}

		}

		_states++;
	}
	

	EXT_INFOF(("State Machine no handle for event %d in state %d", fsm->currentEvent, fsm->currentState));
	
	return;
}

