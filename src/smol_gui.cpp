#include "include/smol/smol_gui.h"
#include <exception>
#include <smol/smol_gui.h>
#include <smol/smol_material.h>
#include <smol/smol_input_manager.h>
#include <smol/smol_event_manager.h>
#include <smol/smol_platform.h>

namespace smol
{
  const float CURSOR_WAIT_MILLISECONDS_ON_EVENT = 0.48f;

  static bool onEventForwarder(const Event& event, void* ptrGUI)
  {
    GUI* gui = (GUI*) ptrGUI;
    return gui->onEvent(event, nullptr);
  };

  bool GUI::onEvent(const Event& event, void* payload)
  {
    if (event.type == Event::TEXT)
    {
      cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
      if (event.textEvent.type == TextEvent::BACKSPACE)
      {
        input.deleteCharacterBeforeCursor();
        changed = true;
      }
      else
      {
        input.addCharacterAtCursor(event.textEvent.character);
        changed = true;
      }
      return true;
    }

    if (event.type == Event::KEYBOARD)
    {
      if (event.keyboardEvent.type == KeyboardEvent::KEY_DOWN || event.keyboardEvent.type == KeyboardEvent::KEY_HOLD)
      {
        Keycode keycode = event.keyboardEvent.keyCode;
        if (keycode != KEYCODE_DELETE &&
            keycode != KEYCODE_HOME &&
            keycode != KEYCODE_END &&
            keycode != KEYCODE_LEFT &&
            keycode != KEYCODE_RIGHT)
          return false;

        bool hasSelection = input.getSelectionStartIndex() >= 0;
        if (event.keyboardEvent.shiftIsDown && !hasSelection)
          input.beginSelectionAtCursor();
        else if (!event.keyboardEvent.shiftIsDown && hasSelection)
          input.clearSelection();

        if (event.keyboardEvent.keyCode == Keycode::KEYCODE_DELETE)
        {
          input.deleteCharacterAfterCursor();
          changed = true;
        }
        else if (event.keyboardEvent.keyCode == Keycode::KEYCODE_HOME)
        {
          input.moveCursorHome();
          cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
          return true;
        }
        else if (event.keyboardEvent.keyCode == Keycode::KEYCODE_END)
        {
          cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
          input.moveCursorEnd();
          return true;
        }
        else if (event.keyboardEvent.keyCode == Keycode::KEYCODE_LEFT)
        {
          cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
          input.moveCursorLeft();
          return true;
        }
        else if (event.keyboardEvent.keyCode == Keycode::KEYCODE_RIGHT)
        {
          cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
          input.moveCursorRight();
          return true;
        }
      }
    }

    return false;
  }

  Vector2 GUI::getScreenSize() const { return Vector2(screenW, screenH); }

  GUISkin& GUI::getSkin() { return skin; }

  Rect GUI::getLastRect() const { return lastRect; }

  void GUI::begin(float deltaTime, int screenWidth, int screenHeight)
  {
    screenW = (float) screenWidth;
    screenH = (float) screenHeight;
    changed = false;
    this->deltaTime = deltaTime;

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

  //
  // Controls
  //
  
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
    currentWindowId = id;

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

    if (lastRect.containsPoint(mouseCursorPosition) && mouseLButtonDownThisFrame())
    {
      topmostWindowId = id;
      currentCursorZ = z;
    }

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
        topmostWindowId = id;
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

    currentWindowId = 0;
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

  void GUI::drawText(const char* text, int32 x, int32 y, int w, Align align, Color bgColor)
  {
    const float fontSize =  skin.labelFontSize;
    const float scaleX  = fontSize / screenW;
    const float scaleY  = fontSize / screenH;
    float posX          = x / screenW;
    float posY          = y / screenH;
    const size_t textLen = strlen(text);

    GlyphDrawData* drawData =
      (GlyphDrawData*) glyphDrawDataArena.pushSize(textLen * sizeof(GlyphDrawData));

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
    if (bgColor.a > 0.00f)
      Renderer::pushSprite(streamBuffer, Vector3(posX, posY, 0.0f), Vector2(bounds.x, bounds.y), Rectf(), bgColor);

    for (int i = 0; i < textLen; i++)
    {
      GlyphDrawData& data = drawData[i];
      Vector3 offset = Vector3(posX + data.position.x * scaleX, posY + data.position.y *  scaleY, z);
      Vector2 size = data.size;
      size.mult(scaleX, scaleY);
      Renderer::pushSprite(streamBuffer, offset, size, data.uv, data.color);
    }

    if (input.isEnabled())
    {
      if (textLen == 0)
      {
        // Beggining of the string
        cursorXPosition = posX;
      }
      else
      {
        int32 cursorIndex = input.getCursorIndex();
        // End, past the of last character
        if (cursorIndex < textLen)
        {
          GlyphDrawData& data = drawData[cursorIndex];
          cursorXPosition = posX + data.position.x * scaleX;
        }
        else {
          GlyphDrawData& data = drawData[cursorIndex - 1];
          cursorXPosition = posX + data.position.x * scaleX + data.size.x * scaleX;
        }

        int32 selectionStartIndex = input.getSelectionStartIndex();
        if (selectionStartIndex >= 0)
        {
          // End, past the of last character
          if (selectionStartIndex < textLen)
          {
            GlyphDrawData& data = drawData[selectionStartIndex];
            selectionXPosition = posX + data.position.x * scaleX;
          }
          else {
            GlyphDrawData& data = drawData[selectionStartIndex - 1];
            selectionXPosition = posX + data.position.x * scaleX + data.size.x * scaleX;
          }
        }
      }
    }
  }

  void GUI::label(GUIControlID id, const char* text, int32 x, int32 y, int32 w, Align align, Color bg)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    drawText(text, x, y, w, align, bg);
  }

  bool GUI::labelButton(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h, Align align, Color bg)
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

  bool GUI::button(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h)
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

  bool GUI::toggleButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h)
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

  bool GUI::radioButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = (int32)(0.8f * (int32)DEFAULT_CONTROL_HEIGHT);
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

  bool GUI::checkBox(GUIControlID id, const char* text, bool toggled, int32 x, int32 y)
  {
    const uint32 size = (int32)(0.8f * (int32)DEFAULT_CONTROL_HEIGHT);
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

  float GUI::horizontalSlider(GUIControlID id, float value, int32 x, int32 y, int32 w)
  {
    //inner handle scales according to user action
    const float handleScaleHover  = 0.8f;
    const float handleScaleNormal = 0.65f;
    const float handleScaleDrag   = 0.5f;
    const uint32 handleSize = (int32) (0.8f * (int32)DEFAULT_CONTROL_HEIGHT);
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
        if (topmostWindowId)
          topmostWindowId = currentWindowId;

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

  float GUI::verticalSlider(GUIControlID id, float value, int32 x, int32 y, int32 h)
  {
    //inner handle scales according to user action
    const float handleScaleHover  = 0.8f;
    const float handleScaleNormal = 0.65f;
    const float handleScaleDrag   = 0.5f;
    const uint32 handleSize = (int32) (0.8f * (int32)DEFAULT_CONTROL_HEIGHT);
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
        if (topmostWindowId)
          topmostWindowId = currentWindowId;

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

  char* GUI::textBox(GUIControlID id, char* buffer, size_t bufferCapacity, int32 x, int32 y, int32 width)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    uint32 h = DEFAULT_CONTROL_HEIGHT;
    lastRect = Rect(x , y, width, h);

    bool mouseOver = lastRect.containsPoint(mouseCursorPosition) && (z <= currentCursorZ);
    bool isActiveControl = activeControlId == id;

    GUISkin::ID styleId = skin.TEXT_INPUT;

    if (isActiveControl)
    {
      styleId = skin.TEXT_INPUT_ACTIVE;

      if (mouseLButtonDownThisFrame() && !mouseOver)
      {
        activeControlId = 0;
        hoverControlId = 0;
        isActiveControl = false;
        endTextInput();
      }
    }
    else if (mouseOver)
    {
      hoverControlId = id;
      styleId = skin.TEXT_INPUT_HOVER;

      if (mouseLButtonDownThisFrame())
      {
        cursorAnimateWaitMilisseconds = CURSOR_WAIT_MILLISECONDS_ON_EVENT;
        activeControlId = id;
        hoverControlId = id;
        beginTextInput(buffer, bufferCapacity);
        if (topmostWindowId)
          topmostWindowId = currentWindowId;
      }
    }

    // Box
    Renderer::pushSprite(streamBuffer,
        Vector3(x / screenW, y / screenH, z), 
        Vector2(width / screenW, h / screenH),
        Rectf(), skin.color[styleId]);

    // Text
    const int labelX = (x - areaOffset.x) + DEFAULT_H_SPACING;
    const int labelY = (y - areaOffset.y) + h/2;
    label(id, buffer, labelX, labelY, 0, LEFT);

    // Cursor
    // Note that label() updates the this->cursorXPosition if isTextInputEnabled == true
    if (isActiveControl)
    { 
      Color c = skin.color[skin.CURSOR];
      if (cursorAnimateWaitMilisseconds <= 0.0f)
      {
        cursorAnimateWaitMilisseconds = 0.0f;

        c.a = (float) sin(4 * Platform::getSecondsSinceStartup());
      }
      else
      {
        c = skin.color[skin.CURSOR_HOT];
        cursorAnimateWaitMilisseconds -= deltaTime;
      }

      Renderer::pushSprite(streamBuffer,
          Vector3(cursorXPosition, (y + 2) / screenH, z), 
          Vector2(1 / screenW, (h - 4) / screenH),
          Rectf(), c);

      if (input.getSelectionStartIndex() >= 0)
      {
        float sP, sW;
        if (cursorXPosition > selectionXPosition)
        {
          sP = cursorXPosition;
          sW = selectionXPosition - cursorXPosition;
        }
        else
        {
          sP = selectionXPosition;
          sW = cursorXPosition - selectionXPosition;
        }

        c = skin.color[GUISkin::TEXT_SELECTION];
        c.a = 0.5f;
        Renderer::pushSprite(streamBuffer,
            Vector3(sP, y / screenH, z), 
            Vector2(sW, h / screenH),
            Rectf(), c);
      }
    }

    return buffer;
  }

  int32 GUI::popupMenu(GUIControlID  id, const char** options, uint32 optionCount, uint32 x, uint32 y, uint32 minWidth, uint32 defaultSelection)
  {
    x = areaOffset.x + x;
    y = areaOffset.y + y;
    popupCount++;

    // We should draw on top of all previous controls...
    float oldZ = z;
    // Popups are the frontmost possible controls. So we always draw them way above everything else.
    z= -0.95f + popupCount * 0.001f;

    int32 selectedOption = POPUP_MENU_IDLE;
    const Point2 mousePos = mouseCursorPosition;
    const int vSpacing = 1;
    const float minHeight = DEFAULT_CONTROL_HEIGHT;
    const float controlHeight = skin.labelFontSize > (float) minHeight ? skin.labelFontSize : (float) minHeight;
    const float halfControlHeight = controlHeight / 2.0f;
    float chevronSize = ((float) controlHeight * 0.5f);


    //
    // Draw all labels first so we know the final dimention of the popup
    //
    bool isActiveControl = activeControlId == id;
    Rectf selectionRect((float)x,(float) y,(float) minWidth, controlHeight);
    bool mouseOver = false;
    for (uint32 i = 0; i < optionCount; i++)
    { 
      const char* strLabel = options[i];
      if (strLabel[0] != 0)
      {
        if (strLabel[0] == '>') // it's a hover activated entry
          strLabel++;

        label(id, strLabel,
            (int32) selectionRect.x + DEFAULT_H_SPACING - areaOffset.x,
            (int32) (selectionRect.y - areaOffset.y + halfControlHeight - (skin.labelFontSize / 2.0f)), 0, NONE, skin.color[GUISkin::MENU]);

        if (lastRect.w + (int32)DEFAULT_H_SPACING * skin.labelFontSize * 0.5f > selectionRect.w)
          selectionRect.w = lastRect.w + (int32)DEFAULT_H_SPACING * skin.labelFontSize * 0.5f;
      }
      selectionRect.y += vSpacing + controlHeight;
    }

    const float totalMenuHeight = ((optionCount * controlHeight) + (optionCount * vSpacing));

    //
    // Draw the background behind the labels
    //
    selectionRect.y = (float) y; // Reset y so we start drawing the selection from the top again
    Rectf totalRect(selectionRect.x, selectionRect.y, selectionRect.w, totalMenuHeight);
    Renderer::pushSprite(streamBuffer,
        Vector3(selectionRect.x / screenW, selectionRect.y / screenH, z + 0.01f),
        Vector2(selectionRect.w / screenW, totalMenuHeight / screenH),
        Rectf(), skin.color[GUISkin::MENU]);

    for (uint32 i = 0; i < optionCount; i++)
    { 
      const char* strLabel = options[i];
      bool isSubmenuParent  =  (strLabel[0] == '>');
      bool isSeparator      =  (strLabel[0] == '0');

      if (isSeparator)
      {
        Renderer::pushSprite(streamBuffer,
            Vector3((selectionRect.x + (int32) DEFAULT_H_SPACING) / screenW , (selectionRect.y + halfControlHeight) / screenH, z),
            Vector2((selectionRect.w - (int32) DEFAULT_H_SPACING * 2) / screenW, 2 / screenH),
            Rectf(), skin.color[GUISkin::SEPARATOR]);
      }
      else if (selectionRect.containsPoint(mousePos) && z < currentCursorZ)
      {
        currentCursorZ = z;
        hoverControlId = id;
        mouseOver = true;
        selectedOption = (POPUP_HOVER | i);

        if (isSubmenuParent)
        {
          strLabel++;
        }

        // Selection background
        Renderer::pushSprite(streamBuffer,
            Vector3(selectionRect.x / screenW , selectionRect.y / screenH, z),
            Vector2(selectionRect.w / screenW, (controlHeight) / screenH),
            Rectf(), skin.color[GUISkin::MENU_SELECTION]);

        // redraw the label over the selection
        label(id, strLabel, (int32) selectionRect.x + DEFAULT_H_SPACING - areaOffset.x,
            (int32) (selectionRect.y - areaOffset.y + halfControlHeight - (skin.labelFontSize / 2.0f)), 0,
            NONE, skin.color[GUISkin::MENU_SELECTION]);

        if (mouseLButtonDownThisFrame() || (isActiveControl && mouseLButtonIsDown()))
        {
          activeControlId = id;
        }
        else if(mouseLButtonUpThisFrame() && isActiveControl)
        {
          activeControlId = 0;
          selectedOption = i;
          changed = true;
        }
      }

      if (isSubmenuParent)
      {
        Renderer::pushSprite(streamBuffer,
            Vector3((selectionRect.x + selectionRect.w - chevronSize - (float) DEFAULT_H_SPACING) / screenW,
              (y + (controlHeight / 2) - (chevronSize / 2)) / screenH,
              z), 
            Vector2((int32) chevronSize / screenW, (int32) (float) chevronSize / screenH),
            skin.spritePopupMenuChevron, Color::WHITE);
      }

      selectionRect.y += controlHeight + vSpacing;
    }

    //
    // Highlight default if nothing was selected
    //
    if (selectedOption == POPUP_MENU_IDLE && defaultSelection >= 0 && defaultSelection < optionCount && hoverControlId != id)
    {
      selectionRect.y = (float) y;
      const char* strLabel = options[defaultSelection];
      bool isHoverOption = (strLabel[0] == '>');
      if (isHoverOption)
        strLabel++;

      Renderer::pushSprite(streamBuffer,
          Vector3(selectionRect.x / screenW , selectionRect.y / screenH, z),
          Vector2(selectionRect.w / screenW, (controlHeight) / screenH),
          Rectf(), skin.color[GUISkin::MENU_SELECTION]);

      // DE the label over the selection
      label(id, strLabel, (int32) selectionRect.x + DEFAULT_H_SPACING - areaOffset.x,
          (int32) (selectionRect.y - areaOffset.y + halfControlHeight - (skin.labelFontSize / 2.0f)), 0,
          NONE, skin.color[GUISkin::MENU_SELECTION]);

      if (isHoverOption)
      {
        // RIGHT SIDE "chevron icon"
        Renderer::pushSprite(streamBuffer,
            Vector3((selectionRect.x + selectionRect.w - chevronSize - (float) DEFAULT_H_SPACING) / screenW,
              (y + (controlHeight / 2) - (chevronSize / 2)) / screenH,
              z), 
            Vector2((int32) chevronSize / screenW, (int32) (float) chevronSize / screenH),
            skin.spritePopupMenuChevron, Color::WHITE);
      }
    }


    if (mouseLButtonDownThisFrame() && !mouseOver)
    {
      activeControlId = 0;
      hoverControlId = 0;
      selectedOption = POPUP_MENU_DMISMISS;
    }

    lastRect = Rect((int32) totalRect.x, (int32) totalRect.y, (int32) totalRect.w, (int32) totalRect.h);

    // We resotre the previous global Z
    z = oldZ;
    return selectedOption;
  }

  int32 GUI::comboBox(GUIControlID  id, const char** options, uint32 optionCount, int32 selectedIndex, uint32 x, uint32 y, uint32 w)
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
    float chevronSize = ((float)DEFAULT_CONTROL_HEIGHT * 0.5f);
    Renderer::pushSprite(streamBuffer,
        Vector3((x + w - chevronSize - (float) DEFAULT_H_SPACING) / screenW,
          (y + ((float) h / 2 ) - (chevronSize / 2)) / screenH, z), 
        Vector2((int32) chevronSize / screenW, (int32) (float) chevronSize / screenH),
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
      newSelectedIndex = popupMenu(id, options, optionCount, x, y + h, w);
    }
    else if (mouseOver)
    {
      hoverControlId = id;
      if (mouseLButtonDownThisFrame() && !isActiveControl)
      {
        activeControlId = id;

        if (topmostWindowId)
          topmostWindowId = currentWindowId;
      }
    }

    if (newSelectedIndex >= 0 && newSelectedIndex < (int32) optionCount)
      selectedIndex = newSelectedIndex;

    return selectedIndex;
  }

  void GUI::end()
  {
    Renderer::end(streamBuffer);
  }

  //
  // Internal and utility functions
  //
  
  void GUI::beginTextInput(char* buffer, size_t size)
  {
    // We cant guarantee the order the GUI will call begin/end so we might be
    // able to end previous input session here befor starting another one.
    if (input.isEnabled())
      endTextInput();

    input.enable();
    eventHandler = EventManager::get().addHandler(onEventForwarder, Event::TEXT | Event::KEYBOARD, this);
    input.setBuffer(buffer, size);
    cursorXPosition = 0.0f;
    selectionXPosition = 0.0f;
  }

  void GUI::endTextInput()
  {
    SMOL_ASSERT(input.isEnabled(), "EndTextInput() called when no text input is not enabled.");
    input.disable();
    EventManager::get().removeHandler(eventHandler);
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

    input.disable();

    const Color windowBackground        = Color(29, 29, 29);
    const Color panelBackground         = Color(77, 77, 77);
    const Color controlBackground       = Color(15, 15, 15);
    const Color controlSurface          = Color(40, 40, 40);
    const Color controlBackgroundHover = Color(20, 20, 20);;
    const Color controlSurfaceHover     = Color(50, 50, 50);
    const Color highlight               = Color(0, 10, 250);
    const Color contrastLight           = Color(150, 150, 150);

    skin.color[GUISkin::TEXT]                   = Color::WHITE;
    skin.color[GUISkin::TEXT_DEBUG_BACKGROUND]  = Color::BLACK;
    skin.color[GUISkin::TEXT_DISABLED]          = Color::GRAY;
    skin.color[GUISkin::TEXT_SELECTION]         = highlight;

    skin.color[GUISkin::TEXT_INPUT]         = controlBackground;
    skin.color[GUISkin::TEXT_INPUT_HOVER]   = controlBackgroundHover;
    skin.color[GUISkin::TEXT_INPUT_ACTIVE]  = controlBackground;

    skin.color[GUISkin::CURSOR]             = Color::WHITE;
    skin.color[GUISkin::CURSOR_HOT]         = Color(255, 165, 0);

    skin.color[GUISkin::BUTTON]        = controlSurface;
    skin.color[GUISkin::BUTTON_HOVER]  = controlSurfaceHover;
    skin.color[GUISkin::BUTTON_ACTIVE] = controlBackground;

    skin.color[GUISkin::TOGGLE_BUTTON]                = controlSurface;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER]          = controlSurfaceHover;
    skin.color[GUISkin::TOGGLE_BUTTON_HOVER_ACTIVE]   = highlight;
    skin.color[GUISkin::TOGGLE_BUTTON_ACTIVE]         = highlight;

    skin.color[GUISkin::COMBO_BOX]              = controlBackground;
    skin.color[GUISkin::COMBO_BOX_HOVER]         = controlSurface;

    skin.color[GUISkin::CHECKBOX]               = controlBackground;
    skin.color[GUISkin::CHECKBOX_HOVER]         = controlSurface;
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
