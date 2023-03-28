#ifndef _SHINE_WEB_DEBUG_H_
#define _SHINE_WEB_DEBUG_H_

#include <stdint.h>

#if ENABLE_REMOTE_DEBUG == 1
#include "RemoteDebug.h"
extern RemoteDebug Debug;
#define REMOTE_DEBUG_PRINT(s)                         \
  {                                                   \
    if (Debug.isActive(Debug.INFO)) Debug.println(s); \
  }
#else
#undef REMOTE_DEBUG_PRINT
#define REMOTE_DEBUG_PRINT(s) ;
#endif

#if ENABLE_WEB_DEBUG == 1
extern char acWebDebug[1024];
extern uint16_t u16WebMsgNo;
#define WEB_DEBUG_PRINT(s)                                              \
  {                                                                     \
    if ((strlen(acWebDebug) + strlen(s) + 50) < sizeof(acWebDebug))     \
      sprintf(acWebDebug, "%s#%i: %s\n", acWebDebug, u16WebMsgNo++, s); \
  }
#else
#undef WEB_DEBUG_PRINT
#define WEB_DEBUG_PRINT(s) ;
#endif

#if (ENABLE_WEB_DEBUG == 1) && (ENABLE_REMOTE_DEBUG == 1)
#define DEBUG_PRINT(s)     \
  {                        \
    WEB_DEBUG_PRINT(s);    \
    REMOTE_DEBUG_PRINT(s); \
  }
#elif ENABLE_WEB_DEBUG == 1
#define DEBUG_PRINT(s) \
  { WEB_DEBUG_PRINT(s); }
#elif ENABLE_REMOTE_DEBUG == 1
#define DEBUG_PRINT(s) \
  { REMOTE_DEBUG_PRINT(s); }
#else
#undef DEBUG_PRINT
#define DEBUG_PRINT(s) ;
#endif

#endif
