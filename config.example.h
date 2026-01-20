// ================= config.example.h =================
// Copy this file to config.h and fill in your own values.
// IMPORTANT: never commit config.h with your sensitive data.
//
// Example for the esp8266_temperature project.

#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials for WiFiManager hotspot
#define WIFI_SSID     "YourHotspotName"
#define WIFI_PASSWORD "YourSecurePassword"  // at least 8 characters

// ntfy.sh Push service topic
#define NTFY_TOPIC    "your-ntfy-topic-here"

#endif // CONFIG_H
