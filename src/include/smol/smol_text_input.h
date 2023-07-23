#ifndef SMOL_TEXT_INPUT_H
#define SMOL_TEXT_INPUT_H

#include <smol/smol_engine.h>

namespace smol
{
  class SMOL_ENGINE_API TextInput
  {
    bool enabled;
    char* buffer;
    int32 bufferCapacity;
    int32 bufferUsed;
    int32 cursorIndex;

    public:
    void setBuffer(char* buffer, size_t size);

    bool isEnabled();
    void enable();
    void disable();

    int getCursorIndex();
    void setCursorIndex(int32 index);

    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorHome();
    void moveCursorEnd();

    void addCharacterAtCursor(char c);
    void deleteCharacterBeforeCursor();
    void deleteCharacterAfterCursor();
  };

}
#endif //SMOL_TEXT_INPUT_H

