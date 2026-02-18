#ifndef WIFI_BACKUP_H_
#define WIFI_BACKUP_H_

#include <WiFi.h>
#include <WebServer.h>

/* WiFi AP Configuration */
#define WIFI_SSID           "ESPEED32"
#define WIFI_PASS           "espeed32"
#define WIFI_AP_CHANNEL     1
#define WIFI_MAX_CONNECTIONS 1

/* Function prototype - called from settings menu */
void showWiFiBackupScreen();

#endif
