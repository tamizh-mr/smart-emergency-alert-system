# smart-emergency-alert-system
AI-powered IoT safety system that detects gas, fire &amp; distress calls and auto-alerts emergency services via Telegram 🚨
# 🚨 Smart Emergency Alert System (AI)

> An AI-powered IoT safety system that detects gas leaks, fire, air pollution, and distress calls — and automatically alerts emergency services via Telegram.

![Status](https://img.shields.io/badge/Status-Completed-brightgreen)
![Hardware](https://img.shields.io/badge/Hardware-ESP32-blue)
![ML](https://img.shields.io/badge/ML-Edge%20Impulse-orange)
![Award](https://img.shields.io/badge/Hackathon-2nd%20Prize%20🥈-silver)

---

## 📸 Project Photos

<img src="pic" width="45%"/>
<img src="Project outlook" width="45%"/>

---

## 💡 About

A real-time emergency detection system built on ESP32 that monitors multiple environmental hazards simultaneously. When a threat is detected, it instantly notifies the user via Telegram with location. If no response, it auto-escalates to police and fire departments.

Built and presented at a Hackathon — secured **2nd Prize 🥈**

---

## ⚙️ Features

- 🔥 **Fire Detection** — Flame sensor triggers instant alert + buzzer
- 💨 **Gas Leak Detection** — MQ-2 sensor detects LPG/smoke, cuts power via relay
- 🌫️ **Air Pollution Monitoring** — MQ-135 tracks air quality
- 🎤 **AI Voice/Scream Detection** — TinyML model via Edge Impulse detects distress calls
- 📱 **Telegram Bot Alerts** — Real-time notifications with Google Maps location
- 🚔 **Auto Escalation** — If user doesn't respond in 60 seconds, alerts police & fire dept automatically
- 💡 **LED Indicators** — Green (safe) / Red (danger)

---

## 🛠️ Hardware Used

| Component | Purpose |
|-----------|---------|
| ESP32 | Main microcontroller + WiFi |
| MQ-2 Sensor | Gas / smoke detection |
| MQ-135 Sensor | Air quality monitoring |
| Flame Sensor | Fire detection |
| MAX4466 Mic | Audio input for AI |
| Relay Module | Power cutoff on gas leak |
| Buzzer | Audio alert for fire |
| LEDs (Red/Green) | Visual status indicators |

---

## 🧠 How the AI Works

- Microphone captures audio in real time
- TinyML model (trained on Edge Impulse) classifies the sound
- If a **scream/distress call** is detected with high confidence → Telegram alert sent
- If user doesn't respond → auto escalates to authorities

---

## 📱 Telegram Bot Commands

| Command | Action |
|---------|--------|
| `/safe` | Mark situation as safe, stop escalation |
| `/danger` | Immediately alert police & fire dept |

---

## 🚀 How to Set Up

1. Clone this repo
```bash
git clone https://github.com/tamizh-mr/smart-emergency-alert-system
```

2. Open `smart_emergency_alert_system.ino` in Arduino IDE

3. Fill in your credentials:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
```

4. Install required libraries:
   - UniversalTelegramBot
   - Edge Impulse inferencing library

5. Upload to ESP32 and you're live! ✅

---

## 📁 Repository Structure

```
smart-emergency-alert-system/
├── smart_emergency_alert_system.ino   ← Main code
├── photos/
│   ├── hardware_inside.jpg
│   └── hardware_outside.jpg
└── README.md
```

---

## 👤 Author

**Tamilmurugan M R**
- GitHub: [@tamizh-mr](https://github.com/tamizh-mr)

---

*Built with ❤️ using ESP32 + Edge Impulse + Telegram API*
