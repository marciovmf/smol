#include <smol/smol_gui.h>
#include <smol/smol_material.h>
#include <smol/smol_systems_root.h>

namespace smol
{
  Vector2 GUI::getScreenSize() const { return Vector2(screenW, screenH); }

  Rect GUI::getLastRect() const { return lastRect; }

  void GUI::begin(int screenWidth, int screenHeight)
  {
    screenW = (float) screenWidth;
    screenH = (float) screenHeight;

    if (glyphDrawDataArena.getCapacity() == 0)
    {
      glyphDrawDataArena.initialize(256 * sizeof(GlyphDrawData));
      Renderer::createStreamBuffer(&streamBuffer);
    }
    glyphDrawDataArena.reset();
    Renderer::begin(streamBuffer);
  }

  void GUI::panel(GUICOntrolID id, int32 x, int32 y, int32 w, int32 h)
  {
    lastRect = Rect(x, y, w, h);
    GUISkin::ID styleId = GUISkin::FRAME_ACTIVE;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, 0.0f), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

  }

  void GUI::label(GUICOntrolID id, const char* text, int32 x, int32 y, Align align)
  {
    const uint16 fontSize = skin.labelFontSize;
    const float scaleX  = fontSize / screenW;
    const float scaleY  = fontSize / screenH;
    float posX    = x / screenW;
    float posY    = y / screenH;
    const size_t textLen = strlen(text);

    GlyphDrawData* drawData = (GlyphDrawData*) glyphDrawDataArena.pushSize((1 + textLen) * sizeof(GlyphDrawData));
    Vector2 bounds = skin.font->computeString(text, skin.color[GUISkin::TEXT], drawData, 1.0f);
    bounds.mult(scaleX, scaleY);
    lastRect = Rect((int32)posX, (int32) posY, (int32) bounds.x, (int32) bounds.y);


    if (align == Align::CENTER)
    {
      posX -= bounds.x/2;
      posY -= bounds.y/2;
    }
    else if (align == Align::RIGHT)
    {
      posX -= bounds.x;
      posY -= bounds.y;
    }

    // Draws a solid background behind the text. Keep this here for debugging
    Renderer::pushSprite(streamBuffer, Vector3(posX, posY, 0.0f), Vector2(bounds.x, bounds.y), Rectf(), Color::BLACK);

    for (int i = 0; i < textLen; i++)
    {
      GlyphDrawData& data = drawData[i];
      Vector3 offset = Vector3(posX + data.position.x * scaleX, posY + data.position.y *  scaleY, 0.0f);
      Vector2 size = data.size;
      size.mult(scaleX, scaleY);
      Renderer::pushSprite(streamBuffer, offset, size, data.uv, data.color);
    }
  }

  bool GUI::doButton(GUICOntrolID id, const char* text, int32 x, int32 y, int32 w, int32 h)
  {
    lastRect = Rect(x, y, w, h);

    GUISkin::ID styleId;
    bool mouseOver = lastRect.containsPoint(root->mouse.getCursorPosition());
    bool returnValue = false;

    bool isActiveControl = activeControlId == id;
    bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
    bool upThisFrame = root->mouse.getButtonUp(MOUSE_BUTTON_LEFT);
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);

    if (mouseOver)
    {
      hoverControlId = id;
      styleId = GUISkin::BUTTON_HOVER;

      if (downThisFrame || (isDown && isActiveControl))
      {
        activeControlId = id;
        styleId = GUISkin::BUTTON_ACTIVE;
      }
      else if(upThisFrame && isActiveControl)
      {
        activeControlId = 0;
        returnValue = true;
      }
    }
    else
    {
      hoverControlId = 0;
      styleId = GUISkin::BUTTON;
      if (upThisFrame && isActiveControl)
      {
        activeControlId = 0;
      }
    }

    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, 0.0f), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    const int centerX = x + w/2;
    const int centerY = y + h/2;
    label(id, text, centerX, centerY, CENTER);
    return returnValue;
  }

  bool GUI::doToggleButton(GUICOntrolID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h)
  {
    lastRect = Rect(x, y, w, h);

    GUISkin::ID styleId;
    bool mouseOver = lastRect.containsPoint(root->mouse.getCursorPosition());
    bool returnValue = toggled;

    bool isActiveControl = activeControlId == id;
    bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
    bool upThisFrame = root->mouse.getButtonUp(MOUSE_BUTTON_LEFT);
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);

    if (toggled)
      styleId = GUISkin::TOGGLE_BUTTON_ACTIVE;
    else
      styleId = GUISkin::TOGGLE_BUTTON;

    if (mouseOver)
    {
      hoverControlId = id;

      styleId = toggled ? GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE : GUISkin::TOGGLE_BUTTON_HOVER;

      if (downThisFrame || (isDown && isActiveControl))
      {
        activeControlId = id;
        styleId = GUISkin::TOGGLE_BUTTON_ACTIVE;
      }
      else if(upThisFrame && isActiveControl)
      {
        activeControlId = 0;
        returnValue = !toggled;
      }
    }
    else
    {
      hoverControlId = 0;
      if (upThisFrame && isActiveControl)
      {
        activeControlId = 0;
      }
    }

    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, 0.0f), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    const int centerX = x + w/2;
    const int centerY = y + h/2;
    label(id, text, centerX, centerY, CENTER);
    return returnValue;
  }

  void GUI::end()
  {
    Renderer::end(streamBuffer);
  }


#ifndef SMOL_MODULE_GAME
  Handle<Material> GUI::getMaterial() const
  {
    return material;
  }

  GUI::GUI(Handle<Material> material, Handle<Font> font)
  {
    this->material = material;
    skin.font = font;
    skin.labelFontSize = 16;
    root = SystemsRoot::get();

    Color colorForText = Color(236.f / 255.f, 240.f / 255.f, 241.f / 255.f);
    Color colorForHead = Color::BLUE;
    Color colorForArea = Color::BLACK;
    //Color color_for_body = Color(44.f / 255.f, 62.f / 255.f, 80.f / 255.f);
    //Color color_for_pops = Color(33.f / 255.f, 46.f / 255.f, 60.f / 255.f);

    skin.color[GUISkin::TEXT]          = Color(colorForText.r, colorForText.g, colorForText.b, 1.00f );
    skin.color[GUISkin::TEXT_DISABLED] = Color(colorForText.r, colorForText.g, colorForText.b, 0.58f );

    skin.color[GUISkin::BUTTON]        = Color(colorForHead.r, colorForHead.g, colorForHead.b, 0.50f );
    skin.color[GUISkin::BUTTON_HOVER]  = Color(colorForHead.r, colorForHead.g, colorForHead.b, 0.86f );
    skin.color[GUISkin::BUTTON_ACTIVE] = Color::MAROON;

    skin.color[GUISkin::TOGGLE_BUTTON]                = Color(colorForHead.r, colorForHead.g, colorForHead.b, 0.50f );
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER]          = Color(colorForHead.r, colorForHead.g, colorForHead.b, 0.86f );
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE]   = Color(150, 0, 0, 255);
    skin.color[GUISkin::TOGGLE_BUTTON_ACTIVE]         = Color::MAROON;

    skin.color[GUISkin::FRAME]         = Color(colorForArea.r, colorForArea.g, colorForArea.b, 1.00f );
    skin.color[GUISkin::FRAME_HOVER]   = Color(colorForHead.r, colorForHead.g, colorForHead.b, 0.78f );
    skin.color[GUISkin::FRAME_ACTIVE]  = Color(38, 38, 38, 255);
  }

#endif

}
