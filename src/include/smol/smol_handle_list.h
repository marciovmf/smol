#ifndef SMOL_HANDLE_LIST
#define SMOL_HANDLE_LIST

#include <smol/smol_log.h>
#include <smol/smol_arena.h>
#include <string.h>

#define getSlotIndex(slotInfo) ((int)((char*) (slotInfo) - slots.getData()) / sizeof(SlotInfo))

#define INVALID_HANDLE(T) (Handle<T>{ (int) 0xFFFFFFFF, (int) 0xFFFFFFFF})

namespace smol
{
  //
  // SlotInfo
  //
  struct SlotInfo
  {
    union
    {
      int resourceIndex;
      int nextFreeSlotIndex;
    };

    int version;
  };

  //
  // Handle
  //
  template <typename T>
    struct Handle
    {
      int slotIndex;
      int version;

      int compare(const Handle<T>& other);
      int operator== (const Handle<T>& other);
      int operator!= (const Handle<T>& other);
    };

  template <typename T>
    inline int Handle<T>::compare(const Handle<T>& other)
    {
      return other.slotIndex == slotIndex && other.version == version;
    }

  template <typename T>
    inline int Handle<T>::operator==(const Handle<T>& other)
    {
      return compare(other);
    }

  template <typename T>
    inline int Handle<T>::operator!=(const Handle<T>& other)
    {
      return !compare(other);
    }

  //
  // Handle
  //
  template <typename T>
    class HandleList
    {
      Arena slots;
      Arena resources;
      int resourceCount;
      int freeSlotListCount;
      int freeSlotListStart;

      public:
      HandleList(int initialCapacity);
      Handle<T> reserve();
      Handle<T> add(const T&);
      T* lookup(Handle<T> handle) const;
      void remove(Handle<T> handle);
      void reset();
      int count() const;
      const T* getArray();
    };

  //
  // HandleList
  //
  template<typename T> 
    HandleList<T>::HandleList(int initialCapacity):
      slots(sizeof(SlotInfo) * initialCapacity),
      resources(sizeof(T) * initialCapacity),
      resourceCount(0),
      freeSlotListCount(0),
      freeSlotListStart(-1)
  { }

  template<typename T>
    inline int HandleList<T>::count() const
    {
      return resourceCount;
    }

  template<typename T>
    const T* HandleList<T>::getArray()
    {
      return (const T*) resources.getData();
    }

  template<typename T>
    Handle<T> HandleList<T>::reserve()
    {
      SlotInfo* slotInfo;
      T* resource;
      ++resourceCount;

      if(freeSlotListCount)
      {
        // Reuse a Free slot.
        --freeSlotListCount;
        slotInfo = ((SlotInfo*) slots.getData()) + freeSlotListStart; 
        freeSlotListStart = slotInfo->nextFreeSlotIndex;
      }
      else
      {
        // Add a new slot and a new resource
        slotInfo = (SlotInfo*) slots.pushSize(sizeof(SlotInfo));
        slotInfo->version = 0;
        resources.pushSize(sizeof(T));
      }

      slotInfo->resourceIndex = resourceCount - 1;   // The newly added resource or the first empty space from a deleted resource
      resource = ((T*) resources.getData())+ slotInfo->resourceIndex; // gets the resource this slot points to

      // Create a handle to the resource
      Handle<T> handle;
      handle.slotIndex = getSlotIndex(slotInfo);
      handle.version = slotInfo->version;
      return handle;
    }

  template<typename T>
    Handle<T> HandleList<T>::add(const T& t)
    {
      Handle<T> handle = reserve();
      T* resource = lookup(handle);
      memcpy((void*) resource, (void*) &t, sizeof(T));
      return handle;
    }

  template <typename T>
    T* HandleList<T>::lookup(Handle<T> handle) const
    {
      if (handle.slotIndex >= slots.getCapacity() || handle.slotIndex < 0)
      {
        //Log::warning("Attempting to lookup a Handle slot out of bounds");
        return nullptr;
      }

      SlotInfo* slotInfo = ((SlotInfo*) slots.getData()) + handle.slotIndex;
      T* resource = nullptr;
      if (handle.version == slotInfo->version)
      {
        resource = ((T*) resources.getData()) + slotInfo->resourceIndex;
      }
      return resource;
    }

  template <typename T>
    void HandleList<T>::remove(Handle<T> handle)
    {
      // We never leave holes on the resource list!
      // When deleting any resource (other than the last one) we actually
      // move the last resource to the place of the one being deleted
      // and fix the it's slot so it points to the correct resource index.

      if (handle.slotIndex >= slots.getCapacity() / sizeof(T) || handle.slotIndex < 0)
      {
        Log::warning("Attempting to remove a Handle slot out of bounds");
        return;
      }

      SlotInfo* slotOfLast = ((SlotInfo*) slots.getData()) + (resourceCount-1);
      SlotInfo* slotOfRemoved = ((SlotInfo*)  slots.getData()) + handle.slotIndex;
      ++slotOfRemoved->version;

      if (slotOfRemoved != slotOfLast)
      {
        // Move the last resource to the space left by the one being removed
        T* resourceLast = ((T*) resources.getData()) + slotOfLast->resourceIndex;
        T* resourceRemoved = ((T*) resources.getData()) + slotOfRemoved->resourceIndex;
        memcpy((void*) resourceRemoved, (void*) resourceLast, sizeof(T));
      }

      slotOfRemoved->nextFreeSlotIndex = freeSlotListStart;
      freeSlotListStart = handle.slotIndex;
      ++freeSlotListCount;
      --resourceCount;
    }

  template <typename T>
    void HandleList<T>::reset()
    {
      // invalidate ALL slots
      for (int i = 0; i < resourceCount; i++)
      {
        SlotInfo* slot = ((SlotInfo*)  slots.getData()) + i;
        slot->version++;
      }

      slots.reset();
      resources.reset();
      resourceCount = 0;
      freeSlotListCount = 0;
      freeSlotListStart = -1;
    }
}

#endif  // SMOL_HANDLE_LIST
