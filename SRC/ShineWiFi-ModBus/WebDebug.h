#pragma once

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
