#ifndef SMOL_RENDERER_TYPES_H
#define SMOL_RENDERER_TYPES_H

#include <smol/smol_handle_list.h>
#include <smol/smol_color.h>
#include <smol/smol_rect.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>
#include <smol/smol_mat4.h>
#include <smol/smol_camera.h>
#include <smol/smol_material.h>
#include <smol/smol_shader.h>
#include <smol/smol_texture.h>
#include <smol/smol_triangle_mesh.h>
#include <smol/smol_mesh.h>
#include <smol/smol_renderable.h>
#include <smol/smol_font.h>

//smol_color.h is not used by this header, but included by convenience and consistency since Color is a type meant to be used by the renderer

namespace smol
{
  enum RenderQueue : char
  {
    QUEUE_OPAQUE = 10,
    QUEUE_TRANSPARENT = 20,
    QUEUE_GUI = 30,
    QUEUE_TERRAIN = 40
  };

  enum Primitive : char
  {
    TRIANGLE,
    TRIANGLE_STRIP,
    LINE,
    POINT
  };

#pragma pack(push, 1)
  struct VertexPCU
  {
    Vector3 position;
    Color color;
    Vector2 uv;
  };
#pragma pack(pop)

}

#endif  // SMOL_RENDERER_TYPES_H
