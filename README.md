# esp8266_temperature - beware of vibe coding!

Ein zuverlässiger WLAN-Temperaturmonitor auf Basis eines ESP8266 (NodeMCU) mit DS18B20-Sensor.
Entwickelt für den dauerhaften Einsatz z. B. in Dojos, Trainingsräumen oder Technikräumen.

## Funktionen

* 🌡️ Temperaturmessung mit **DS18B20** (1-Wire)
* 🚦 **Ampel-LEDs** für sofortige Statusanzeige

  * Grün: Temperatur OK
  * Gelb: Warnbereich
  * Rot: Kritischer Bereich
* 📲 **Push-Benachrichtigungen** über **ntfy.sh**
* ❤️ **Täglicher Heartbeat** („System läuft, aktuelle Temperatur …“)
* 🌐 **Web-Statusseite** (lokal im WLAN)
* ⚙️ **Einfache Einrichtung** per WiFiManager (Captive Portal)
* 💾 Persistente Speicherung der Grenzwerte (EEPROM)
* 🔘 Reset-Taster zum Zurücksetzen der WLAN-Konfiguration

---

## Hardware

* NodeMCU / ESP8266
* DS18B20 Temperatursensor (empfohlen mit 4,7 kΩ Pull-Up)
* 3 LEDs (rot, gelb, grün) + Vorwiderstände (z. B. 220–330 Ω)
* Taster (für WLAN-Reset)
* USB-Stromversorgung (Netzteil oder z. B. FRITZ!Box)

---

## Inbetriebnahme

1. **Push-Dienst vorbereiten**

   * Konto bei [https://ntfy.sh](https://ntfy.sh) anlegen (oder öffentlichen Server nutzen)
   * Gewünschtes Topic abonnieren (z. B. per App oder Browser)

2. **Gerät starten**

   * ESP8266 mit Strom versorgen
   * Es erscheint ein WLAN-Hotspot (z. B. `Dojo-Temp`)

3. **Einrichtung**

   * Mit Smartphone oder Laptop verbinden
   * Browser öffnet sich automatisch
     (alternativ: [http://192.168.4.1](http://192.168.4.1))
   * WLAN auswählen
   * Temperatur-Grenzwerte festlegen
   * Speichern

4. **Fertig**

   * Gerät verbindet sich mit dem WLAN
   * LEDs zeigen den aktuellen Status
   * Push-Nachrichten werden gesendet

---

## Web-Statusseite

Im normalen Betrieb ist das Gerät unter seiner lokalen IP erreichbar:

```
http://<IP-des-Geräts>/
```

Angezeigt werden u. a.:

* aktueller Temperaturwert
* Status (OK / Warnung / Kritisch)
* konfigurierte Grenzwerte
* IP-Adresse
* Laufzeit (Uptime)

Die Seite aktualisiert sich automatisch.

---

## Reset-Funktion

* **Reset-Taster ≥ 3 Sekunden gedrückt halten**
* WLAN-Zugangsdaten werden gelöscht
* Gerät startet neu im Einrichtungsmodus

---

## Hinweise zum Betrieb

* Für stabile Messungen bei längeren Kabeln:

  * saubere Lötverbindungen
  * korrekter Pull-Up-Widerstand
* LEDs können bei Bedarf gedimmt werden (z. B. höhere Widerstände)
* Dauerbetrieb über USB empfohlen

---

## Sicherheit & Konfiguration

* **Keine Zugangsdaten im Repository**
* WLAN-Daten und Grenzwerte werden ausschließlich lokal gespeichert
* Push-Topic kann individuell gesetzt werden

---

## Lizenz

Open Source – zur freien Nutzung, Anpassung und Erweiterung.
Keine Gewährleistung.

---

Viel Erfolg beim Einsatz 🥋


## Technical Description

This project implements a standalone temperature monitoring system based on an ESP8266 microcontroller and a DS18B20 digital temperature sensor.

### Hardware Architecture

* **Microcontroller:** ESP8266 (e.g. NodeMCU / Wemos D1 mini)
* **Temperature Sensor:** DS18B20 (1-Wire bus)
* **User Interface:**

  * Three LEDs (green / yellow / red) for local status indication
  * One push button for long-press reset (WiFi configuration)
* **Connectivity:** 2.4 GHz WiFi

### Software Architecture

The firmware is written using the Arduino framework for ESP8266 and follows a non-blocking, interval-based design.

Main components:

* **WiFiManager**

  * Handles initial WiFi provisioning via captive portal
  * Allows runtime configuration of temperature thresholds
* **EEPROM**

  * Persistent storage for warning and critical temperature thresholds
  * Persistent storage of the last heartbeat timestamp
* **Temperature Monitoring**

  * Periodic measurement using DallasTemperature library
  * Configurable measurement interval
  * Immediate first measurement after boot
* **State Machine**

  * Three temperature states: OK, WARNING, CRITICAL
  * State transitions trigger push notifications and LED updates
* **Push Notifications**

  * Uses ntfy.sh via HTTPS (WiFiClientSecure)
  * Supports priorities for different message types
  * Includes startup message, state changes, sensor errors, and daily heartbeat
* **Web Status Interface**

  * Embedded HTTP server on port 80
  * Provides a minimal status page with current temperature, thresholds, state, IP and uptime
  * HTTP headers and meta tags prevent caching
  * Optional automatic refresh aligned with measurement interval
* **Heartbeat Mechanism**

  * Sends a daily “system alive” notification
  * Timestamp stored in EEPROM to survive reboots
* **Failsafe Behavior**

  * Sensor disconnection triggers an immediate error notification
  * Long button press (>3 seconds) resets WiFi credentials and reboots

### Timing Model

* All periodic actions are based on `millis()` timing
* No blocking delays in the main loop (except short, controlled sensor stabilization delays)
* First temperature measurement is executed immediately after startup

### Security Considerations

* No credentials or secrets are stored in the source code
* Sensitive values (WiFi, ntfy topic, passwords) are externalized via a `config.h` file
* `config.h` is excluded from version control via `.gitignore`
* A `config.example.h` file documents required configuration parameters

### Design Goals

* High reliability with minimal complexity
* Clear separation between configuration, logic, and presentation
* Stable long-term unattended operation
* Easy deployment and reproducibility

