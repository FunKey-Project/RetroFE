#include "StateMachine.h"

StateMachine::StateMachine(lua_State *l, LuaEvent *e, UserInput *i)
: luaState_(l)
, luaEvent_(e)
, input_(i)
, state_(ON_STARTUP_ENTER)
, nextState_(ON_IDLE_ENTER)
{
}

void StateMachine::update(float dt, SDL_Event *e)
{
    input_->update(*e);

    switch(state_) {
        case ON_STARTUP_ENTER:
            luaEvent_->trigger(luaState_, "startuponEnter");
            state_ = ON_STARTUP_UPDATE;
            break;

        case ON_STARTUP_UPDATE:
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

        case ON_IDLE_ENTER:
            luaEvent_->trigger(luaState_, "idleEnter");
            state_ = ON_IDLE_UPDATE;
            break;

        case ON_IDLE_UPDATE:
            luaEvent_->trigger(luaState_, "idleUpdate");

            if(!luaEvent_->isBusy(luaState_, "idleisBusy")) {
                if(input_->keystate(UserInput::KeyCodeUp)) {
                    state_ = ON_IDLE_EXIT;
                    nextState_ = ON_SCROLL_ENTER;
                }
            }
            break;
        case ON_IDLE_EXIT:
            luaEvent_->trigger(luaState_, "idleExit");
            state_ = ON_IDLE_EXIT_WAIT;
            break;
        
        case ON_IDLE_EXIT_WAIT:
            if(!luaEvent_->isBusy(luaState_, "idleisBusy")) {
                state_ = nextState_;
            }
            break;
        case ON_SCROLL_ENTER:
            luaEvent_->trigger(luaState_, "scrollonEnter");
            state_ = ON_SCROLL_UPDATE;
            break;

        case ON_SCROLL_UPDATE:
            luaEvent_->trigger(luaState_, "scrollUpdate");

            if(!luaEvent_->isBusy(luaState_, "scrollisBusy")) {
                state_ = ON_SCROLL_EXIT;
            }
            break;

        case ON_SCROLL_EXIT:
            luaEvent_->trigger(luaState_, "scrollonExit");
            state_ = ON_SCROLL_EXIT_WAIT;
            break;
        
        case ON_SCROLL_EXIT_WAIT:
            if(!luaEvent_->isBusy(luaState_, "scrollisBusy")) {
                state_ = ON_IDLE_ENTER;
            }
            break;

    }
}
