
/*
 * wifi_extender.ino – Main firmware entry point
 * Product : Abdul WiFi Extender
 * Developer: Abdul Korim
 * GitHub   : https://github.com/abdul-korim-web
 *
 * Based on original work by Pius Onyema Ndukwu
 * https://github.com/Pius171/esp8266-wifi-extender
 *
 * Hardware : ESP8266 NodeMCU ESP-12E
 * Core     : ESP8266 Arduino Core 3.0.0+
 * Libraries: ESPAsyncWebServer, ESPAsyncTCP, ArduinoJson 6.x, LittleFS
 *
 * License  : MIT
 *
 * ── LED behaviour ──────────────────────────────────────────────────────────
 *  Solid ON   → Trying to connect to router
 *  Blink 200ms → Connected + NAPT active (repeater working)
 *  Blink 1 s  → Setup AP mode (web config portal active)
 *  Solid OFF  → NAPT not supported in this build
 *
 * ── Button (GPIO0 / FLASH button) ─────────────────────────────────────────
 *  Hold ≥ 3 s → Factory reset (erases LittleFS and restarts)
 */

#include "WM.h"

// ── NAPT guard (same as original) ─────────────────────────────────────────
#if LWIP_FEATURES && !LWIP_IPV6

#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>

#define NAPT      1000
#define NAPT_PORT 10

// ── Globals ────────────────────────────────────────────────────────────────
WM my_wifi;

bool  repeaterWorking = false;
int   ledState        = HIGH;   // HIGH = off (active-low LED)
unsigned long previousMillis = 0;
long  blinkInterval   = 1000;   // ms

// Flash-button factory reset
unsigned long btnPressStart = 0;
bool          btnWasLow     = false;

// ── Forward declarations ───────────────────────────────────────────────────
void startSetupPortal();
void startRepeater(const String& ssid, const String& pass,
                   const String& ap,   const String& appass);

// ═══════════════════════════════════════════════════════════════════════════
//  setup()
// ═══════════════════════════════════════════════════════════════════════════
void setup() {
  delay(500);
  pinMode(0, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off (active low)

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("============================================"));
  Serial.println(F("   Abdul WiFi Extender — Firmware booting  "));
  Serial.println(F("   Developer : Abdul Korim                  "));
  Serial.println(F("   GitHub    : github.com/abdul-korim-web   "));
  Serial.println(F("============================================"));
  Serial.printf("Free heap on start: %u bytes\n", ESP.getFreeHeap());

  if (!LittleFS.begin()) {
    Serial.println(F("[ERROR] LittleFS mount failed – halting"));
    // Flash fast to signal FS error
    while (true) {
      digitalWrite(LED_BUILTIN, LOW);  delay(100);
      digitalWrite(LED_BUILTIN, HIGH); delay(100);
    }
  }
  Serial.println(F("[FS] LittleFS mounted OK"));
  my_wifi.listDir("/");

  // ── Read saved credentials ───────────────────────────────────────────────
  String ssid   = my_wifi.get_credentials(0);
  String pass   = my_wifi.get_credentials(1);
  String ap     = my_wifi.get_credentials(2);
  String appass = my_wifi.get_credentials(3);

  if (ssid == "null" || ssid.length() == 0) {
    Serial.println(F("[WiFi] No config found → starting Setup Portal"));
    startSetupPortal();
  } else {
    startRepeater(ssid, pass, ap, appass);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  startSetupPortal()  – AP-only mode, serves the web config UI
// ═══════════════════════════════════════════════════════════════════════════
void startSetupPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
  Serial.printf("[AP] SSID: %s  IP: %s\n",
                DEFAULT_AP_SSID,
                WiFi.softAPIP().toString().c_str());

  my_wifi.create_server();
  my_wifi.begin_server();
  Serial.println(F("[HTTP] Web server started (setup portal)"));

  blinkInterval = 1000;  // slow blink = portal active
  repeaterWorking = false;
}

// ═══════════════════════════════════════════════════════════════════════════
//  startRepeater()  – STA + AP + NAPT
// ═══════════════════════════════════════════════════════════════════════════
void startRepeater(const String& ssid, const String& pass,
                   const String& ap,   const String& appass) {
  Serial.printf("[WiFi] Connecting to: %s\n", ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (timeout >= 120) {   // 60 s timeout
      Serial.println(F("[WiFi] Timeout → falling back to Setup Portal"));
      startSetupPortal();
      return;
    }
    Serial.print('.');
    timeout++;
    digitalWrite(LED_BUILTIN, LOW);  // solid on while connecting
    delay(500);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.printf("\n[STA] IP=%s  DNS0=%s  DNS1=%s\n",
                WiFi.localIP().toString().c_str(),
                WiFi.dnsIP(0).toString().c_str(),
                WiFi.dnsIP(1).toString().c_str());

  // Pass DNS from upstream to AP-side clients
  dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
  dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));

  // Configure SoftAP with Android-compatible IP block
  WiFi.softAPConfig(
    IPAddress(172, 217, 28, 254),
    IPAddress(172, 217, 28, 254),
    IPAddress(255, 255, 255, 0));

  String apName = (ap == "null" || ap.length() == 0) ? "AbdulWiFi" : ap;
  String apPw   = (appass == "null") ? "" : appass;
  WiFi.softAP(apName, apPw);
  Serial.printf("[AP]  SSID: %s  IP: %s\n",
                WiFi.softAPSSID().c_str(),
                WiFi.softAPIP().toString().c_str());

  // ── Enable NAPT ──────────────────────────────────────────────────────────
  Serial.printf("[NAPT] Heap before init: %u\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("[NAPT] ip_napt_init ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("[NAPT] ip_napt_enable_no ret=%d\n", (int)ret);
    if (ret == ERR_OK) {
      Serial.printf("[NAPT] Routing '%s' → Internet via '%s'\n",
                    apName.c_str(), ssid.c_str());
      repeaterWorking = true;
      blinkInterval   = 200;   // fast blink = all good
    }
  }
  Serial.printf("[NAPT] Heap after init: %u\n", ESP.getFreeHeap());

  if (ret != ERR_OK) {
    Serial.println(F("[NAPT] Init failed – repeater may not forward traffic"));
    repeaterWorking = false;
    blinkInterval   = 500;
  }

  // ── Start the admin dashboard server ────────────────────────────────────
  my_wifi.create_server();
  my_wifi.begin_server();
  Serial.println(F("[HTTP] Admin dashboard server started"));
  Serial.printf("[HTTP] Open http://%s in your browser\n",
                WiFi.softAPIP().toString().c_str());
}

// ═══════════════════════════════════════════════════════════════════════════
//  loop()
// ═══════════════════════════════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  // ── Flash-button handler (GPIO0) ─────────────────────────────────────────
  if (digitalRead(0) == LOW) {
    if (!btnWasLow) {
      btnPressStart = now;
      btnWasLow     = true;
    } else if (now - btnPressStart >= 3000) {
      // Held ≥ 3 s → factory reset
      Serial.println(F("[BUTTON] 3-second hold → factory reset"));
      LittleFS.format();
      delay(200);
      ESP.restart();
    }
  } else {
    btnWasLow = false;
  }

  // ── LED blink ─────────────────────────────────────────────────────────────
  if (now - previousMillis >= (unsigned long)blinkInterval) {
    previousMillis = now;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(LED_BUILTIN, ledState);
  }
}

// ── NAPT not supported branch ─────────────────────────────────────────────
#else

void setup() {
  Serial.begin(115200);
  Serial.println(F("[ERROR] NAPT not supported in this lwIP build"));
  Serial.println(F("        Recompile with LWIP_FEATURES=1 and LWIP_IPV6=0"));
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off
}

void loop() {}

#endif  // LWIP_FEATURES && !LWIP_IPV6