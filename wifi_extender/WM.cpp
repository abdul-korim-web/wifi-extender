/*
 * WM.cpp – WiFi Manager / Web Server implementation
 * Product : Abdul WiFi Extender
 * Developer: Abdul Korim
 * GitHub   : https://github.com/abdul-korim-web
 *
 * Based on original work by Pius Onyema Ndukwu
 * https://github.com/Pius171/esp8266-wifi-extender
 *
 * License  : MIT
 *
 * FIX: All POST handlers now properly accumulate the full request body
 *      across multiple TCP chunks before parsing JSON.
 *      ESPAsyncWebServer delivers the body in chunks via (data, len, index, total).
 *      We use request->_tempObject as a heap String* to buffer chunks,
 *      and only parse+respond when (index + len == total).
 */

#include "WM.h"

// ── Global server instance ─────────────────────────────────────────────────
AsyncWebServer server(80);

// ═══════════════════════════════════════════════════════════════════════════
//  Body accumulation helper
//  Usage in every POST body callback:
//    if (!accumBody(request, data, len, index, total)) return;
//    String body = *(String*)request->_tempObject;
//    freeBody(request);
//    // now parse body
// ═══════════════════════════════════════════════════════════════════════════

// Returns true only when the complete body has been received.
static bool accumBody(AsyncWebServerRequest* req,
                      uint8_t* data, size_t len,
                      size_t index, size_t total) {
  if (index == 0) {
    // First chunk – allocate a new String on the heap
    if (req->_tempObject) {
      delete (String*)req->_tempObject;
    }
    req->_tempObject = new String();
    ((String*)req->_tempObject)->reserve(total);
  }
  // Append this chunk
  ((String*)req->_tempObject)->concat((const char*)data, len);
  // Return true only when we have everything
  return (index + len >= total);
}

// Free the temp body buffer
static void freeBody(AsyncWebServerRequest* req) {
  if (req->_tempObject) {
    delete (String*)req->_tempObject;
    req->_tempObject = nullptr;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════

void WM::send_json_error(AsyncWebServerRequest* r, int code, const String& msg) {
  String body = "{\"ok\":false,\"error\":\"" + msg + "\"}";
  r->send(code, "application/json", body);
}

String WM::generate_token() {
  const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String token = "";
  randomSeed(ESP.getCycleCount() ^ millis());
  for (int i = 0; i < SESSION_TOKEN_LEN; i++) {
    token += chars[random(0, sizeof(chars) - 1)];
  }
  return token;
}

bool WM::session_valid(const String& token) {
  if (token.length() == 0 || _session_token.length() == 0) return false;
  if (token != _session_token) return false;
  if (millis() - _session_time > SESSION_TIMEOUT_MS) {
    _session_token = "";
    return false;
  }
  return true;
}

void WM::refresh_session(const String& token) {
  if (token == _session_token) _session_time = millis();
}

bool WM::is_valid_session(AsyncWebServerRequest* request) {
  String token = "";
  if (request->hasHeader("Authorization")) {
    String auth = request->getHeader("Authorization")->value();
    if (auth.startsWith("Bearer ")) token = auth.substring(7);
  }
  if (token.length() == 0 && request->hasParam("token")) {
    token = request->getParam("token")->value();
  }
  if (token.length() == 0 && request->hasHeader("Cookie")) {
    String cookie = request->getHeader("Cookie")->value();
    int idx = cookie.indexOf("session=");
    if (idx >= 0) {
      int end = cookie.indexOf(';', idx);
      token = (end < 0) ? cookie.substring(idx + 8)
                        : cookie.substring(idx + 8, end);
    }
  }
  bool valid = session_valid(token);
  if (valid) refresh_session(token);
  return valid;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Auth helpers
// ═══════════════════════════════════════════════════════════════════════════

bool WM::check_auth(const String& user, const String& pass) {
  File f = LittleFS.open(CFG_AUTH_PATH, "r");
  if (!f) {
    return (user == "admin" && pass == "admin");
  }
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  String stored_user = doc["user"] | "admin";
  String stored_pass = doc["pass"] | "admin";
  return (user == stored_user && pass == stored_pass);
}

bool WM::save_auth(const String& user, const String& pass) {
  StaticJsonDocument<256> doc;
  doc["user"] = user;
  doc["pass"] = pass;
  File f = LittleFS.open(CFG_AUTH_PATH, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

String WM::get_auth_user() {
  File f = LittleFS.open(CFG_AUTH_PATH, "r");
  if (!f) return "admin";
  StaticJsonDocument<256> doc;
  deserializeJson(doc, f);
  f.close();
  return doc["user"] | "admin";
}

// ═══════════════════════════════════════════════════════════════════════════
//  Credential helpers
// ═══════════════════════════════════════════════════════════════════════════

String WM::get_credentials(int a) {
  // a: 0=ssid  1=pass  2=ap  3=appass
  File file = LittleFS.open(CFG_WIFI_PATH, "r");
  if (!file) {
    Serial.println(F("config.json not found - first run"));
    return "null";
  }
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return "null";
  const char* fields[4] = {"ssid", "pass", "ap", "appass"};
  if (a < 0 || a > 3) return "null";
  String val = doc[fields[a]] | "null";
  return val;
}

bool WM::save_credentials(const String& ssid, const String& pass,
                           const String& ap,   const String& appass) {
  StaticJsonDocument<512> doc;
  doc["ssid"]   = ssid;
  doc["pass"]   = pass;
  doc["ap"]     = ap;
  doc["appass"] = appass;
  File f = LittleFS.open(CFG_WIFI_PATH, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Filesystem helper
// ═══════════════════════════════════════════════════════════════════════════

void WM::listDir(const char* dirname) {
  Serial.printf("Listing: %s\n", dirname);
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    File file = root.openFile("r");
    Serial.printf("  FILE: %s  SIZE: %u\n", root.fileName().c_str(), file.size());
    file.close();
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Server lifecycle
// ═══════════════════════════════════════════════════════════════════════════

void WM::begin_server() { server.begin(); }

void WM::create_server() {
  register_auth_routes();
  register_api_routes();
  register_static_routes();
  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect("/");
  });
}

// ═══════════════════════════════════════════════════════════════════════════
//  Static routes
// ═══════════════════════════════════════════════════════════════════════════

void WM::register_static_routes() {
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

// ═══════════════════════════════════════════════════════════════════════════
//  Auth routes
// ═══════════════════════════════════════════════════════════════════════════

void WM::register_auth_routes() {

  // ── POST /api/login ───────────────────────────────────────────────────────
  // Body: { "user": "admin", "pass": "admin" }
  server.on("/api/login", HTTP_POST,
    [](AsyncWebServerRequest* request) {},  // request handler (empty – body not ready yet)
    nullptr,                                 // upload handler
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {

      // ── Accumulate chunks; return if body not complete yet ────────────────
      if (!accumBody(request, data, len, index, total)) return;

      String bodyStr = *(String*)request->_tempObject;
      freeBody(request);

      // ── Parse JSON ────────────────────────────────────────────────────────
      StaticJsonDocument<256> body;
      DeserializationError err = deserializeJson(body, bodyStr);
      if (err) {
        Serial.printf("[login] JSON parse error: %s | raw: %s\n", err.c_str(), bodyStr.c_str());
        send_json_error(request, 400, "Invalid JSON");
        return;
      }

      String user = body["user"] | "";
      String pass = body["pass"] | "";

      if (!check_auth(user, pass)) {
        send_json_error(request, 401, "Invalid credentials");
        return;
      }

      _session_token = generate_token();
      _session_time  = millis();

      String resp = "{\"ok\":true,\"token\":\"" + _session_token + "\"}";
      AsyncWebServerResponse* res = request->beginResponse(200, "application/json", resp);
      res->addHeader("Set-Cookie",
        "session=" + _session_token + "; Path=/; HttpOnly; SameSite=Strict");
      request->send(res);
    }
  );

  // ── POST /api/logout ──────────────────────────────────────────────────────
  // No body needed
  server.on("/api/logout", HTTP_POST, [this](AsyncWebServerRequest* request) {
    _session_token = "";
    AsyncWebServerResponse* res = request->beginResponse(200, "application/json", "{\"ok\":true}");
    res->addHeader("Set-Cookie", "session=; Path=/; Max-Age=0");
    request->send(res);
  });

  // ── POST /api/change-password ─────────────────────────────────────────────
  // Body: { "current": "oldpass", "newpass": "newpass" }
  server.on("/api/change-password", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {

      if (!accumBody(request, data, len, index, total)) return;

      // Auth check AFTER body is complete
      if (!is_valid_session(request)) {
        freeBody(request);
        send_json_error(request, 401, "Unauthorized");
        return;
      }

      String bodyStr = *(String*)request->_tempObject;
      freeBody(request);

      StaticJsonDocument<256> body;
      if (deserializeJson(body, bodyStr)) {
        send_json_error(request, 400, "Bad JSON");
        return;
      }

      String cur    = body["current"] | "";
      String newp   = body["newpass"] | "";
      String curuser = get_auth_user();

      if (!check_auth(curuser, cur)) {
        send_json_error(request, 403, "Wrong current password");
        return;
      }
      if (newp.length() < 4) {
        send_json_error(request, 400, "Password too short (min 4)");
        return;
      }
      save_auth(curuser, newp);
      request->send(200, "application/json", "{\"ok\":true}");
    }
  );
}

// ═══════════════════════════════════════════════════════════════════════════
//  API routes
// ═══════════════════════════════════════════════════════════════════════════

void WM::register_api_routes() {

  // ── GET /api/status ───────────────────────────────────────────────────────
  server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }

    bool sta_connected = (WiFi.status() == WL_CONNECTED);
    int  rssi          = sta_connected ? WiFi.RSSI() : 0;
    int  clients       = WiFi.softAPgetStationNum();
    uint32_t heap      = ESP.getFreeHeap();
    uint32_t uptime_s  = millis() / 1000;

    char buf[768];
    snprintf(buf, sizeof(buf),
      "{"
        "\"ok\":true,"
        "\"internet\":%s,"
        "\"repeater\":%s,"
        "\"sta_ssid\":\"%s\","
        "\"ap_ssid\":\"%s\","
        "\"rssi\":%d,"
        "\"clients\":%d,"
        "\"free_heap\":%u,"
        "\"uptime_s\":%u,"
        "\"local_ip\":\"%s\","
        "\"ap_ip\":\"%s\","
        "\"mac\":\"%s\""
      "}",
      sta_connected ? "true" : "false",
      sta_connected ? "true" : "false",
      sta_connected ? WiFi.SSID().c_str() : "",
      WiFi.softAPSSID().c_str(),
      rssi, clients, heap, uptime_s,
      sta_connected ? WiFi.localIP().toString().c_str() : "",
      WiFi.softAPIP().toString().c_str(),
      WiFi.macAddress().c_str()
    );
    request->send(200, "application/json", buf);
  });

  // ── GET /api/scan ─────────────────────────────────────────────────────────
  server.on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }

    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
      request->send(200, "application/json", "{\"ok\":true,\"scanning\":true,\"networks\":[]}");
      return;
    }
    if (n == -1) {
      request->send(200, "application/json", "{\"ok\":true,\"scanning\":true,\"networks\":[]}");
      return;
    }

    String json = "{\"ok\":true,\"scanning\":false,\"networks\":[";
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      String ssid = WiFi.SSID(i);
      ssid.replace("\\", "\\\\");
      ssid.replace("\"", "\\\"");
      json += "{\"ssid\":\"" + ssid + "\",\"rssi\":";
      json += WiFi.RSSI(i);
      json += ",\"enc\":";
      json += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "0" : "1";
      json += "}";
    }
    json += "]}";
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
    request->send(200, "application/json", json);
  });

  // ── GET /api/clients ──────────────────────────────────────────────────────
  server.on("/api/clients", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }

    struct station_info* stat_info = wifi_softap_get_station_info();
    String json = "{\"ok\":true,\"clients\":[";
    bool first = true;
    while (stat_info != nullptr) {
      if (!first) json += ",";
      first = false;
      uint8_t* mac = stat_info->bssid;
      char mac_str[18];
      snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      IPAddress ip(stat_info->ip.addr);
      json += "{\"mac\":\"" + String(mac_str) + "\",\"ip\":\"" + ip.toString() + "\"}";
      stat_info = STAILQ_NEXT(stat_info, next);
    }
    wifi_softap_free_station_info();
    json += "]}";
    request->send(200, "application/json", json);
  });

  // ── GET /api/sysinfo ──────────────────────────────────────────────────────
  server.on("/api/sysinfo", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }

    FSInfo fs_info;
    LittleFS.info(fs_info);
    char buf[768];
    snprintf(buf, sizeof(buf),
      "{"
        "\"ok\":true,"
        "\"chip_id\":\"%08X\","
        "\"flash_size\":%u,"
        "\"flash_speed\":%u,"
        "\"free_heap\":%u,"
        "\"heap_fragmentation\":%u,"
        "\"max_free_block\":%u,"
        "\"cpu_freq\":%u,"
        "\"sdk_ver\":\"%s\","
        "\"core_ver\":\"%s\","
        "\"sketch_size\":%u,"
        "\"sketch_free\":%u,"
        "\"fs_total\":%u,"
        "\"fs_used\":%u,"
        "\"uptime_s\":%u"
      "}",
      ESP.getChipId(),
      ESP.getFlashChipRealSize(),
      ESP.getFlashChipSpeed(),
      ESP.getFreeHeap(),
      ESP.getHeapFragmentation(),
      ESP.getMaxFreeBlockSize(),
      ESP.getCpuFreqMHz(),
      ESP.getSdkVersion(),
      ESP.getCoreVersion().c_str(),
      ESP.getSketchSize(),
      ESP.getFreeSketchSpace(),
      (uint32_t)fs_info.totalBytes,
      (uint32_t)fs_info.usedBytes,
      (uint32_t)(millis() / 1000)
    );
    request->send(200, "application/json", buf);
  });

  // ── GET /api/wifi-config ──────────────────────────────────────────────────
  server.on("/api/wifi-config", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }
    String ssid = get_credentials(0);
    String ap   = get_credentials(2);
    if (ssid == "null") ssid = "";
    if (ap   == "null") ap   = "";
    String json = "{\"ok\":true,\"ssid\":\"" + ssid + "\",\"ap\":\"" + ap + "\"}";
    request->send(200, "application/json", json);
  });

  // ── POST /api/wifi-config ─────────────────────────────────────────────────
  // Body: { "ssid":"...", "pass":"...", "ap":"...", "appass":"..." }
  server.on("/api/wifi-config", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {

      // ── Wait for complete body ────────────────────────────────────────────
      if (!accumBody(request, data, len, index, total)) return;

      if (!is_valid_session(request)) {
        freeBody(request);
        send_json_error(request, 401, "Unauthorized");
        return;
      }

      String bodyStr = *(String*)request->_tempObject;
      freeBody(request);

      Serial.printf("[wifi-config] body: %s\n", bodyStr.c_str());

      StaticJsonDocument<512> body;
      DeserializationError err = deserializeJson(body, bodyStr);
      if (err) {
        Serial.printf("[wifi-config] JSON error: %s\n", err.c_str());
        send_json_error(request, 400, "Bad JSON");
        return;
      }

      String ssid   = body["ssid"]   | "";
      String pass   = body["pass"]   | "";
      String ap     = body["ap"]     | "";
      String appass = body["appass"] | "";

      if (ssid.length() == 0) { send_json_error(request, 400, "ssid required"); return; }
      if (ap.length()   == 0) { send_json_error(request, 400, "ap name required"); return; }

      if (save_credentials(ssid, pass, ap, appass)) {
        request->send(200, "application/json",
          "{\"ok\":true,\"message\":\"Saved! Device restarting...\"}");
        delay(500);
        ESP.restart();
      } else {
        send_json_error(request, 500, "Failed to save config");
      }
    }
  );

  // ── POST /api/restart ─────────────────────────────────────────────────────
  server.on("/api/restart", HTTP_POST, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }
    request->send(200, "application/json", "{\"ok\":true}");
    delay(300);
    ESP.restart();
  });

  // ── POST /api/reset-wifi ──────────────────────────────────────────────────
  server.on("/api/reset-wifi", HTTP_POST, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }
    LittleFS.remove(CFG_WIFI_PATH);
    request->send(200, "application/json",
      "{\"ok\":true,\"message\":\"WiFi config cleared. Restarting...\"}");
    delay(300);
    ESP.restart();
  });

  // ── POST /api/factory-reset ───────────────────────────────────────────────
  server.on("/api/factory-reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
    if (!is_valid_session(request)) { send_json_error(request, 401, "Unauthorized"); return; }
    LittleFS.format();
    request->send(200, "application/json",
      "{\"ok\":true,\"message\":\"Factory reset done. Restarting...\"}");
    delay(300);
    ESP.restart();
  });
}
