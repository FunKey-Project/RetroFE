#include "StateMachine.h"

StateMachine::StateMachine(lua_State *l, LuaEvent *e)
: luaState_(l)
, luaEvent_(e)
, state_(ON_STARTUP_ENTER)
{
}

void StateMachine::update(float dt)
{
    switch(state_) {
        case ON_STARTUP_ENTER:
            luaEvent_->trigger(luaState_, "startuponEnter");
            state_ = ON_STARTUP_UPDATE;
            break;

        case ON_STARTUP_UPDATE:
//todo: only do it if the main system is not busy
            luaEvent_->trigger(luaState_, "startuponUpdate");

            if(!luaEvent_->isBusy(luaState_, "startupisBusy")) {
                state_ = ON_STARTUP_EXIT;
            }
            break;

        case ON_STARTUP_EXIT:
            luaEvent_->trigger(luaState_, "startuponExit");
            state_ = ON_STARTUP_EXIT_WAIT;
            break;
        
        case ON_STARTUP_EXIT_WAIT:
            if(!luaEvent_->isBusy(luaState_, "startupisBusy")) {
                state_ = ON_IDLE_ENTER;
            }
            break;
    }
}
