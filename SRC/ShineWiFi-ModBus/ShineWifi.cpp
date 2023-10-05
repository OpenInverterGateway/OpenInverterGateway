#include "ShineWifi.h"

bool ShineWifiDisconnect() {
#ifdef ESP8266
  if ((WiFi.getMode() & WIFI_STA) != 0) {
    bool ret;
#ifdef WM_DEBUG_LEVEL
    Log.print(F("WiFi station disconnect"));
#endif
    ETS_UART_INTR_DISABLE();  // @todo possibly not needed
    ret = wifi_station_disconnect();
    WiFi.mode(WIFI_OFF);
    ETS_UART_INTR_ENABLE();
    return ret;
  }
#elif defined(ESP32)
#ifdef WM_DEBUG_LEVEL
  Log.print("WiFi station disconnect");
#endif
  WiFi.mode(WIFI_OFF);
  return WiFi.disconnect();  // not persistent atm
#endif
  return false;
}