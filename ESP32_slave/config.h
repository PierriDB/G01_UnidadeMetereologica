/************************ Adafruit IO Config *******************************/

#define IO_USERNAME "PierriDB"
#define IO_KEY "aio_YWMn80GEuLUmw8Bpnw6zKebcSz53"

/******************************* WIFI **************************************/

#define WIFI_SSID "APTO22_5G-EXT"
#define WIFI_PASS "999953781"
//#define WIFI_SSID "DELLABRUNA"
//#define WIFI_PASS "dellabruna2022"

#include "AdafruitIO_WiFi.h"

#if defined(USE_AIRLIFT) || defined(ADAFRUIT_METRO_M4_AIRLIFT_LITE) ||         \
    defined(ADAFRUIT_PYPORTAL)

#if !defined(SPIWIFI_SS) // if the wifi definition isnt in the board variant

#define SPIWIFI SPI
#define SPIWIFI_SS 10 // Chip select pin
#define NINA_ACK 9    // a.k.a BUSY or READY pin
#define NINA_RESETN 6 // Reset pin
#define NINA_GPIO0 -1 // Not connected
#endif
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS, SPIWIFI_SS,
                   NINA_ACK, NINA_RESETN, NINA_GPIO0, &SPIWIFI);
#else
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
#endif
