#ifndef SMOL_TEXT_INPUT_H
#define SMOL_TEXT_INPUT_H

#include <smol/smol_engine.h>

namespace smol
{
  class SMOL_ENGINE_API TextInput
  {
    char* buffer;
    int32 bufferCapacity;
    int32 bufferUsed;
    int32 cursorIndex;
    int32 selectionIndex; // negative number means no selection
    bool enabled;

    bool deleteSelection();

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
    void beginSelectionAtCursor();
    int32 getSelectionStartIndex(); // returns the character index where the selection starts or -1 if no selection is active.
    void clearSelection();
  };

}
#endif //SMOL_TEXT_INPUT_H

