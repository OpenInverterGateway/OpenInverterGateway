#include "Config.h"
#include "WebDebug.h"
#include <stdint.h>

#if ENABLE_REMOTE_DEBUG == 1
RemoteDebug Debug;
#endif

#if ENABLE_WEB_DEBUG == 1
char acWebDebug[1024] = "";
uint16_t u16WebMsgNo = 0;
#endif
