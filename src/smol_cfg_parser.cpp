#include <smol/smol_cfg_parser.h>
#include <smol/smol_arena.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>
#include <ctype.h>
#include <string.h>

namespace smol
{
  struct Lexer
  {
    const char* data;
    const char* eof;
    int line;
    int column;

    Lexer(const char* buffer, size_t size):
      data(buffer), eof(data + size), line(1), column(1)
    {
    }

    inline bool isEOF()
    {
      return data >= eof;
    }

    char getc()
    {
      if (data >= eof) return (char) -1;
      char c = *data++;
      ++column;
      if (c == '\n')
      {
        ++line;
        column = 0;
      }
      return c;
    }

    char peek()
    {
      if (data >= eof) return (char) -1;
      return *data;
    }

    inline bool isWhiteSpace(char c)
    {
      return (c == ' ' || c == '\t' || c == '\r');
    }

    void skipToEndOfLine()
    {
      while (*data != '\n' && !isEOF())
      {
        ++data;
        ++column;
      }
    }

    void skipWhiteSpaceAndLineBreak()
    {
      while (data < eof && (isWhiteSpace(*data) || *data == '\n'))
      {
        if (*data == '\n')
        {
          ++line;
          column = 0;
        }
        else
        {
          ++column;
        }
        ++data;
      }
      // Skip comment
      if (*data == '#')
      {
        skipToEndOfLine();
        skipWhiteSpaceAndLineBreak();
      }
    }

    void skipWhiteSpace()
    {
      while (data < eof && isWhiteSpace(*data))
      {
        ++data;
        ++column;
      }

      // Skip comment
      if (*data == '#')
      {
        skipToEndOfLine();
        skipWhiteSpace();
      }
    }
  };

  struct Token
  {
    enum Type
    {
      NUMBER,
      IDENTIFIER,
      STRING,
      COMMA,
      QUOTE,
      BACKSLASH,
      LINEBREAK,
      COLON,
      END_OF_FILE,
      VECTOR_START,
      VECTOR_END,
      INVALID
    };

    Type type;
    size_t size;
    const char* data;
  };

  float substringToFloat(const char* strNumber, size_t size)
  {
    const char maxNumberLiteralLen = 64;
    char temp[maxNumberLiteralLen];
    size_t numberLiteralLen = size >= maxNumberLiteralLen ? maxNumberLiteralLen : size;
    memcpy(temp, strNumber, numberLiteralLen);
    temp[size] = 0;
    return (float) atof(temp);
  }

  static Token getToken(Lexer& lexer)
  {
    lexer.skipWhiteSpace();
    const char* tokenStart = lexer.data;

    char c = lexer.getc();

    Token token = {};
    token.type = Token::INVALID;
    token.data = tokenStart;

    if (isdigit(c) || c == '.' || c == '-' || c == '+') // numbers can start with a dot "."
    {
      int dotCount = 0;
      do
      {
        c = lexer.peek();
        if (c == '.') 
          ++dotCount;
      }while ((isdigit(c) || c == '.') && dotCount <= 1 && lexer.getc());

      token.type = Token::NUMBER;
      token.size = lexer.data - token.data;
    }
    else if (isalpha(c) || c == '_')
    {
      do
      {
        c = lexer.peek();
      }while ((isalpha(c) || isdigit(c) || c == '_' || c == '-' ) && lexer.getc());

      token.type = Token::IDENTIFIER;
      token.size = lexer.data - token.data;
    }
    else if (c == '"')
    {
      token.data++;
      do
      {
        c = lexer.peek();
      }while ( c != '"' && !lexer.isEOF() && lexer.getc());

      if (lexer.isEOF())
      {
        smol::Log::error("Unexpected EOF while parsing string at %d, %d", lexer.line, lexer.column);
      }

      if (c != '"')
      {
        smol::Log::error("Unexpected character '%x' while parsing string at %d, %d", c, lexer.line, lexer.column);
        return token;
      }

      token.type = Token::STRING;
      token.size = lexer.data - token.data;
      lexer.getc(); // to consume the closing quote
    }
    else if (c == ',')
    {
      token.type = Token::COMMA;
      token.size = 1;
    }
    else if (c == ':')
    {
      token.type = Token::COLON;
      token.size = 1;
    }
    else if (c == '\\')
    {
      token.type = Token::BACKSLASH;
      token.size = 1;
    }
    else if (c == '\n')
    {
      token.type = Token::LINEBREAK;
      token.size = 1;
    }
    else if (c == '{')
    {
      token.type = Token::VECTOR_START;
      token.size = 1;
    }
    else if (c == '}')
    {
      token.type = Token::VECTOR_END;
      token.size = 1;
    }
    else if (lexer.isEOF())
    {
      token.type = Token::END_OF_FILE;
      token.size = 1;
    }

    return token;
  }

  static bool requireToken(Lexer& lexer, Token::Type type, Token& token)
  {
    token = getToken(lexer);
    if (token.type != type)
      return false;

    return true;
  }

  static void unexpectedTokenError(Token::Type type, Lexer& lexer)
  {
    char* name;

    switch(type)
    {
      case Token::NUMBER:
        name = "NUMBER";
        break;
      case Token::IDENTIFIER:
        name = "IDENTIFIER";
        break;
      case Token::STRING:
        name = "STRING";
        break;
      case Token::COMMA:
        name = "COMMA";
        break;
      case Token::QUOTE:
        name = "QUOTE";
        break;
      case Token::BACKSLASH:
        name = "BACKSLASH";
        break;
      case Token::LINEBREAK:
        name = "LINEBREAK";
        break;
      case Token::COLON:
        name = "COLON";
        break;
      case Token::END_OF_FILE:
        name = "END_OF_FILE";
        break;
      case Token::VECTOR_START:
        name = "VECTOR_START";
        break;
      case Token::VECTOR_END:
        name = "VECTOR_END";
        break;
      case Token::INVALID:
      default:
        name = "character";
        break;
    }

    smol::Log::error("Unexpected %s at line: %d, %d", name, lexer.line, lexer.column);
  }

  static bool parseEntry(Lexer& lexer, Arena& arena, ConfigEntry** out)
  {
    smol::ConfigVariable var = {};
    bool done = false;
    int variableCount = 0;

    lexer.skipWhiteSpaceAndLineBreak();

    while(!done)
    {
      Token tIdentifier;
      Token t;
      Token rValue;

      // VARNAME
      if (!requireToken(lexer, Token::Type::IDENTIFIER, tIdentifier))
      {
        unexpectedTokenError(tIdentifier.type, lexer);
        return false;
      }

      // VARNAME:
      if (!requireToken(lexer, Token::Type::COLON, t))
      {
        unexpectedTokenError(t.type, lexer);
        return false;
      }

      // null terminate variable name on the original source file buffer
      // Notice we already got the ":" after the variable name, so it's safe
      // to write over that part of the buffer.
      *((char*)tIdentifier.data + tIdentifier.size) = 0; 
      var.name = tIdentifier.data;

      rValue = getToken(lexer);

      // VARNAME: ?
      switch (rValue.type)
      {
        case Token::Type::NUMBER:
          {
            var.numberValue = substringToFloat(rValue.data, rValue.size);
            var.type = ConfigVariable::NUMBER;
          }
          break;

        case Token::Type::STRING:
          {
            // null terminate the string name by changing the original source code buffer.
            *((char*)rValue.data + rValue.size) = 0;
            var.stringValue = rValue.data;
            var.type = ConfigVariable::STRING;
          }
          break;

        case Token::Type::VECTOR_START:
          {
            //This can be a vec2, vec3 or vec4
            const int maxVectorSize = 4;
            int numElements = 0;

            do
            {
              lexer.skipWhiteSpaceAndLineBreak();

              // Require a number
              if (!requireToken(lexer, Token::Type::NUMBER, rValue))
              {
                unexpectedTokenError(rValue.type, lexer);
                return false;
              }
              // the values are declared into a Union, so I'm just pointing to the
              // largest member in order to store values sequentially while I
              // don't know how many elements are in the vectore we are parsing
              // at the moment
              float* value = (float*) &var.vec4Value;
              value[numElements++] = substringToFloat(rValue.data, rValue.size);

              lexer.skipWhiteSpaceAndLineBreak();
              t = getToken(lexer);

            }while(t.type == Token::COMMA && numElements <= maxVectorSize);

            if (t.type != Token::VECTOR_END)
            {
              unexpectedTokenError(rValue.type, lexer);
              return false;
            }

            if (numElements == 1)
              var.type = ConfigVariable::NUMBER;
            else if (numElements == 2)
              var.type = ConfigVariable::VECTOR2;
            else if (numElements == 3)
              var.type = ConfigVariable::VECTOR3;
            else if (numElements == 4)
              var.type = ConfigVariable::VECTOR4;
            else
              SMOL_ASSERT(numElements >=1 && numElements <= 4, "Vector elements should be 1, 2, 3 or 4");

          }
          break;

        default:
          unexpectedTokenError(rValue.type, lexer);
          return false;
      }

      t = getToken(lexer);

      // End of entry
      if (t.type == Token::Type::LINEBREAK || t.type == Token::Type::END_OF_FILE)
      {
        done = true;
      }
      else if (t.type != Token::Type::COMMA)
      {
        // If not a line break, it must have yet anoter variable.
        // Otherwise it's a syntax error.
        unexpectedTokenError(t.type, lexer);
        return false;
      }

      ConfigVariable* variable = (ConfigVariable*) arena.pushSize(sizeof(ConfigVariable));
      *variable = var;
      ++variableCount;

      lexer.skipWhiteSpaceAndLineBreak();
    }

    // Alocate the entry
    ConfigEntry* entry = (ConfigEntry*) arena.pushSize(sizeof(ConfigEntry));
    entry->variableCount = variableCount;
    entry->variables = ((ConfigVariable*) entry) - variableCount;   // walk back N variables to find the first variable for this entry
    *out = entry;

    return true;
  }

  Config:: Config(size_t initialArenaSize):
    arena(initialArenaSize), entryCount(0), buffer(nullptr), entries(nullptr) { }

  Config::Config(const char* path, size_t initialArenaSize):
    arena(initialArenaSize), entryCount(0), buffer(nullptr), entries(nullptr)
  {
    load(path);
  }

  bool Config::load(const char* path)
  {
    size_t bufferSize;
    buffer = Platform::loadFileToBuffer(path, &bufferSize);
    Lexer lexer(buffer, bufferSize);
    bool hasErrors = false;
    ConfigEntry* previousEntry;

    while (!lexer.isEOF() && !hasErrors)
    {
      ConfigEntry* out;
      if (!parseEntry(lexer, arena, &out))
      {
        hasErrors = true;
        continue;
      }

      // Is it the first entry ?
      if (entryCount == 0)
      {
        entries = out; 
        previousEntry = out;
      }
      else
      {
        previousEntry->next = out;
        previousEntry = out;
      }

      ++entryCount;
    }

    if(hasErrors)
    {
      arena.reset();
      Platform::unloadFileBuffer(buffer);
      entries = nullptr;
      buffer = nullptr;
      entryCount = 0;
      return false;
    }

    return true;
  }
};
