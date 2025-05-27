#include <Arduino.h>

// TB6612FNG Motortreiber 1 vorne (Motor 1 + Motor 2)
#define AIN1_1 25
#define AIN2_1 26
#define PWMA_1 33
#define BIN1_1 27
#define BIN2_1 14
#define PWMB_1 32
#define STBY_1 4

// TB6612FNG Motortreiber 2 hinten (Motor 3 + Motor 4)
#define AIN1_2 23
#define AIN2_2 22
#define PWMA_2 21
#define BIN1_2 19
#define BIN2_2 18
#define PWMB_2 5
#define STBY_2 15

// PWM-Kanäle
#define CH_PWMA_1 0
#define CH_PWMB_1 1
#define CH_PWMA_2 2
#define CH_PWMB_2 3

// Globale Variablen
int currentPWM;// aktuelle Geschwindigkeit, wird immer aktualisiert
int targetPWM; // Zielgeschwindigkeit
bool motorsAreStopping = false;
bool directontrolActive = false;

void setup() {
  Serial.begin(115200);

  // Motor 1 + 2 (Treiber 1)
  pinMode(AIN1_1, OUTPUT);
  pinMode(AIN2_1, OUTPUT);
  pinMode(PWMA_1, OUTPUT);
  pinMode(BIN1_1, OUTPUT);
  pinMode(BIN2_1, OUTPUT);
  pinMode(PWMB_1, OUTPUT);
  pinMode(STBY_1, OUTPUT);

  // Motor 3 + 4 (Treiber 2)
  pinMode(AIN1_2, OUTPUT);
  pinMode(AIN2_2, OUTPUT);
  pinMode(PWMA_2, OUTPUT);
  pinMode(BIN1_2, OUTPUT);
  pinMode(BIN2_2, OUTPUT);
  pinMode(PWMB_2, OUTPUT);
  pinMode(STBY_2, OUTPUT);

  // Treiber aktivieren
  digitalWrite(STBY_1, HIGH);
  digitalWrite(STBY_2, HIGH);

  // PWM Setup
  ledcSetup(CH_PWMA_1, 5000, 8);
  ledcAttachPin(PWMA_1, CH_PWMA_1);

  ledcSetup(CH_PWMB_1, 5000, 8);
  ledcAttachPin(PWMB_1, CH_PWMB_1);

  ledcSetup(CH_PWMA_2, 5000, 8);
  ledcAttachPin(PWMA_2, CH_PWMA_2);

  ledcSetup(CH_PWMB_2, 5000, 8);
  ledcAttachPin(PWMB_2, CH_PWMB_2);

  emergencyStop();
  Serial.println("✅ Roboter bereit! Warte auf Befehle: F (vor), B (zurück), L (links), R (rechts), S (stopp), N (notaus)");
}

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 20; // alle 20ms
String serialBuffer = "";

void loop() {
  if (Serial.available()) {
    char incoming = Serial.read();
    if (incoming == '\n') {
      processCommand(serialBuffer);
      serialBuffer = ""; // Buffer leeren nach Verarbeitung
    } else {
      serialBuffer += incoming; // Zeichen anhängen
    }
  }

  if (millis() - lastUpdateTime >= updateInterval) {
    updateMotors();
    lastUpdateTime = millis();
  }
}

void processCommand(String input) {
  input.trim();
  if (input.length() == 0) return;

  char cmd = input.charAt(0); // erstes Zeichen = Befehl

  // Modell 2: Direktansteuerung der linken und rechten Seite
  if (cmd == 'M') {
    int commaIndex = input.indexOf(',');
    if (commaIndex != -1) {
      int left = input.substring(1, commaIndex).toInt();
      int right = input.substring(commaIndex + 1).toInt();
      controlMotorsDirect(left, right);
    } else {
      Serial.println("❌ Ungültiges M-Format. Beispiel: M120,100");
    }
    return;
  }

  // Modell 1: klassische Richtung + Geschwindigkeit
  int speed = 200;
  if (input.length() > 1) {
    speed = input.substring(1).toInt(); 
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;   
  }

  if (cmd == 'F') {
    targetPWM = speed;
    driveForward();
  } else if (cmd == 'B') {
    targetPWM = speed;
    driveBackward();
  } else if (cmd == 'L') {
    targetPWM = speed;
    turnLeft();
  } else if (cmd == 'R') {
    targetPWM = speed;
    turnRight();
  } else if (cmd == 'S') {
    stopMotors();
  } else if (cmd == 'N') {
    emergencyStop();
  } else {
    Serial.print("❌ Ungültiger Befehl: ");
    Serial.println(cmd);
  }
}

void driveForward() {
  directontrolActive = false;
  // Vorwärts für alle Motoren
  setMotor(1, true);
  setMotor(2, true);
  setMotor(3, true);
  setMotor(4, true);

  Serial.println("🚗 Vorwärts");
}

void driveBackward() {
  directontrolActive = false;
  // Rückwärts für alle Motoren
  setMotor(1, false);
  setMotor(2, false);
  setMotor(3, false);
  setMotor(4, false);

  Serial.println("🚙 Rückwärts");
}

void turnLeft() {
  directontrolActive = false;
  // Linksdrehen (Motoren gegenläufig)
  setMotor(1, false);
  setMotor(2, true);
  setMotor(3, false);
  setMotor(4, true);

  Serial.println("↩️ Links drehen");
}

void turnRight() {
  directontrolActive = false;
  // Rechtsdrehen (Motoren gegenläufig)
  setMotor(1, true);
  setMotor(2, false);
  setMotor(3, true);
  setMotor(4, false);

  Serial.println("↪️ Rechts drehen");
}

void emergencyStop() {
  directontrolActive = false;
  // Alles stoppen
  ledcWrite(CH_PWMA_1, 0);
  ledcWrite(CH_PWMB_1, 0);
  ledcWrite(CH_PWMA_2, 0);
  ledcWrite(CH_PWMB_2, 0);

  currentPWM = 0;
  targetPWM = 0;
  Serial.println("🚨 NOT-AUS aktiviert!");
}

void stopMotors() {
  directontrolActive = false;
  motorsAreStopping = true;
  targetPWM = 0;
  Serial.println("🛑 Stoppen eingeleitet");
}

void updateMotors() {
  if (directontrolActive) return; // bei Modell 2: nichts tun

  if (motorsAreStopping) {
    if (currentPWM > 0) {
      currentPWM -= 5;
      if (currentPWM < 0) currentPWM = 0;
    } 
    if (currentPWM == 0) {
      motorsAreStopping = false; // fertig mit Bremsen
    }
  } else {
    if (currentPWM < targetPWM) {
      currentPWM += 5;
      if (currentPWM > targetPWM) currentPWM = targetPWM;
    } else if (currentPWM > targetPWM) {
      currentPWM -= 5;
      if (currentPWM < targetPWM) currentPWM = targetPWM;
    }
  }

  // Motoren aktualisieren
  ledcWrite(CH_PWMA_1, currentPWM);
  ledcWrite(CH_PWMB_1, currentPWM);
  ledcWrite(CH_PWMA_2, currentPWM);
  ledcWrite(CH_PWMB_2, currentPWM);
}

void setMotor(int motorNum, bool forward) {
  switch(motorNum) {
    case 1:
      digitalWrite(AIN1_1, forward ? HIGH : LOW);
      digitalWrite(AIN2_1, forward ? LOW : HIGH);
      break;
    case 2:
      digitalWrite(BIN1_1, forward ? HIGH : LOW);
      digitalWrite(BIN2_1, forward ? LOW : HIGH);
      break;
    case 3:
      digitalWrite(AIN1_2, forward ? HIGH : LOW);
      digitalWrite(AIN2_2, forward ? LOW : HIGH);
      break;
    case 4:
      digitalWrite(BIN1_2, forward ? HIGH : LOW);
      digitalWrite(BIN2_2, forward ? LOW : HIGH);
      break;
  }
}

void controlMotorsDirect(int left, int right) {
  motorsAreStopping = false;
  directontrolActive = true;

  // Begrenzung der Eingabe auf gültige Werte
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  // Richtung setzen
  bool leftForward = (left >= 0);
  bool rightForward = (right >= 0);

  // Links (Motor 1 & 3)
  setMotor(1, leftForward);
  setMotor(3, leftForward);

  // Rechts (Motor 2 & 4)
  setMotor(2, rightForward);
  setMotor(4, rightForward);

  // PWM setzen (Betrag der Werte)
  ledcWrite(CH_PWMA_1, abs(left)); // Motor 1 (vorne links)
  ledcWrite(CH_PWMA_2, abs(left)); // Motor 3 (hinten links)
  ledcWrite(CH_PWMB_1, abs(right)); // Motor 2 (vorne rechts)
  ledcWrite(CH_PWMB_2, abs(right)); // Motor 4 (hinten rechts)

  Serial.print("🎯 Direktsteuerung: L=");
  Serial.print(left);
  Serial.print(" R=");
  Serial.println(right);
}