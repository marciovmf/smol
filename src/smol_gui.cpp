#include <algorithm>
#include <ios>
#include <smol/smol_gui.h>
#include <smol/smol_material.h>
#include <smol/smol_systems_root.h>
#include "smol_gui_icons.h"
#include <system_error>

namespace smol
{
  Vector2 GUI::getScreenSize() const { return Vector2(screenW, screenH); }

  GUISkin& GUI::getSkin() { return skin; }

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
    GUISkin::ID styleId = GUISkin::PANEL;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, 0.0f), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);
  }

  void GUI::horizontalSeparator(int32 x, int32 y, int32 width)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    Vector2 point[2];
    point[0] = {x / screenW,  y / screenH};
    point[1] = {(x + width) / screenW, y / screenH};
    Renderer::pushLines(streamBuffer, point, 2, skin.color[GUISkin::SEPARATOR], 2 / screenH);
  }

  void GUI::verticalSeparator(int32 x, int32 y, int32 height)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    Vector2 point[2];

    point[0] = {x / screenW,  y / screenH};
    point[1] = {x / screenW, (y + height) / screenH};

    Renderer::pushLines(streamBuffer, point, 2, skin.color[GUISkin::SEPARATOR], 2 / screenW);
  }

  Point2 GUI::beginWindow(GUICOntrolID id, const char* title, int32 x, int32 y, int32 w, int32 h)
  {
    lastRect = Rect(x, y, w, h);
    windowCount++;

    bool isBeingDragged = draggedControlId == id;
    const int titleBarHeight = 30;
    Rect titleBarRect = Rect(x, y, w, titleBarHeight);
    bool mouseOverTitleBar = titleBarRect.containsPoint(root->mouse.getCursorPosition());

    // Draw title bar
    GUISkin::ID styleId = (mouseOverTitleBar || isBeingDragged) ? GUISkin::WINDOW_TITLE_BAR_HOVER : GUISkin::WINDOW_TITLE_BAR; 
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, 0.0f), 
        Vector2(w / screenW, titleBarHeight / screenH),
        Rectf(), skin.color[styleId]);
    label(id, title, x + DEFAULT_H_SPACING, y + (titleBarHeight/2), LEFT);

    // draw the window panel
    Color windowColor = skin.color[GUISkin::WINDOW];
    windowColor.a = skin.windowOpacity;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, (y + titleBarHeight) / screenH, 0.0f), 
        Vector2(w / screenW, (h  - titleBarHeight) / screenH),
        Rectf(), windowColor);
        
    beginArea(x, y + titleBarHeight, w, h - titleBarHeight);
    lastRect = Rect(x, y, w, h);

    // check for dragging the title bar
    Point2 newPos = Point2{x, y};
    const Point2& cursorPos = root->mouse.getCursorPosition();
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);

    if(isBeingDragged)
    {
      if (isDown)
      {
        newPos.x = cursorDragOffset.x + cursorPos.x;
        newPos.y = cursorDragOffset.y + cursorPos.y;
      }
      else
      {
        draggedControlId = 0;
      }
    }
    else if (mouseOverTitleBar)
    {
      bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
      if (downThisFrame)
      {
        draggedControlId = id;
        cursorDragOffset = Point2{x - cursorPos.x, y - cursorPos.y};
        newPos.x = cursorDragOffset.x + cursorPos.x;
        newPos.y = cursorDragOffset.y + cursorPos.y;
      }
    }

    return newPos;
  }

  void GUI::endWindow()
  {
    if (windowCount == 0)
    {
      debugLogError("Unbalanced begin/end window calls.");
      return;
    }
    windowCount--;
    endArea();
  }

  void GUI::beginArea(int x, int y, int w, int h)
  {
    if ((areaCount + 1) >= MAX_NESTED_AREAS)
    {
      debugLogError("Too many nested areas. Maximum allowd is %d", MAX_NESTED_AREAS);
      return;
    }

    area[areaCount] = Rect(x, y, w, h);
    areaOffset = Rect(areaOffset.x + x, areaOffset.y + y, areaOffset.w + w, areaOffset.h + h);
    areaCount++;
  }

  void GUI::endArea()
  {
    if (areaCount == 0)
    {
      debugLogError("Unbalanced begin/end area calls.");
      return;
    }

    Rect& r = area[areaCount];
    areaCount--;
    areaOffset = Rect(areaOffset.x - r.x, areaOffset.y - r.y, areaOffset.w - r.w, areaOffset.h - r.h);
    if (areaCount == 0)
      areaOffset = Rect(0, 0, 0 ,0);
  }

  void GUI::label(GUICOntrolID id, const char* text, int32 x, int32 y, Align align)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;

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
    else if (align == Align::LEFT)
    {
      posY -= bounds.y/2;
    }

    // Draws a solid background behind the text. Keep this here for debugging
    //Renderer::pushSprite(streamBuffer, Vector3(posX, posY, 0.0f), Vector2(bounds.x, bounds.y), Rectf(), Color::BLACK);

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
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x , y, w, h);

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

    // We don't want to offset the label twice, so we remove the areaOffset
    const int centerX = x - areaOffset.x + w/2;
    const int centerY = y - areaOffset.y + h/2;
    label(id, text, centerX, centerY, CENTER);
    return returnValue;
  }

  bool GUI::doToggleButton(GUICOntrolID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
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

    // We don't want to offset the label twice, so we remove the areaOffset
    const int centerX = x - areaOffset.x + w/2;
    const int centerY = y - areaOffset.y + h/2;
    label(id, text, centerX, centerY, CENTER);
    return returnValue;
  }

  bool GUI::doRadioButton(GUICOntrolID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = 24;
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, size, size);

    GUISkin::ID bgStyle, tickStyle;

    bool mouseOver = lastRect.containsPoint(root->mouse.getCursorPosition());
    bool returnValue = toggled;

    bool isActiveControl = activeControlId == id;
    bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
    bool upThisFrame = root->mouse.getButtonUp(MOUSE_BUTTON_LEFT);
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);

    bgStyle = GUISkin::CHECKBOX;
    tickStyle = GUISkin::CHECKBOX;

    if (mouseOver)
    {
      hoverControlId = id;

      bgStyle = GUISkin::CHECKBOX_HOVER;

      if (downThisFrame || (isDown && isActiveControl))
      {

        bgStyle = GUISkin::CHECKBOX_ACTIVE;
        activeControlId = id;
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

    // Background

    const float boxX = x / screenW;
    const float boxY = y / screenH;
    const float boxW = size / screenW;
    const float boxH = size / screenH;

    tickStyle = (toggled || isActiveControl) ? GUISkin::CHECKBOX_CHECK : GUISkin::CHECKBOX;
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, 0.0f), Vector2(boxW, boxH), iconRADIO(), skin.color[bgStyle]);
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, 0.0f), Vector2(boxW, boxH), iconRADIO_CHECKED(), skin.color[tickStyle]);

    if (text)
    {
      // We don't want to offset the label twice, so we remove the areaOffset
      const int labelX = x - areaOffset.x + size + DEFAULT_H_SPACING;
      const int labelY = y + (size/2) - areaOffset.y;
      label(id, text, labelX, labelY, LEFT);
      Rect textRect = getLastRect();
      lastRect.w += textRect.w;
    }

    return returnValue;
  }

  bool GUI::doCheckBox(GUICOntrolID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = 24;
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, size, size);

    bool mouseOver = lastRect.containsPoint(root->mouse.getCursorPosition());
    bool returnValue = toggled;
    bool isActiveControl = activeControlId == id;
    bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
    bool upThisFrame = root->mouse.getButtonUp(MOUSE_BUTTON_LEFT);
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);

    GUISkin::ID bgStyle   = GUISkin::CHECKBOX;
    GUISkin::ID tickStyle = GUISkin::CHECKBOX;

    if (mouseOver)
    {
      hoverControlId = id;

      bgStyle = GUISkin::CHECKBOX_HOVER;

      if (downThisFrame || (isDown && isActiveControl))
      {

        bgStyle = GUISkin::CHECKBOX_ACTIVE;
        activeControlId = id;
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

    // Background

    const float boxX = x / screenW;
    const float boxY = y / screenH;
    const float boxW = size / screenW;
    const float boxH = size / screenH;

    tickStyle = (toggled  || isActiveControl) ? GUISkin::CHECKBOX_CHECK : GUISkin::CHECKBOX;
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, 0.0f), Vector2(boxW, boxH), iconCHECKBOX(), skin.color[bgStyle]);
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, 0.0f), Vector2(boxW, boxH), iconCHECKBOX_CHECKED(), skin.color[tickStyle]);

    if (text)
    {
      // We don't want to offset the label twice, so we remove the areaOffset
      const int labelX = x - areaOffset.x + size + DEFAULT_H_SPACING;
      const int labelY = y + (size/2) - areaOffset.y;
      label(id, text, labelX, labelY, LEFT);
      Rect textRect = getLastRect();
      lastRect.w += textRect.w;
    }

    return returnValue;
  }

  float GUI::doHorizontalSlider(GUICOntrolID id, float value, int32 x, int32 y, int32 w)
  {
    const uint32 verticalSize = 24;
    uint32 handleWidth = (uint32)(18 * skin.sliderHandleThickness);
    if (handleWidth < 4) handleWidth = 4;

    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, w, verticalSize);

    float returnValue = value;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    float handlePos = x + value * (w - handleWidth);
    Rect handleRect = Rect((int32)handlePos, y, handleWidth, verticalSize);

    const Point2& cursorPos = root->mouse.getCursorPosition();
    bool mouseOverHandle = handleRect.containsPoint(cursorPos);
    bool isBeingDragged = draggedControlId == id;
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);
    GUISkin::ID handleStyle = GUISkin::SLIDER_HANDLE;

    if(isBeingDragged)
    {
      if (isDown)
      {
        handleStyle = GUISkin::SLIDER_HANDLE_ACTIVE;
        handlePos = (float)(cursorDragOffset.x + cursorPos.x);
      }
      else
      {
        handleStyle = GUISkin::SLIDER_HANDLE_HOVER;
        draggedControlId = 0;
      }

    }
    else if (mouseOverHandle)
    {
      handleStyle = GUISkin::SLIDER_HANDLE_HOVER;
      bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
      if (downThisFrame)
      {
        handleStyle = GUISkin::SLIDER_HANDLE_ACTIVE;
        draggedControlId = id;
        cursorDragOffset = Point2{(int)handlePos - cursorPos.x, 0};
        handlePos = (float)(cursorDragOffset.x + cursorPos.x);
      }
    }

    const int32 leftLimit = x;
    const int32 rightLimit = x + w - handleWidth;
    const int32 sliderLength = rightLimit - leftLimit;

    if (handlePos > rightLimit)
      handlePos = (float) rightLimit;
    if (handlePos < leftLimit)
      handlePos = (float) leftLimit;
    returnValue = (handlePos - x) / (float) sliderLength;

    // Horizontal line
    const float halfVerticalSize = verticalSize / 2.0f;
    const float lineThickness = verticalSize * skin.sliderThickness;
    const float halfLinethickness = halfVerticalSize * skin.sliderThickness;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, (y + halfVerticalSize - halfLinethickness) / screenH, 0.0f), 
        Vector2(w / screenW, lineThickness / screenH),
        Rectf(), skin.color[GUISkin::SLIDER]);

    // handle
    Renderer::pushSprite(streamBuffer,
        Vector3(handlePos / screenW, y / screenH, 0.0f), 
        Vector2(handleWidth / screenW, verticalSize / screenH),
        Rectf(), skin.color[handleStyle]);
    return returnValue;
  }

  float GUI::doVerticalSlider(GUICOntrolID id, float value, int32 x, int32 y, int32 h)
  {
    const uint32 horizontalSize = 24;
    uint32 handleHeight = (uint32)(18 * skin.sliderHandleThickness);
    if (handleHeight < 4) handleHeight = 4;

    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, horizontalSize, h);

    float returnValue = value;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    float handlePos = y + value * (h - handleHeight);
    Rect handleRect = Rect(x, (int32)handlePos, horizontalSize, handleHeight);

    const Point2& cursorPos = root->mouse.getCursorPosition();
    bool mouseOverHandle = handleRect.containsPoint(cursorPos);
    bool isBeingDragged = draggedControlId == id;
    bool isDown = root->mouse.getButton(MOUSE_BUTTON_LEFT);
    GUISkin::ID handleStyle = GUISkin::SLIDER_HANDLE;

    if(isBeingDragged)
    {
      if (isDown)
      {
        handleStyle = GUISkin::SLIDER_HANDLE_ACTIVE;
        handlePos = (float)(cursorDragOffset.y + cursorPos.y);
      }
      else
      {
        handleStyle = GUISkin::SLIDER_HANDLE_HOVER;
        draggedControlId = 0;
      }

    }
    else if (mouseOverHandle)
    {
      handleStyle = GUISkin::SLIDER_HANDLE_HOVER;
      bool downThisFrame = root->mouse.getButtonDown(MOUSE_BUTTON_LEFT);
      if (downThisFrame)
      {
        handleStyle = GUISkin::SLIDER_HANDLE_ACTIVE;
        draggedControlId = id;
        cursorDragOffset = Point2{(int)handlePos - cursorPos.y, 0};
        handlePos = (float)(cursorDragOffset.y + cursorPos.y);
      }
    }

    const int32 bottomLimit = y;
    const int32 topLimit = y + h - handleHeight;
    const int32 sliderLength = topLimit - bottomLimit;

    if (handlePos > topLimit)
      handlePos = (float) topLimit;
    if (handlePos < bottomLimit)
      handlePos = (float) bottomLimit;
    returnValue = (handlePos- y) / (float) sliderLength;

    // vertical line
    const float halfHorizontalSize = horizontalSize / 2.0f;
    const float lineThickness = horizontalSize * skin.sliderThickness;
    const float halfLinethickness = halfHorizontalSize * skin.sliderThickness;
    Renderer::pushSprite(streamBuffer,
        Vector3((x + halfHorizontalSize - halfLinethickness) / screenW, y / screenH, 0.0f), 
        Vector2(lineThickness / screenW, h / screenH),
        Rectf(), skin.color[GUISkin::SLIDER]);

    // handle
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, handlePos / screenH, 0.0f), 
        Vector2(horizontalSize / screenW, handleHeight / screenH),
        Rectf(), skin.color[handleStyle]);
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
    areaCount = 0;
    areaOffset = Rect(0, 0, 0, 0);
    root = SystemsRoot::get();


    skin.sliderThickness = 0.1f;
    skin.sliderHandleThickness = 0.6f;
    skin.windowOpacity = .9f;

    const Color windowBackground        = Color(29, 29, 29);
    const Color panelBackground         = Color(77, 77, 77);
    const Color controlBackground       = Color(15, 15, 15);
    const Color controlSurface          = Color(40, 40, 40);
    const Color controlBackgroundHoover = controlSurface;
    const Color controlSurfaceHover     = Color(50, 50, 50);
    const Color highlight               = Color(0, 10, 250);
    const Color contrastLight           = Color(150, 150, 150);


    skin.color[GUISkin::TEXT]          = Color::WHITE;
    skin.color[GUISkin::TEXT_DISABLED] = Color::GRAY;

    skin.color[GUISkin::BUTTON]        = controlSurface;
    skin.color[GUISkin::BUTTON_HOVER]  = controlSurfaceHover;
    skin.color[GUISkin::BUTTON_ACTIVE] = controlBackground;

    skin.color[GUISkin::TOGGLE_BUTTON]                = controlSurface;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER]          = controlSurfaceHover;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE]   = highlight;
    skin.color[GUISkin::TOGGLE_BUTTON_ACTIVE]         = highlight;

    skin.color[GUISkin::CHECKBOX]               = controlBackground;
    skin.color[GUISkin::CHECKBOX_HOVER]         = controlBackgroundHoover;
    skin.color[GUISkin::CHECKBOX_ACTIVE]        = Color::BLACK;
    skin.color[GUISkin::CHECKBOX_CHECK]         = Color::WHITE;

    skin.color[GUISkin::SLIDER]                 = contrastLight;
    skin.color[GUISkin::SLIDER_HANDLE]          = controlBackground;
    skin.color[GUISkin::SLIDER_HANDLE_HOVER]    = Color::WHITE;
    skin.color[GUISkin::SLIDER_HANDLE_ACTIVE]   = Color::BLACK;

    skin.color[GUISkin::PANEL]                  = panelBackground;

    skin.color[GUISkin::WINDOW]                 = windowBackground;
    skin.color[GUISkin::WINDOW_TITLE_BAR]       = controlSurface;
    skin.color[GUISkin::WINDOW_TITLE_BAR_HOVER] = controlSurfaceHover;

    skin.color[GUISkin::SEPARATOR]              = Color(180, 180, 180);
  }

#endif

}
