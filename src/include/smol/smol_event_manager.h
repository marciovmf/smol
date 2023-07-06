#ifndef SMOL_EVENT_MANAGER_H
#define SMOL_EVENT_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_event.h>
#include <smol/smol_handle_list.h>

namespace smol
{
  typedef bool (*EventHandlerCallback)(const Event&, void* context);

  struct SMOL_ENGINE_API EventHandler final
  {
    EventHandlerCallback callback;
    EventHandler(EventHandlerCallback handlerCallback, uint32 mask, void* context);
    void* context;
    int32 mask;
    EventHandler();
  };

  typedef Handle<EventHandler> EventHandlerId;
  
  template class SMOL_ENGINE_API HandleList<EventHandler>;

  class SMOL_ENGINE_API EventManager final
  {
    HandleList<EventHandler> handlers;
    Arena events;
    EventManager();

    public:
    static EventManager& get();
    EventHandlerId addHandler(EventHandlerCallback handlerFunc, uint32 eventMask, void* context = nullptr);
    void removeHandler(EventHandlerId handler);
    void pushEvent(const Event& event);
    void dispatchEvents();

    // Disallow copies
    EventManager(const EventManager& other) = delete;
    EventManager(const EventManager&& other) = delete;
    void operator=(const EventManager& other) = delete;
    void operator=(const EventManager&& other) = delete;
  };
}

#endif //SMOL_EVENT_MANAGER_H

