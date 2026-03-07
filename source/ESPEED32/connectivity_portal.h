#ifndef CONNECTIVITY_PORTAL_H_
#define CONNECTIVITY_PORTAL_H_

#include <WiFi.h>
#include <WebServer.h>

/* WiFi AP Configuration */
#define WIFI_SSID           "ESPEED32"
#define WIFI_PASS           "espeed32"
#define WIFI_AP_CHANNEL     1
#define WIFI_MAX_CONNECTIONS 1

/* Function prototypes - called from settings menu */
void showWiFiPortalScreen();
void showUSBPortalScreen();

#endif
