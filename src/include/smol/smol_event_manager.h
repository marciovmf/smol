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
    EventManager();

    public:
    static EventManager& get();
    EventHandlerId subscribe(EventHandlerCallback handlerFunc, uint32 eventMask, void* context = nullptr);
    void unsubscribe(EventHandlerId handler);
    void raise(const Event& event);
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
  };
}

#endif //SMOL_EVENT_MANAGER_H

