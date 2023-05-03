#ifndef SMOL_RENDERABLE_H
#define SMOL_RENDERABLE_H

#include <smol/smol_engine.h>
#include <smol/smol_material.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_mesh.h>

namespace smol
{
  struct SMOL_ENGINE_API Renderable final
  {
    Handle<Material> material;
    Handle<Mesh> mesh;
    Renderable::Renderable(Handle<Material> material, Handle<Mesh> mesh);
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::Renderable>;
  template class SMOL_ENGINE_API smol::Handle<smol::Renderable>;
}

#endif  //SMOL_RENDERABLE_H
