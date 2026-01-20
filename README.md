# esp8266_temperature

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

## Inbetriebnahme (Kurzfassung)

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

Die Seite aktualisiert sich automatisch und wird bewusst **nicht gecacht**.

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
