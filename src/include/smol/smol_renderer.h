#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <smol/smol_engine.h>
#include <smol/smol_scene.h>

namespace smol
{
  class SMOL_ENGINE_API Renderer
  {
    int width;
    int height;
    Scene* scene;

    public:
    Renderer (Scene& scene, int width, int height);
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    void resize(int width, int height);   // Resizes the necessary resources to accomodathe the required dimentions.
    void render();                        // Called once per frame to render the scene.
    ~Renderer();
    Vector2 getViewportSize();
  };

}

#endif  // SMOL_RENDERER_H

