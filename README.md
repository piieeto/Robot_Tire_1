# 🤖 Robot — ESP32 gesteuerter 4-Rad-Roboter mit Gamepad- und Konsolensteuerung

Dieses Projekt enthält den Code für einen differenziell gesteuerten Roboter mit vier DC-Motoren, einem ESP32 , Raspberry Pi 4b und zwei TB6612FNG-Motortreibern.  
Der Roboter kann entweder per Konsole (über USB) oder über ein Bluetooth-Gamepad (z. B. 8BitDo Ultimate) gesteuert werden.

---

## 📦 Hardware

| Komponente          | Details                              |
|---------------------|--------------------------------------|
| Mikrocontroller     | ESP32                                |
| Motortreiber        | 2× TB6612FNG                         |
| Motoren             | 4× N20 Getriebemotoren (300 RPM, 6 V)|
| Stromversorgung     | 4× 18650 Akkus in Halterung          |
| Steuerung           | Raspberry Pi (USB-Serial an ESP32) + Bluetooth-Gamepad |
| Chassis             | Eigenbau, 3D-gedruckt                |

---

## ⚙️ Aufbau & Pinbelegung

### Motorzuordnung

| Motor-Nr | Position          |
|----------|-------------------|
| 1        | vorne links        |
| 2        | vorne rechts       |
| 3        | hinten links       |
| 4        | hinten rechts      |

### ESP32 Pinbelegung

| Signal     | ESP32 Pin | Beschreibung                   |
|------------|-----------|-------------------------------|
| AIN1_1     | 25        | Motor 1 Richtung              |
| AIN2_1     | 26        | Motor 1 Richtung              |
| PWMA_1     | 33        | Motor 1 PWM                   |
| BIN1_1     | 27        | Motor 2 Richtung              |
| BIN2_1     | 14        | Motor 2 Richtung              |
| PWMB_1     | 32        | Motor 2 PWM                   |
| STBY_1     | 4         | Standby Treiber 1             |
| AIN1_2     | 23        | Motor 3 Richtung              |
| AIN2_2     | 22        | Motor 3 Richtung              |
| PWMA_2     | 21        | Motor 3 PWM                   |
| BIN1_2     | 19        | Motor 4 Richtung              |
| BIN2_2     | 18        | Motor 4 Richtung              |
| PWMB_2     | 5         | Motor 4 PWM                   |
| STBY_2     | 15        | Standby Treiber 2             |

---

## 🧠 Steuerung

### Modell 1 – Konsolenmodus (über USB)

Einfache Steuerbefehle direkt über die Konsole:

- `F100` → Vorwärts mit PWM 100
- `B100` → Rückwärts
- `L100` → Drehen nach links
- `R100` → Drehen nach rechts
- `S`    → Sanft stoppen
- `N`    → Not-Aus (sofortiger Stopp)

### Modell 2 – Gamepadmodus (differenziell)

Stickbewegungen am Gamepad werden zu `M[l,r]`-Befehlen:

- `M255,255` → Vorwärts
- `M255,100` → Rechtskurve
- `M-255,255` → Drehung auf der Stelle
- `M0,0` → Stopp
- Taste B → Sofort-Stopp (`M0,0`)

---

## 🔧 Auto-Balance

Falls die Motoren nicht gleich schnell laufen, können individuelle Korrekturen gesetzt werden:

```cpp
float motorCorrection[4] = {1.00, 1.00, 1.00, 1.00}; // Motor 1–4
```

Diese Korrekturen werden auf die PWM-Werte angewendet.

---

## 📁 Projektstruktur

| Datei                 | Beschreibung                             |
|------------------------|------------------------------------------|
| `esp32_main.ino`       | Hauptcode für den ESP32 (Motorkontrolle)|
| `main.py`              | Python-Interface (Konsole + Gamepad)     |
| `serial_interface.py`  | Serielle Kommunikation mit ESP32         |
| `gamepad_control.py`   | Gamepad-Input → PWM-Berechnung & Ausgabe |

---

## 📌 Lizenz

MIT License – du darfst alles verwenden, anpassen & verbessern.  
Beiträge willkommen!

---

## 👤 Autor

[GitHub: @piieeto](https://github.com/piieeto)


---

## 📡 Kommunikation

Der Raspberry Pi übernimmt die zentrale Steuerung und sendet über USB-Serial-Befehle an den ESP32.

### Steueroptionen:

- **Konsolenmodus:** Eingabe über `main.py` via Terminal
- **Gamepadmodus:** Bluetooth-Gamepad (8BitDo, Xbox, etc.) wird mit `evdev` gelesen → Ausgabe über `M[left,right]`

> Verbindung über `/dev/ttyUSB0`, Baudrate 115200

Bluetooth-Gamepads werden direkt über den Raspberry Pi verbunden. Der ESP32 empfängt ausschließlich serielle Steuerbefehle.
