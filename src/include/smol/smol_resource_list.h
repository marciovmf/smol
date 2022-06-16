#ifndef SMOL_PACKED_LIST
#define SMOL_PACKED_LIST

#include <smol/smol_log.h>
#include <smol/smol_arena.h>
#include <string.h>

#define getSlotIndex(slotInfo) ((int)((char*) (slotInfo) - slots.getData()) / sizeof(SlotInfo))

#define INVALID_HANDLE(T) (Handle<T>{ (int) 0xFFFFFFFF, (int) 0xFFFFFFFF})

namespace smol
{
  struct SlotInfo
  {
    union
    {
      int resourceIndex;
      int nextFreeSlotIndex;
    };

    int version;
  };

  template <typename T>
    struct Handle
    {
      int slotIndex;
      int version;

      inline int compare(const Handle<T>& other);
      inline int operator== (const Handle<T>& other);
      inline int operator!= (const Handle<T>& other);
    };

  template <typename T>
    int Handle<T>::compare(const Handle<T>& other)
    {
      return other.slotIndex == slotIndex && other.version == version;
    }

  template <typename T>
    int Handle<T>::operator==(const Handle<T>& other)
    {
      return compare(other);
    }

  template <typename T>
    int Handle<T>::operator!=(const Handle<T>& other)
    {
      return !compare(other);
    }

  template <typename T>
    class ResourceList
    {
      Arena slots;
      Arena resources;
      int resourceCount;
      int freeSlotListCount;
      int freeSlotListStart;

      public:
      ResourceList(int initialCapacity);
      Handle<T> reserve();
      Handle<T> add(const T&);
      T* lookup(Handle<T> handle);
      void remove(Handle<T> handle);
      void reset();
      int count();
      const T* getArray();
    };

  template<typename T> 
    ResourceList<T>::ResourceList(int initialCapacity):
      slots(sizeof(SlotInfo) * initialCapacity),
      resources(sizeof(T) * initialCapacity),
      resourceCount(0),
      freeSlotListCount(0),
      freeSlotListStart(-1)
  { }

  template<typename T>
    inline int ResourceList<T>::count()
    {
      return resourceCount;
    }

  template<typename T>
    const T* ResourceList<T>::getArray()
    {
      return (const T*) resources.getData();
    }

  template<typename T>
    Handle<T> ResourceList<T>::reserve()
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
    Handle<T> ResourceList<T>::add(const T& t)
    {
      Handle<T> handle = reserve();
      T* resource = lookup(handle);
      memcpy((void*) resource, (void*) &t, sizeof(T));
      return handle;
    }

  template <typename T>
    T* ResourceList<T>::lookup(Handle<T> handle)
    {
      //SMOL_ASSERT(handle.slotIndex < slots.capacity, "Handle slot is out of bounds");
      //SMOL_ASSERT(handle.slotIndex >= 0, "Handle slot is out of bounds");
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
    void ResourceList<T>::remove(Handle<T> handle)
    {
      // We never leave holes on the resource list!
      // When deleting any resource (other than the last one) we actually
      // move the last resource to the place of the one being deleted
      // and fix the it's slot so it points to the correct resource index.

      //SMOL_ASSERT(handle.slotIndex < slots.capacity / sizeof(T), "Handle slot is out of bounds");
      //SMOL_ASSERT(handle.slotIndex >= 0, "Handle slot is out of bounds");
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
        memcpy((void*) resourceRemoved,
            (void*) resourceLast,
            sizeof(T));
      }

      slotOfRemoved->nextFreeSlotIndex = freeSlotListStart;
      freeSlotListStart = handle.slotIndex;
      ++freeSlotListCount;
      --resourceCount;
    }

  template <typename T>
    void ResourceList<T>::reset()
    {
      slots.reset();
      resources.reset();
      resourceCount = 0;
      freeSlotListCount = 0;
      freeSlotListStart = -1;
    }
}

#endif  // SMOL_PACKED_LIST
