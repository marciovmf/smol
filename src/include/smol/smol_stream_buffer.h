#ifndef SMOL_STREAM_BUFFER_H
#define SMOL_STREAM_BUFFER_H

#include <smol/smol_renderer_types.h>

namespace smol
{
  struct StreamBuffer
  {
    enum Format
    {
      UNINITIALIZED   = 0,
      POS_COLOR_UV    = 1,
      POS_COLOR_UV_UV = 2,
    };

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    uint32 indicesPerElement;     // how many indices per element;
    uint32 capacity;              // maximun number of elements (not bytes!)
    uint32 used;                  // number of elements stored in the buffer
    uint32 flushCount;            // used to enlarge the buffer later if we flushed early
    size_t elementSize;           // size of a sigle element
    Format format;
    bool bound;
  };
}
#endif //SMOL_STREAM_BUFFER_H

