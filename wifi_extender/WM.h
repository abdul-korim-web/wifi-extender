/*
 * WM.h – WiFi Manager / Web Server header
 * Product : Abdul WiFi Extender
 * Developer: Abdul Korim
 * GitHub   : https://github.com/abdul-korim-web
 *
 * Based on original work by Pius Onyema Ndukwu
 * https://github.com/Pius171/esp8266-wifi-extender
 *
 * License  : MIT
 */

#ifndef WM_H
#define WM_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#if (ARDUINO >= 100)
  #include "Arduino.h"
#endif

// ── Session token (simple single-session auth) ─────────────────────────────
#define SESSION_TOKEN_LEN  32
#define SESSION_TIMEOUT_MS 1800000UL  // 30 minutes

// ── Config file paths ───────────────────────────────────────────────────────
#define CFG_WIFI_PATH  "/config.json"
#define CFG_AUTH_PATH  "/auth.json"

// ── AP defaults ─────────────────────────────────────────────────────────────
#define DEFAULT_AP_SSID  "AbdulWiFi-Setup"
#define DEFAULT_AP_PASS  ""            // open during first-run setup

class WM {
 public:
  // ── Filesystem helpers ───────────────────────────────────────────────────
  void     listDir(const char* dirname);

  // ── Server lifecycle ─────────────────────────────────────────────────────
  void     create_server();     // register all route handlers
  void     begin_server();      // call server.begin()

  // ── Credential helpers ───────────────────────────────────────────────────
  String   get_credentials(int a);         // 0=ssid 1=pass 2=ap 3=appass
  bool     save_credentials(const String& ssid, const String& pass,
                             const String& ap,   const String& appass);

  // ── Auth helpers ─────────────────────────────────────────────────────────
  bool     check_auth(const String& user, const String& pass);
  bool     save_auth(const String& user, const String& pass);
  String   get_auth_user();
  bool     is_valid_session(AsyncWebServerRequest* request);
  String   generate_token();

 private:
  // ── Internal route registrars ────────────────────────────────────────────
  void     register_static_routes();
  void     register_api_routes();
  void     register_auth_routes();

  // ── Helpers ──────────────────────────────────────────────────────────────
  void     send_json_error(AsyncWebServerRequest* r, int code, const String& msg);
  bool     session_valid(const String& token);
  void     refresh_session(const String& token);

  // ── State ────────────────────────────────────────────────────────────────
  String   _session_token;
  unsigned long _session_time = 0;
};

#endif // WM_H
