#include <smol/smol_gui.h>
#include <smol/smol_material.h>
#include <smol/smol_input_manager.h>
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
    changed = false;
    if (enabled)
    {
      Mouse& mouse = InputManager::get().mouse;
      LMBDownThisFrame = mouse.getButtonDown(MOUSE_BUTTON_LEFT);
      LMBUpThisFrame = mouse.getButtonUp(MOUSE_BUTTON_LEFT);
      LMBIsDown = mouse.getButton(MOUSE_BUTTON_LEFT);
      mouseCursorPosition = mouse.getCursorPosition();
    }
    else
    {
      hoverControlId = 0;
      activeControlId = 0;
      draggedControlId = 0;
      mouseCursorPosition = Point2{-1, -1};
      LMBDownThisFrame = false;
      LMBUpThisFrame = false;
      LMBIsDown = false;
    }

    z = 0.0f;
    currentCursorZ = 0.0f;
    popupCount = 0;
    windowCount = 0;

    if (glyphDrawDataArena.getCapacity() == 0)
    {
      glyphDrawDataArena.initialize(256 * sizeof(GlyphDrawData));
      Renderer::createStreamBuffer(&streamBuffer, 1024);
    }
    glyphDrawDataArena.reset();
    Renderer::begin(streamBuffer);
  }

  void GUI::panel(GUIControlID id, int32 x, int32 y, int32 w, int32 h)
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

  Point2 GUI::beginWindow(GUIControlID id, const char* title, int32 x, int32 y, int32 w, int32 h, bool topmost)
  {
    lastRect = Rect(x, y, w, h);
    windowCount++;

    // Bring the topmost window way above other windows

    if (topmost)
      topmostWindowId = id;

    if (topmostWindowId == id)
      z = -0.5f + windowCount * 0.01f;
    else
      z = -0.1f + windowCount * 0.01f;

    bool isBeingDragged = draggedControlId == id;
    const int titleBarHeight = 30;
    Rect titleBarRect(x, y, w, titleBarHeight);
    bool mouseOverTitleBar = titleBarRect.containsPoint(mouseCursorPosition);

    // Draw title bar
    GUISkin::ID styleId = (mouseOverTitleBar || isBeingDragged) ? GUISkin::WINDOW_TITLE_BAR_HOVER : GUISkin::WINDOW_TITLE_BAR; 
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, z), 
        Vector2(w / screenW, titleBarHeight / screenH),
        Rectf(), skin.color[styleId]);

    label(id, title, x + DEFAULT_H_SPACING,
        (int32)(y + (titleBarHeight/2.0f)),
        0,
        LEFT);

    // draw the window panel
    Color windowColor = skin.color[GUISkin::WINDOW];
    windowColor.a = skin.windowOpacity;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, (y + titleBarHeight) / screenH, z), 
        Vector2(w / screenW, (h  - titleBarHeight) / screenH),
        Rectf(), windowColor);

    beginArea(x, y + titleBarHeight, w, h - titleBarHeight);
    lastRect = Rect(x, y, w, h);

    // check for dragging the title bar
    Point2 newPos = Point2{x, y};
    const Point2& cursorPos = mouseCursorPosition;

    if(isBeingDragged)
    {
      if (mouseLButtonIsDown())
      {
        topmostWindowId = id;
        currentCursorZ = z;
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
      if (mouseLButtonDownThisFrame())
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

  void GUI::label(GUIControlID id, const char* text, int32 x, int32 y, int32 w, Align align, Color bg)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;

    const float fontSize =  skin.labelFontSize;
    const float scaleX  = fontSize / screenW;
    const float scaleY  = fontSize / screenH;
    float posX          = x / screenW;
    float posY          = y / screenH;
    const size_t textLen = strlen(text);

    GlyphDrawData* drawData =
      (GlyphDrawData*) glyphDrawDataArena.pushSize((1 + textLen) * sizeof(GlyphDrawData));

    GUISkin::ID textColor = enabled ?  GUISkin::TEXT : GUISkin::TEXT_DISABLED;
    Vector2 bounds = skin.font->computeString(text, skin.color[textColor], drawData, w / (float)fontSize, 1.0f + skin.lineHeightAdjust);
    bounds.mult(scaleX, scaleY);
  

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

    lastRect = Rect((int32)(posX * screenW), (int32)(posY * screenW), (int32) (bounds.x * screenW), (int32) (bounds.y * screenH));

    // Draws a solid background behind the text. Keep this here for debugging
    if (bg.a > 0.00f)
      Renderer::pushSprite(streamBuffer, Vector3(posX, posY, 0.0f), Vector2(bounds.x, bounds.y), Rectf(), bg);

    for (int i = 0; i < textLen; i++)
    {
      GlyphDrawData& data = drawData[i];
      Vector3 offset = Vector3(posX + data.position.x * scaleX, posY + data.position.y *  scaleY, z);
      Vector2 size = data.size;
      size.mult(scaleX, scaleY);
      Renderer::pushSprite(streamBuffer, offset, size, data.uv, data.color);
    }

  }

  bool GUI::doLabelButton(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h, Align align, Color bg)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x , y, w, h);

    bool returnValue = false;
    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool isActiveControl = activeControlId == id;

    if (mouseOver)
    {
      hoverControlId = id;
      Renderer::pushSprite(streamBuffer,
          Vector3(x / screenW, y / screenH, z), 
          Vector2(w / screenW, h / screenH),
          Rectf(), skin.color[GUISkin::BUTTON_HOVER]);

      if (mouseLButtonDownThisFrame() || (mouseLButtonIsDown() && isActiveControl))
      {
        activeControlId = id;

      }
      else if(mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
        returnValue = true;
        changed = true;
      }
    }
    else
    {
      if (mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
      }
    }

    // We don't want to offset the label twice, so we remove the areaOffset
    const int centerX = x - areaOffset.x + w/2;
    const int centerY = y - areaOffset.y + h/2;
    label(id, text, centerX, centerY, 0, align, bg);
    return returnValue;
  }

  bool GUI::doButton(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x , y, w, h);

    GUISkin::ID styleId;
    bool returnValue = false;
    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool isActiveControl = activeControlId == id;

    if (mouseOver)
    {
      hoverControlId = id;
      styleId = GUISkin::BUTTON_HOVER;

      if (mouseLButtonDownThisFrame() || (mouseLButtonIsDown() && isActiveControl))
      {
        activeControlId = id;
        styleId = GUISkin::BUTTON_ACTIVE;
      }
      else if(mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
        returnValue = true;
        changed = true;
      }
    }
    else
    {
      //hoverControlId = 0;
      styleId = GUISkin::BUTTON;
      if (mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
      }
    }

    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, z), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    // We don't want to offset the label twice, so we remove the areaOffset
    const int centerX = x - areaOffset.x + w/2;
    const int centerY = y - areaOffset.y + h/2;
    label(id, text, centerX, centerY, 0, CENTER);
    return returnValue;
  }

  bool GUI::doToggleButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, w, h);

    GUISkin::ID styleId;
    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool returnValue = toggled;

    bool isActiveControl = activeControlId == id;

    if (toggled)
      styleId = GUISkin::TOGGLE_BUTTON_ACTIVE;
    else
      styleId = GUISkin::TOGGLE_BUTTON;

    if (mouseOver)
    {
      hoverControlId = id;
      styleId = toggled ? GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE : GUISkin::TOGGLE_BUTTON_HOVER;

      if (mouseLButtonDownThisFrame() || (mouseLButtonIsDown() && isActiveControl))
      {
        activeControlId = id;
        styleId = GUISkin::TOGGLE_BUTTON_ACTIVE;
      }
      else if(mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
        returnValue = !toggled;
        changed = true;
      }
    }
    else
    {
      hoverControlId = 0;
      if (mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
      }
    }

    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, z), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    // We don't want to offset the label twice, so we remove the areaOffset
    const int centerX = x - areaOffset.x + w/2;
    const int centerY = y - areaOffset.y + h/2;
    label(id, text, centerX, centerY, CENTER);
    return returnValue;
  }

  bool GUI::doRadioButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = DEFAULT_CONTROL_HEIGHT;
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, size, size);

    GUISkin::ID bgStyle;
    GUISkin::ID tickStyle;
    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool returnValue = toggled;
    bool isActiveControl = activeControlId == id;

    bgStyle = GUISkin::CHECKBOX;
    tickStyle = GUISkin::CHECKBOX;

    if (mouseOver)
    {
      hoverControlId = id;

      bgStyle = GUISkin::CHECKBOX_HOVER;

      if (mouseLButtonDownThisFrame() || (mouseLButtonIsDown() && isActiveControl))
      {

        bgStyle = GUISkin::CHECKBOX_ACTIVE;
        activeControlId = id;
      }
      else if(mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
        returnValue = !toggled;
        changed = true;
      }
    }
    else
    {
      hoverControlId = 0;
      if (mouseLButtonUpThisFrame() && isActiveControl)
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
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, z), Vector2(boxW, boxH), skin.spriteRadioButton, skin.color[bgStyle]);
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, z), Vector2(boxW, boxH), skin.spriteRadioButtonChecked, skin.color[tickStyle]);

    if (text)
    {
      // We don't want to offset the label twice, so we remove the areaOffset
      const int labelX = x - areaOffset.x + size + DEFAULT_H_SPACING;
      const int labelY = y + (size/2) - areaOffset.y;
      label(id, text, labelX, labelY, 0, LEFT);
      Rect textRect = getLastRect();
      lastRect.w += textRect.w;
    }

    return returnValue;
  }

  bool GUI::doCheckBox(GUIControlID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = DEFAULT_CONTROL_HEIGHT;
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, size, size);

    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool returnValue = toggled;
    bool isActiveControl = activeControlId == id;

    GUISkin::ID bgStyle   = GUISkin::CHECKBOX;
    GUISkin::ID tickStyle = GUISkin::CHECKBOX;

    if (mouseOver)
    {
      hoverControlId = id;
      bgStyle = GUISkin::CHECKBOX_HOVER;
      if (mouseLButtonDownThisFrame() || (mouseLButtonIsDown() && isActiveControl))
      {
        bgStyle = GUISkin::CHECKBOX_ACTIVE;
        activeControlId = id;
      }
      else if(mouseLButtonUpThisFrame() && isActiveControl)
      {
        activeControlId = 0;
        returnValue = !toggled;
        changed = true;
      }
    }
    else
    {
      hoverControlId = 0;
      if (mouseLButtonUpThisFrame() && isActiveControl)
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
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, z), Vector2(boxW, boxH), skin.spriteCheckBox, skin.color[bgStyle]);
    Renderer::pushSprite(streamBuffer, Vector3(boxX, boxY, z), Vector2(boxW, boxH), skin.spriteCheckBoxChecked, skin.color[tickStyle]);

    if (text)
    {
      // We don't want to offset the label twice, so we remove the areaOffset
      const int labelX = x - areaOffset.x + size + DEFAULT_H_SPACING;
      const int labelY = y + (size/2) - areaOffset.y;
      label(id, text, labelX, labelY, 0, LEFT);
      Rect textRect = getLastRect();
      lastRect.w += textRect.w;
    }

    return returnValue;
  }

  float GUI::doHorizontalSlider(GUIControlID id, float value, int32 x, int32 y, int32 w)
  {
    //inner handle scales according to user action
    const float handleScaleHover  = 0.8f;
    const float handleScaleNormal = 0.65f;
    const float handleScaleDrag   = 0.5f;
    const uint32 handleSize = DEFAULT_CONTROL_HEIGHT;
    float innerHandleScale = handleScaleNormal;

    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, w, handleSize);

    float returnValue = value;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    float handlePos = x + value * (w - handleSize);
    Rect handleRect = Rect((int32)handlePos, y, handleSize, handleSize);

    const Point2& cursorPos = mouseCursorPosition;
    bool mouseOverHandle = handleRect.containsPoint(cursorPos) && (z <= currentCursorZ);
    bool isBeingDragged = draggedControlId == id;
    bool isActiveControl = activeControlId == id;

    if(isBeingDragged && isActiveControl)
    {
      if (mouseLButtonIsDown())
      {
        innerHandleScale = handleScaleDrag;
        handlePos = (float)(cursorDragOffset.x + cursorPos.x);
      }
      else
      {
        draggedControlId = 0;
        activeControlId = 0;
      }
    }
    else if (mouseOverHandle)
    {
      innerHandleScale = handleScaleHover;
      if (mouseLButtonDownThisFrame())
      {
        draggedControlId = id;
        activeControlId = id;
        cursorDragOffset = Point2{(int)handlePos - cursorPos.x, 0};
        handlePos = (float)(cursorDragOffset.x + cursorPos.x);
      }
    }

    const int32 leftLimit = x;
    const int32 rightLimit = x + w - handleSize;
    const int32 sliderLength = rightLimit - leftLimit;

    if (handlePos > rightLimit)
      handlePos = (float) rightLimit;
    if (handlePos < leftLimit)
      handlePos = (float) leftLimit;
    returnValue = (handlePos - x) / (float) sliderLength;

    if (returnValue != value)
      changed = true;

    // Horizontal line
    const float halfHandleSize = handleSize / 2.0f;
    const float lineThickness = handleSize * skin.sliderThickness;
    const float halfLinethickness = halfHandleSize * skin.sliderThickness;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, (y + halfHandleSize - halfLinethickness) / screenH, z), 
        Vector2(w / screenW, lineThickness / screenH),
        Rectf(), skin.color[GUISkin::SLIDER]);

    // handle
    Renderer::pushSprite(streamBuffer,
        Vector3(handlePos / screenW, y / screenH, z), 
        Vector2(handleSize / screenW, handleSize / screenH),
        skin.spriteSliderHandle, skin.color[GUISkin::SLIDER_HANDLE]);

    // inner handle
    float innerHandleSize = handleSize * innerHandleScale;
    float halfInnerHandleSize = innerHandleSize / 2.0f;
    Renderer::pushSprite(streamBuffer,
        Vector3(
          (handlePos + (halfHandleSize) - halfInnerHandleSize) / screenW,
          (y + halfHandleSize - halfInnerHandleSize) / screenH, z), 
        Vector2(innerHandleSize / screenW, innerHandleSize / screenH),
        skin.spriteSliderHandle, skin.color[GUISkin::SLIDER_HANDLE_INNER]);

    return returnValue;
  }

  float GUI::doVerticalSlider(GUIControlID id, float value, int32 x, int32 y, int32 h)
  {
    //inner handle scales according to user action
    const float handleScaleHover  = 0.8f;
    const float handleScaleNormal = 0.65f;
    const float handleScaleDrag   = 0.5f;
    const uint32 handleSize = DEFAULT_CONTROL_HEIGHT;
    float innerHandleScale = handleScaleNormal;

    x = areaOffset.x + x;
    y = areaOffset.y + y;
    lastRect = Rect(x, y, handleSize, h);

    float returnValue = value;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    float handlePos = y + value * (h - handleSize);
    Rect handleRect = Rect(x, (int32)handlePos, handleSize, handleSize);

    const Point2& cursorPos = mouseCursorPosition;
    bool mouseOverHandle = handleRect.containsPoint(cursorPos) && (z <= currentCursorZ);
    bool isBeingDragged = draggedControlId == id;

    if(isBeingDragged)
    {
      if (mouseLButtonIsDown())
      {
        innerHandleScale = handleScaleDrag;
        handlePos = (float)(cursorDragOffset.y + cursorPos.y);
      }
      else
      {
        draggedControlId = 0;
      }
    }
    else if (mouseOverHandle)
    {
      innerHandleScale = handleScaleHover;
      if (mouseLButtonDownThisFrame())
      {
        draggedControlId = id;
        cursorDragOffset = Point2{0, (int)handlePos - cursorPos.y};
        handlePos = (float)(cursorDragOffset.y + cursorPos.y);
      }
    }

    const int32 bottomLimit = y;
    const int32 topLimit = y + h - handleSize;
    const int32 sliderLength = topLimit - bottomLimit;

    if (handlePos > topLimit)
      handlePos = (float) topLimit;
    if (handlePos < bottomLimit)
      handlePos = (float) bottomLimit;
    returnValue = (handlePos - y) / (float) sliderLength;

    if (returnValue != value)
      changed = true;

    // vertical line
    const float halfHandleSize = handleSize / 2.0f;
    const float lineThickness = handleSize * skin.sliderThickness;
    const float halfLinethickness = halfHandleSize * skin.sliderThickness;
    Renderer::pushSprite(streamBuffer,
        Vector3((x + halfHandleSize - halfLinethickness) / screenW, y / screenH, z), 
        Vector2(lineThickness / screenW, h / screenH),
        Rectf(), skin.color[GUISkin::SLIDER]);

    // handle
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, handlePos / screenH, z), 
        Vector2(handleSize / screenW, handleSize / screenH),
        skin.spriteSliderHandle, skin.color[GUISkin::SLIDER_HANDLE]);

    // inner handle
    float innerHandleSize = handleSize * innerHandleScale;
    float halfInnerHandleSize = innerHandleSize / 2.0f;
    Renderer::pushSprite(streamBuffer,
        Vector3(
          (x + halfHandleSize - halfInnerHandleSize) / screenW,
          (handlePos + (halfHandleSize) - halfInnerHandleSize) / screenH, z), 
        Vector2(innerHandleSize / screenW, innerHandleSize / screenH),
        skin.spriteSliderHandle, skin.color[GUISkin::SLIDER_HANDLE_INNER]);

    return returnValue;
  }

  int32 GUI::doOptionList(GUIControlID  id, const char** options, uint32 optionCount, uint32 x, uint32 y, uint32 minWidth)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    popupCount++;

    // We should draw on top of all previous controls...
    float oldZ = z;
    // Popups are the frontmost possible controls. So we always draw them way above everything else.
    z= -0.95f + popupCount * 0.001f;

    int32 selectedOption = -1;
    const Point2 mousePos = mouseCursorPosition;
    const int vSpacing = 1;
    const float totalMenuHeight = ((optionCount * skin.labelFontSize) + (optionCount * vSpacing));

    // First we draw all the labels to figure out how large this popup should be.
    bool isActiveControl = activeControlId == id;
    Rectf selectionRect((float)x,(float) y,(float) minWidth, skin.labelFontSize);
    bool mouseOver = false;
    for (uint32 i = 0; i < optionCount; i++)
    { 
      // label
      label(id, options[i],
          (int32)selectionRect.x + DEFAULT_H_SPACING - areaOffset.x,
          (int32) selectionRect.y - areaOffset.y, 0, NONE, skin.color[GUISkin::MENU]);

      if (lastRect.w + (int32)DEFAULT_H_SPACING * skin.labelFontSize * 0.5f > selectionRect.w)
        selectionRect.w = lastRect.w + (int32)DEFAULT_H_SPACING * skin.labelFontSize * 0.5f;

      selectionRect.y += skin.labelFontSize + vSpacing;
    }

    // Reset y so we start drawing the selection from the top again
    selectionRect.y = (float) y;
    // Draw the background
    Renderer::pushSprite(streamBuffer,
        Vector3(selectionRect.x / screenW, selectionRect.y / screenH, z + 0.01f),
        Vector2(selectionRect.w / screenW, totalMenuHeight / screenH),
        Rectf(), skin.color[GUISkin::MENU]);

    // Test for mouse interaction on each label
    for (uint32 i = 0; i < optionCount; i++)
    { 
      if (selectionRect.containsPoint(mousePos) && z < currentCursorZ)
      {
        currentCursorZ = z;
        hoverControlId = id;
        mouseOver = true;
        Renderer::pushSprite(streamBuffer,
            Vector3(selectionRect.x / screenW , selectionRect.y / screenH, z),
            Vector2(selectionRect.w / screenW, selectionRect.h / screenH),
            Rectf(), skin.color[GUISkin::MENU_SELECTION]);

        // redraw the label over the selection
        label(id, options[i],
            (int32)selectionRect.x + DEFAULT_H_SPACING - areaOffset.x,
            (int32) selectionRect.y - areaOffset.y, 0, NONE, skin.color[GUISkin::MENU_SELECTION]);

        if (mouseLButtonDownThisFrame() || (isActiveControl && mouseLButtonIsDown()))
          activeControlId = id;
        else if(mouseLButtonUpThisFrame() && isActiveControl)
        {
          activeControlId = 0;
          selectedOption = i;
          changed = true;
        }
      }

      selectionRect.y += skin.labelFontSize + vSpacing;
    }

    if (mouseLButtonDownThisFrame() && !mouseOver)
    {
      activeControlId = 0;
      hoverControlId = 0;
    }

    // We resotre the previous global Z
    z = oldZ;
    return selectedOption;
  }


  int32 GUI::doPopupMenu(GUIControlID  id, const char** options, uint32 optionCount, uint32 x, uint32 y, uint32 maxWidth)
  {
    return false;
  }

  int32 GUI::doComboBox(GUIControlID  id, const char** options, uint32 optionCount, int32 selectedIndex, uint32 x, uint32 y, uint32 w)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    uint32 h = DEFAULT_CONTROL_HEIGHT;
    lastRect = Rect(x , y, w, h);

    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool isActiveControl = activeControlId == id;


    // BOX
    GUISkin::ID styleId = mouseOver ? GUISkin::COMBO_BOX : GUISkin::COMBO_BOX_HOVER;
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, z), 
        Vector2(w / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    // RIGHT SIDE "chevron icon"
    Renderer::pushSprite(streamBuffer,
        Vector3((x + w - DEFAULT_CONTROL_HEIGHT - DEFAULT_H_SPACING) / screenW, y / screenH, z), 
        Vector2((int32) DEFAULT_CONTROL_HEIGHT / screenW, (int32) DEFAULT_CONTROL_HEIGHT / screenH),
        skin.spriteComboBoxChevron, Color::WHITE);

    // Label
    const char* text = "";
    if (selectedIndex >= 0 && selectedIndex < (int32) optionCount)
      text = options[selectedIndex];

    x -= areaOffset.x;
    y -= areaOffset.y;
    // We don't want to offset the label twice, so we remove the areaOffset
    const int labelX = x + DEFAULT_H_SPACING;
    const int labelY = y + h/2;
    label(id, text, labelX, labelY, 0, LEFT);

    int32 newSelectedIndex = selectedIndex;
    if (isActiveControl)
    {
      newSelectedIndex = doOptionList(id, options, optionCount, x, y + h, w);
    }
    else if (mouseOver)
    {
      hoverControlId = id;
      if (mouseLButtonDownThisFrame() && !isActiveControl)
      {
        activeControlId = id;
      }
    }

    if (newSelectedIndex >= 0)
      selectedIndex = newSelectedIndex;

    return selectedIndex;
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

  inline bool GUI::mouseLButtonDownThisFrame()
  {
    return LMBDownThisFrame && enabled;
  }

  bool GUI::mouseLButtonUpThisFrame()
  {
    return LMBUpThisFrame && enabled;
  }

  bool GUI::mouseLButtonIsDown()
  {
    return LMBIsDown && enabled;
  }

  void GUI::initialize(Handle<Material> material, Handle<Font> font)
  {
    Renderer::createStreamBuffer(&streamBuffer, 512);
    this->material = material;
    skin.font = font;
    skin.labelFontSize = 16;
    skin.lineHeightAdjust = 1.0f;
    areaCount = 0;
    areaOffset = Rect(0, 0, 0, 0);
    z = 0.0f;
    enabled = true;
    drawLabelDebugBackground = false;

    skin.sliderThickness = 0.1f;
    skin.windowOpacity = .98f;

    const Color windowBackground        = Color(29, 29, 29);
    const Color panelBackground         = Color(77, 77, 77);
    const Color controlBackground       = Color(15, 15, 15);
    const Color controlSurface          = Color(40, 40, 40);
    const Color controlBackgroundHoover = controlSurface;
    const Color controlSurfaceHover     = Color(50, 50, 50);
    const Color highlight               = Color(0, 10, 250);
    const Color contrastLight           = Color(150, 150, 150);

    skin.color[GUISkin::TEXT]                   = Color::WHITE;
    skin.color[GUISkin::TEXT_DEBUG_BACKGROUND]  = Color::BLACK;
    skin.color[GUISkin::TEXT_DISABLED]          = Color::GRAY;

    skin.color[GUISkin::BUTTON]        = controlSurface;
    skin.color[GUISkin::BUTTON_HOVER]  = controlSurfaceHover;
    skin.color[GUISkin::BUTTON_ACTIVE] = controlBackground;

    skin.color[GUISkin::TOGGLE_BUTTON]                = controlSurface;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER]          = controlSurfaceHover;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE]   = highlight;
    skin.color[GUISkin::TOGGLE_BUTTON_ACTIVE]         = highlight;

    skin.color[GUISkin::COMBO_BOX]              = controlBackground;
    skin.color[GUISkin::COMBO_BOX_HOVER]         = controlBackgroundHoover;

    skin.color[GUISkin::CHECKBOX]               = controlBackground;
    skin.color[GUISkin::CHECKBOX_HOVER]         = controlBackgroundHoover;
    skin.color[GUISkin::CHECKBOX_ACTIVE]        = Color::BLACK;
    skin.color[GUISkin::CHECKBOX_CHECK]         = Color::WHITE;

    skin.color[GUISkin::SLIDER]                 = contrastLight;
    skin.color[GUISkin::SLIDER_HANDLE]          = contrastLight;
    skin.color[GUISkin::SLIDER_HANDLE_INNER]    = controlBackground;

    skin.color[GUISkin::PANEL]                  = panelBackground;

    skin.color[GUISkin::WINDOW]                 = windowBackground;
    skin.color[GUISkin::WINDOW_TITLE_BAR]       = controlSurface;
    skin.color[GUISkin::WINDOW_TITLE_BAR_HOVER] = controlSurfaceHover;


    skin.color[GUISkin::MENU]             = windowBackground;
    skin.color[GUISkin::MENU_SELECTION]   = highlight;

    skin.color[GUISkin::SEPARATOR]              = controlSurfaceHover;
  }

#endif

}
