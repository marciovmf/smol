#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"
// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif


#ifdef SMOL_DEBUG
#include <stdio.h>
	#define LogMsg(prefix, msg, ...) do {printf("\n%s - ", prefix); printf(msg, __VA_ARGS__);} while(0)
	#define LogMsgAndFileLine(prefix, msg, ...) do {printf("\n%s - ", prefix); printf(msg, __VA_ARGS__); printf("@ %s:%d", __FILE__, __LINE__); } while(0)
	#define LogInfo(msg, ...) LogMsg("[INFO]", msg, __VA_ARGS__)
	#define LogWarning(msg, ...) LogMsgAndFileLine("[WARNING]", msg, __VA_ARGS__)
	#define LogError(msg, ...) LogMsgAndFileLine("[ERROR]", msg, __VA_ARGS__)
	#define LDK_ASSERT(condition, msg, ...) do{if (!(condition)) { LogMsgAndFileLine("[Assertion Failed]", msg, __VA_ARGS__); *((int*)0) = 0;} } while(0)
#else
	#define LogMsg(prefix, msg, ...)
	#define LogMsgAndFileLine(prefix, msg, ...)
	#define LogInfo(msg, ...) 
	#define LogWarning(msg, ...) 
	#define LogError(msg, ...) 
	#define LDK_ASSERT(condition, msg, ...)
#endif//SMOL_DEBUG


#endif //SMOL_H
