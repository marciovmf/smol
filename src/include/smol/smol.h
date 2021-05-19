#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"
// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif

#ifndef SMOL_RELEASE
#include <stdio.h>
	#define LogMsg(prefix, msg, ...) do {printf("\n%s - ", prefix); printf(msg, __VA_ARGS__);} while(0)
	#define LogMsgAndFileLine(prefix, msg, ...) do {printf("\n%s - ", prefix); printf(msg, __VA_ARGS__); printf("@ %s:%d", __FILE__, __LINE__); } while(0)
	#define LogInfo(msg, ...) LogMsg("[INFO]", msg, __VA_ARGS__)
	#define LogWarning(msg, ...) LogMsgAndFileLine("[WARNING]", msg, __VA_ARGS__)
	#define LogError(msg, ...) LogMsgAndFileLine("[ERROR]", msg, __VA_ARGS__)
	#define SMOL_ASSERT(condition, msg, ...) do{if (!(condition)) { LogMsgAndFileLine("[Assertion Failed]", msg, __VA_ARGS__); *((int*)0) = 0;} } while(0)
#else
  #define SMOL_EMPTY_MACRO do{} while(0)
	#define LogMsg(prefix, msg, ...) SMOL_EMPTY_MACRO
	#define LogInfo(msg, ...) SMOL_EMPTY_MACRO
  #define LogMsgAndFileLine(prefix, msg, ...) SMOL_EMPTY_MACRO
	#define LogInfo(msg, ...) SMOL_EMPTY_MACRO
	#define LogWarning(msg, ...) SMOL_EMPTY_MACRO
	#define LogError(msg, ...) SMOL_EMPTY_MACRO
	#define SMOL_ASSERT(condition, msg, ...) SMOL_EMPTY_MACRO
#endif// NOT SMOL_RELEASE


#endif //SMOL_H
