#include <smol/smol_event_manager.h>
#include <smol/smol_event.h>

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

  EventManager::EventManager() { }

  EventHandlerId EventManager::subscribe(EventHandlerCallback handlerFunc, uint32 eventMask, void* context)
  {
    return handlers.add(EventHandler(handlerFunc, eventMask, context));
  }

  void EventManager::unsubscribe(EventHandlerId handler)
  {
    handlers.remove(handler);
  }

  void EventManager::raise(const Event& event)
  {
    const uint32 numHandlers = handlers.count();
    const EventHandler* allHandlers = handlers.getArray();

    for (uint32 i = 0; i < numHandlers; i++)
    {
      const EventHandler& handler = allHandlers[i];
      if (event.type & handler.mask)
      {
        bool eventHandled = handler.callback(event, handler.context);
        if (eventHandled)
          break;
      }
    }
  }
}
