#ifndef SMOL_PACKED_LIST
#define SMOL_PACKED_LIST

#include <smol/smol_arena.h>
#include <string.h>

// Returns the address of the Resource<T> at index n
#define getResourcePtrAt(n) (((Resource<T>*) resourceList.data) + (n))

// Returns the address of the slot at index n
#define getSlotPtrAt(n) (((ResourceSlot*) slotList.data) + (n))

//TODO(marcio): Get this from the settings system
#define SMOL_RESOURCE_LIST_ARENA_SIZE MEGABYTE(5)

namespace smol
{
  // A handle is a loose reference to a resource
  template <typename T> struct Handle
  {
    unsigned int slotIndex;
    unsigned int version;
  };

  struct ResourceSlot
  {
    unsigned int resourceIndex;
    unsigned int version;
  };

  // A resource is some data kept packed in memory and identified by an id
  template <typename T>
    struct Resource
    {
      unsigned int id;
      T resource; 
    };

  template <typename T>
    struct ResourceList
    {
      unsigned int freeListHead;
      unsigned int freeListCount;
      unsigned int resourceCount;
      Arena slotList;
      Arena resourceList;

      ResourceList();

      // Adds a resource to the resource list and returns a handle
      Handle<T> add(T& t);

      // Removes a resource identified by a handle
      void remove(Handle<T> handle);

      // Gets a pointer to the resource identified by a handle
      T* lookup(Handle<T> handle);

      // returns true if the given handle is valid. Valid handles are handles pointing to live resources.
      bool valid(Handle<T> handle);
    };

  template <typename T>
    ResourceList<T>::ResourceList() : 
      freeListHead(0), freeListCount(0), resourceCount(0),
      slotList(SMOL_RESOURCE_LIST_ARENA_SIZE),
      resourceList(SMOL_RESOURCE_LIST_ARENA_SIZE) { }

  template <typename T>
    Handle<T> ResourceList<T>::add(T& t)
    {
      ResourceSlot* slot = nullptr;
      Resource<T>* resource = nullptr;

      if (freeListCount)
      {
        freeListCount--;
        slot = getSlotPtrAt(freeListHead);
        resource = getResourcePtrAt(resourceCount);
      }
      else
      {
        slot = (ResourceSlot*) slotList.pushSize(sizeof(ResourceSlot));
        slot->version = 0;
        resource = (Resource<T>*) resourceList.pushSize(sizeof(Resource<T>));
      }

      ++resourceCount;
      unsigned int resourceIndex = (unsigned int) (resource - (Resource<T>*) resourceList.data);
      unsigned int slotIndex = (unsigned int) (slot - (ResourceSlot*) slotList.data);
      slot->resourceIndex = resourceIndex;

      // Copy the resource data to it's correct location and set it's id
      memcpy((void*) &resource->resource, (const void*)&t, sizeof(T));
      resource->id = resourceIndex;

      Handle<T> handle;
      handle.slotIndex = slotIndex;
      handle.version = slot->version;
      return handle;
    }

  template <typename T>
    void ResourceList<T>::remove(Handle<T> handle)
    {
      --resourceCount;
      ResourceSlot* slot = getSlotPtrAt(handle.slotIndex);
      Resource<T>* last = getResourcePtrAt(resourceCount);
      Resource<T>* deleted = getResourcePtrAt(slot->resourceIndex);

      // Update the deleted slot and add it to the free list
      slot->version++;
      slot->resourceIndex = freeListHead;
      freeListHead = handle.slotIndex;

      // copy the last resource over the one being deleted and fix slot that pointed to what was the last resource
      if (last != deleted)
      {
        memcpy(deleted, last, sizeof(Resource<T>));
        slot = getSlotPtrAt(last->id);
        slot->resourceIndex = (unsigned int)(deleted - (Resource<T>*)resourceList.data);
      }

      ++freeListCount;
    }

  template <typename T>
    T* ResourceList<T>::lookup(Handle<T> handle)
    {
      ResourceSlot* slot = getSlotPtrAt(handle.slotIndex);
      if (handle.version != slot->version) 
        return nullptr;

      Resource<T>* resource = getResourcePtrAt(slot->resourceIndex);
      return &resource->resource;
    }

  template <typename T>
    inline bool ResourceList<T>::valid(Handle<T> handle)
    {
      ResourceSlot* slot = getSlotPtrAt(handle.slotIndex);
      return handle.version != slot->version;
    }
}

#undef getResourcePtrAt
#undef getSlotPtrAt

#endif  // SMOL_PACKED_LIST
