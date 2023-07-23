#include <smol/smol_text_input.h>
#include <smol/smol_log.h>
#include <string.h>
#include <vcruntime_string.h>

namespace smol
{
  void moveCharactersForward(char *buff, int32 startIndex, int32 length)
  {
    for (int i = startIndex + length; i > startIndex; i--)
    {
      buff[i] = buff[i-1];
    }
  }

  void moveCharactersBackward(char *buff, int32 startIndex, int32 length)
  {
    memcpy(buff + startIndex - 1, buff + startIndex, length);
  }

  void TextInput::setBuffer(char* buffer, size_t size)
  {
    this->buffer        = buffer;
    bufferCapacity = (int32) size;
    bufferUsed     = (int32) strlen(buffer);
    cursorIndex         = (int32) bufferUsed;
    SMOL_ASSERT(bufferCapacity > bufferUsed, "Input buffer contents are larger than the buffer size. Did you forget to initialized the buffer ?");
    memset(buffer + bufferUsed, 0, size - bufferUsed);
  }

  inline bool TextInput::isEnabled()
  {
    return enabled;
  }

  inline void TextInput::enable()
  {
    enabled = true;
  }

  inline void TextInput::disable()
  {
    enabled = false;
  }

  inline int32 TextInput::getCursorIndex()
  {
    return cursorIndex;
  }

  inline void TextInput::setCursorIndex(int32 index)
  {
    if (index > 0 && index < bufferUsed)
      cursorIndex = index;
  }

  inline void TextInput::moveCursorLeft()
  {
    if (cursorIndex > 0)
    {
      cursorIndex--;
    }
  }

  inline void TextInput::moveCursorRight()
  {
    if (cursorIndex < bufferUsed) 
    {
      cursorIndex++;
    }
  }

  void TextInput::moveCursorHome()
  {
    cursorIndex = 0;
  }

  void TextInput::moveCursorEnd()
  {
    cursorIndex = bufferUsed;
  }

  void TextInput::addCharacterAtCursor(char c)
  {
    if (bufferUsed >= (bufferCapacity - 1))
      return;

    if (cursorIndex == 0)
    {
      strncpy(buffer + 1, buffer, bufferUsed);
      buffer[cursorIndex++] = c;
      bufferUsed++;
    }
    else if (cursorIndex == bufferUsed)
    {
      buffer[cursorIndex++] = c;
      bufferUsed++;
    }
    else
    {
      // middle of the buffer
      moveCharactersForward(buffer, cursorIndex, bufferUsed - cursorIndex);
      buffer[cursorIndex++] = c;
      bufferUsed++;
    }
  }

  void TextInput::deleteCharacterBeforeCursor()
  {
    if (bufferUsed > 0 && cursorIndex > 0)
    {
      if (cursorIndex < bufferUsed)
      {
        moveCharactersBackward(buffer, cursorIndex, bufferUsed - cursorIndex);
        buffer[--bufferUsed] = 0;
      }
      else if (cursorIndex == bufferUsed)
        buffer[--bufferUsed] = 0;

      cursorIndex--;
    }
  }

  void TextInput::deleteCharacterAfterCursor()
  {
    if (cursorIndex+1 <= bufferUsed) // when cursorIndex == bufferUsed, it's past the last character which is a valid cursor position!
    {
      cursorIndex++;
      deleteCharacterBeforeCursor();
    }
  }
}
