local Event = {}
Event.__index = Event

function Event.new()
  local instance = {  
    events = {}
  }

  setmetatable(instance, EventManager)

  return instance
end

function Event:register(event, cbself, callback)
    local listeners = self.events[event]
    if listeners ~= nil then
        listeners[#listeners + 1] = callback
    else
        self.events[event] = {{cb=callback, s=cbself}}
    end
end

function Event:trigger(event, ...)
    local listeners = self.events[event]
    if listeners ~= nil then
        for i = 1, #listeners do
            listeners[i].cb(listeners[i].s, ...)
        end
    end
end

return Event
