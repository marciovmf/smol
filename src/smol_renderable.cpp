
#include <smol/smol_renderable.h>

namespace smol
{
  Renderable::Renderable(Handle<Material> material, Handle<Mesh> mesh):
    material(material), mesh(mesh) { }
}
