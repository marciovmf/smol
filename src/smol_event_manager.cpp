#include <smol/smol_event_manager.h>
#include <smol/smol_event.h>

#ifndef SMOL_EVENT_QUEUE_INITIAL_CAPACITY
#define SMOL_EVENT_QUEUE_INITIAL_CAPACITY 16
#endif

namespace smol
{
  EventHandler::EventHandler() { }

  EventHandler::EventHandler(EventHandlerCallback handlerCallback, uint32 mask, void* context):
    callback(handlerCallback), context(context), mask(mask) { }

  EventManager& EventManager::get()
  {
    static EventManager instance;
    return instance;
  }

  EventManager::EventManager(): events(SMOL_EVENT_QUEUE_INITIAL_CAPACITY * sizeof(Event)) { }

  EventHandlerId EventManager::addHandler(EventHandlerCallback handlerFunc, uint32 eventMask, void* context)
  {
    return handlers.add(EventHandler(handlerFunc, eventMask, context));
  }

  void EventManager::removeHandler(EventHandlerId handler)
  {
    handlers.remove(handler);
  }

  void EventManager::pushEvent(const Event& event)
  {
    Event* e = (Event*) events.pushSize(sizeof(Event));
    *e = event;
  }

  void EventManager::dispatchEvents()
  {
    const uint32 numHandlers = handlers.count();
    const EventHandler* handlerList = handlers.getArray();
    const Event* eventList = (const Event*) events.getData();
    uint32 numEvents = (uint32) (events.getUsed() / sizeof(Event));

    for(uint32 i = 0; i < numEvents; i++)
    {
      const Event& event = *eventList++;
      for (uint32 handlerIndex = 0; handlerIndex < numHandlers; handlerIndex++)
      {
        const EventHandler& handler = handlerList[handlerIndex];
        if (event.type & handler.mask)
        {
          bool eventHandled = handler.callback(event, handler.context);
          if (eventHandled)
            break;
        }
      }
    }
    events.reset();
  }
}
