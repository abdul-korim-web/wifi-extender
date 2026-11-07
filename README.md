<div align="center">

<img src="https://img.shields.io/badge/ESP8266-NodeMCU-blue?style=for-the-badge&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/Arduino-C++-teal?style=for-the-badge&logo=arduino&logoColor=white"/>
<img src="https://img.shields.io/badge/LittleFS-Filesystem-orange?style=for-the-badge"/>
<img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge"/>

<br/><br/>

```
  ___  ___  ___  _   _ _     __        ___ ___ _   
 / _ \| _ )|   \| | | | |    \ \      / (_) __(_)  
| (_) | _ \| |) | |_| | |__   \ \/\/ /| | _| | |   
 \__,_|___/|___/ \___/|____|   \_/\_/ |_|_| |_|_|  
  _____      _                 _                    
 | ____|_  _| |_ ___ _ __   __| | ___ _ __          
 |  _| \ \/ / __/ _ \ '_ \ / _` |/ _ \ '__|         
 | |___ >  <| ||  __/ | | | (_| |  __/ |            
 |_____/_/\_\\__\___|_| |_|\__,_|\___|_|            
```

# 📡 Abdul WiFi Extender

### *A Professional ESP8266-Based WiFi Range Extender with Modern Admin Dashboard*

**Developed by [Abdul Korim](https://github.com/abdul-korim-web)**  
*Based on original work by [Pius Onyema Ndukwu](https://github.com/Pius171/esp8266-wifi-extender)*

---

</div>

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🔁 **WiFi Repeater** | Extends your existing WiFi using lwIP NAPT |
| 🖥️ **Admin Dashboard** | Beautiful dark-themed web interface |
| 📶 **Network Scanner** | Scan and select WiFi networks from browser |
| 👥 **Device Monitor** | View all connected clients (MAC + IP) |
| 📊 **Live Stats** | Real-time RSSI, heap memory, uptime |
| 🔐 **Secure Login** | Session-based authentication |
| ⚙️ **Easy Config** | Configure everything without reflashing |
| 🔄 **Factory Reset** | One-click reset via web or GPIO0 button |
| 🎬 **Video Review** | Step-by-step installation & review video |

---

## 📹 Installation & Review Video

Watch the complete hardware setup, installation walkthrough, and dashboard review:

<div align="center">

[![Watch the Video](https://img.shields.io/badge/▶%20Watch%20Video-Installation%20%26%20Review-red?style=for-the-badge&logo=youtube&logoColor=white)](review/wifi%20extender%20using%20exp8266.mp4)

*Video file location:* [`review/wifi extender using exp8266.mp4`](review/wifi%20extender%20using%20exp8266.mp4)

</div>

---

## 🖼️ Dashboard Preview

```
┌─────────────────────────────────────────────────────┐
│  📡 Abdul WiFi Extender        [Dashboard]          │
├──────────┬──────────────────────────────────────────┤
│          │  🌐 Internet  📶 Repeater  📡 Signal     │
│ Dashboard│  ✅ Online   ✅ Active   -62 dBm Good    │
│ WiFi     │                                          │
│ Devices  │  👥 Clients   💾 Heap    ⏱️ Uptime      │
│ Monitor  │     3          28 KB      2h 15m         │
│ Settings │                                          │
│ About    │  Network Info    Device Health           │
│          │  IP: 172.217.28.254  ████████░░ 78% heap │
│ [Logout] │  AP: AbdulWiFi       ████████████ Good   │
└──────────┴──────────────────────────────────────────┘
```

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|--------------|
| **Microcontroller** | ESP8266 NodeMCU ESP-12E |
| **Flash Memory** | 4MB (required) |
| **Power** | 5V via USB or 3.3V regulated |
| **Connection** | USB-to-Serial (CH340 / CP2102) |

---

## 📦 Required Libraries

> Install these before compiling!

| Library | Version | How to Install |
|---------|---------|---------------|
| **ArduinoJson** | 6.x *(not v7!)* | Arduino Library Manager |
| **ESPAsyncWebServer** | 1.2.3+ | [GitHub ZIP](https://github.com/me-no-dev/ESPAsyncWebServer) |
| **ESPAsyncTCP** | 1.2.2+ | [GitHub ZIP](https://github.com/me-no-dev/ESPAsyncTCP) |
| **LittleFS** | Built-in | Included with ESP8266 Core |
| **lwIP NAPT** | Built-in | Included with ESP8266 Core 3.x |

---

## ⚡ Installation Guide

> [!IMPORTANT]
> Follow these steps **in exact order**. Using wrong board versions will cause issues.

---

### 🔴 Step 1 — Erase ALL Flash Memory First

> This is **mandatory** before uploading. Skipping this causes random crashes and corrupted filesystems.

1. Open **Arduino IDE**
2. Select your board: **Tools → Board → NodeMCU 1.0 (ESP-12E Module)**
3. Select your port: **Tools → Port → COM3** *(or your port)*
4. Go to: **Tools → Erase Flash → All Flash Contents**
5. Click **Burn Bootloader** — this fully erases the chip

**OR** use esptool manually:
```bash
esptool.py --port COM3 erase_flash
```

---

### 🟡 Step 2 — Upload Firmware (.ino) using Core **3.0.0**

> [!WARNING]
> The `.ino` firmware **MUST** be uploaded with **ESP8266 Core version 3.0.0** for NAPT compatibility.

#### Set Board Version to 3.0.0:
1. Go to **Tools → Board → Boards Manager**
2. Search for **esp8266 by ESP8266 Community**
3. Select version **3.0.0** and click **Install**
4. Wait for installation to complete

#### Arduino IDE Settings for Upload:

| Setting | Value |
|---------|-------|
| Board | NodeMCU 1.0 (ESP-12E Module) |
| CPU Frequency | 80 MHz |
| **Flash Size** | **4MB (FS: 2MB, OTA: ~1019KB)** |
| **lwIP Variant** | **v2 Higher Bandwidth** |
| Upload Speed | 115200 |
| Port | COM3 *(your port)* |

> [!CAUTION]
> lwIP Variant must be **"v2 Higher Bandwidth"** — NOT IPv6. NAPT will fail otherwise!

#### Upload the firmware:
1. Open `wifi_extender.ino` in Arduino IDE
2. Apply all settings from the table above
3. Click **Upload** (→) or press `Ctrl+U`
4. Wait for **"Done uploading"** message

---

### 🟢 Step 3 — Upload Data Folder (LittleFS) using Core **3.1.2+**

> [!WARNING]
> The LittleFS data upload **MUST** use **ESP8266 Core version 3.1.2 or higher**.

#### Switch Board Version to 3.1.2+:
1. Go to **Tools → Board → Boards Manager**
2. Search **esp8266 by ESP8266 Community**
3. Select version **3.1.2** *(or the latest available)* → **Install**

#### Install the LittleFS Upload Plugin:

**For Arduino IDE 1.x:**
1. Download from: https://github.com/earlephilhower/arduino-littlefs-upload
2. Place the `.jar` file in: `Arduino/tools/ESP8266LittleFS/tool/`
3. Restart Arduino IDE

**For Arduino IDE 2.x:**
1. Download the `.vsix` plugin from the same GitHub link
2. In Arduino IDE 2: `File → Preferences → Additional plugins`
3. Or install via VS Code extension marketplace

#### Upload the data folder:
1. Make sure **Serial Monitor is CLOSED** *(very important!)*
2. Go to **Tools → ESP8266 LittleFS Data Upload**
3. Wait for **"LittleFS Image Uploaded"** message

> [!NOTE]
> The `data/` folder contains all dashboard HTML, CSS, and JS files. Without this step, the dashboard will not load.

---

### ✅ Step 4 — First Boot & Accessing Dashboard

1. **Initial Setup (Before WiFi is configured):**
   - Power on the ESP8266.
   - It will create an open WiFi network: **`AbdulWiFi-Setup`**.
   - Connect your phone or laptop to **`AbdulWiFi-Setup`**.
   - Open your browser and go to:
     ```
     http://192.168.4.1
     ```

2. **Connected State (After connecting to upstream WiFi):**
   - Once the ESP8266 connects to your router, it starts operating in repeater (NAPT) mode.
   - Connect to your extended WiFi network (e.g., **`AbdulWiFi`**).
   - The Admin Dashboard page is accessible at:
     ```
     http://172.217.28.254
     ```
   *(You can also use the local IP assigned by your main router).*

3. **Login Credentials:**

| Field | Value |
|-------|-------|
| **Username** | `admin` |
| **Password** | `admin` |

> [!CAUTION]
> Change the default password immediately after first login via **Settings → Change Admin Password**!

---

## 🌐 Dashboard Pages

| Page | URL (Connected Mode) | URL (Setup Mode) | Description |
|------|----------------------|------------------|-------------|
| 🔐 Login | `http://172.217.28.254/` | `http://192.168.4.1/` | Secure admin login |
| 🏠 Dashboard | `/dashboard.html` | `/dashboard.html` | Live overview — RSSI, clients, heap, uptime |
| 📶 WiFi Config | `/wifi.html` | `/wifi.html` | Scan networks, configure upstream + extended WiFi |
| 👥 Devices | `/clients.html` | `/clients.html` | Connected client list (MAC + IP) |
| 📊 Monitor | `/monitor.html` | `/monitor.html` | RSSI history chart, memory usage |
| ⚙️ Settings | `/settings.html` | `/settings.html` | Password change, restart, factory reset |
| ℹ️ About | `/about.html` | `/about.html` | Product info and tech details |

---

## 🔌 API Endpoints

All endpoints require `Authorization: Bearer <token>` header (except `/api/login`).

```
POST /api/login          → Authenticate, get session token
POST /api/logout         → End session
GET  /api/status         → Live device stats (RSSI, heap, uptime…)
GET  /api/scan           → Scan nearby WiFi networks
GET  /api/clients        → List connected AP clients
GET  /api/sysinfo        → Firmware & memory details
GET  /api/wifi-config    → Current saved SSID (password hidden)
POST /api/wifi-config    → Save new WiFi credentials → restarts
POST /api/restart        → Reboot the device
POST /api/reset-wifi     → Erase WiFi config → restarts
POST /api/factory-reset  → Format LittleFS → restarts
POST /api/change-password → Change admin password
```

---

## 💡 LED Status Guide

| LED Pattern | Meaning |
|-------------|---------|
| 🔴 Solid ON | Connecting to router… |
| 🟢 Fast blink (200ms) | ✅ Repeater active — NAPT working |
| 🟡 Slow blink (1s) | Setup portal mode — waiting for config |
| ⚡ Rapid flash (100ms) | LittleFS mount error |
| ⚫ Solid OFF | NAPT not supported in this build |

---

## 🔘 Hardware Button (GPIO0 / FLASH)

| Action | Result |
|--------|--------|
| Hold **≥ 3 seconds** | 🔴 Factory reset — erases LittleFS and restarts |
| Short press | No effect during normal operation |

---

## 🏗️ How It Works

```
  ┌──────────┐     WiFi STA      ┌────────────────────┐
  │  Router  │ ◄─────────────── │                    │
  │ Internet │                  │   ESP8266 NodeMCU  │
  └──────────┘                  │                    │
                                │  lwIP NAPT Engine  │
  ┌──────────┐     WiFi AP      │                    │
  │  Devices │ ◄─────────────── │                    │
  │ Clients  │                  └────────────────────┘
  └──────────┘
```

1. ESP8266 connects to your existing router as a **Station (STA)**
2. Simultaneously creates its own **SoftAP** with configurable SSID
3. **lwIP NAPT** routes all AP client traffic through the upstream connection
4. DNS servers from upstream router are relayed to AP clients

---

## ⚠️ Security Notes

| Limitation | Details |
|------------|---------|
| **Plain text storage** | Credentials stored in LittleFS — no hardware TPM on ESP8266 |
| **No HTTPS** | Insufficient RAM for TLS — use on trusted local networks only |
| **Session tokens** | 30-minute expiry, single-session model |
| **Recommendation** | Never expose admin interface to the internet |

---

## 📁 Project Structure

```
wifi_extender/
├── 📄 wifi_extender.ino     ← Main firmware entry point
├── 📄 WM.h                  ← WiFi Manager class header
├── 📄 WM.cpp                ← Server routes & WiFi logic
└── 📂 data/                 ← LittleFS filesystem root
    ├── 🌐 index.html        ← Login page
    ├── 🌐 dashboard.html    ← Main dashboard
    ├── 🌐 wifi.html         ← WiFi configuration
    ├── 🌐 clients.html      ← Connected devices
    ├── 🌐 monitor.html      ← Network monitoring
    ├── 🌐 settings.html     ← System settings
    ├── 🌐 about.html        ← About page
    ├── 🎨 style.css         ← Dark theme stylesheet
    └── ⚙️  app.js            ← Shared JavaScript
```

---

## 🔧 Troubleshooting

<details>
<summary><b>❌ "Bad JSON" response when saving WiFi</b></summary>

Re-upload the firmware. Make sure you are using Core 3.0.0 for the .ino file.
</details>

<details>
<summary><b>❌ Dashboard shows blank page after LittleFS upload</b></summary>

The data folder was not uploaded correctly. Make sure:
- Serial Monitor is **closed** before uploading data
- You used Core **3.1.2+** for the LittleFS upload
- Flash Size is set to **4MB (FS: 2MB)**
</details>

<details>
<summary><b>❌ COM port "Access Denied" error</b></summary>

Close Arduino Serial Monitor. Unplug and replug the USB cable. Try again.
</details>

<details>
<summary><b>❌ NAPT not working — no internet on extended WiFi</b></summary>

Check that lwIP Variant is set to **"v2 Higher Bandwidth"** (not IPv6) in Arduino IDE Tools menu.
</details>

<details>
<summary><b>❌ Forgot admin password</b></summary>

Hold the **GPIO0 (FLASH) button for 3 seconds** to factory reset, or erase flash and re-upload everything.
</details>

<details>
<summary><b>❌ 'dhcps_set_dns' compile error</b></summary>

This is a Core version API change. The firmware uses `dns_setserver()` from lwIP directly — make sure you have the latest version of this project's `WM.cpp`.
</details>

---

## 📜 License

```
MIT License

Copyright (c) 2024 Abdul Korim
Based on original work by Pius Onyema Ndukwu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## 🙏 Credits

| Role | Name | Link |
|------|------|------|
| **Developer** | Abdul Korim | [github.com/abdul-korim-web](https://github.com/abdul-korim-web) |
| **Original Author** | Pius Onyema Ndukwu | [esp8266-wifi-extender](https://github.com/Pius171/esp8266-wifi-extender) |
| **Hardware** | Espressif Systems | [ESP8266](https://www.espressif.com/en/products/socs/esp8266) |

---

<div align="center">

**Made with ❤️ for the ESP8266 maker community**

⭐ *If this project helped you, please consider giving it a star!* ⭐

</div>

