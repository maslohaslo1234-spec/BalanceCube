/*
 * BalanceCube - Reaction Wheel Balancing Cube Firmware (Baseline)
 * Microcontroller: ESP32 DevKit 30-pin
 * Actuators: 3x Nidec 24H Brushless DC Motors
 * Sensor: MPU-6050 (GY-521)
 *
 * Designed for custom carrier PCB (KiCad) & 150mm modular frame (Onshape).
 */

#include <Wire.h>

// ==========================================
// 1. PCB PIN DEFINITIONS
// ==========================================

#define PIN_LED 2   // Wbudowana niebieska dioda na płytce ESP32

// I2C pins for MPU6050 gyroscope
#define PIN_IMU_SDA    21
#define PIN_IMU_SCL    22
#define MPU_ADDR       0x68

// MOTOR 1 (Connector J3 on PCB)
#define PIN_M1_PWM     32
#define PIN_M1_DIR      4
#define PIN_M1_ENC_A   35
#define PIN_M1_ENC_B   33

// MOTOR 2 (Connector J4 on PCB)
#define PIN_M2_PWM     25
#define PIN_M2_DIR     15
#define PIN_M2_ENC_A   13
#define PIN_M2_ENC_B   14

// MOTOR 3 (Connector J5 on PCB)
#define PIN_M3_PWM     18
#define PIN_M3_DIR      5
#define PIN_M3_ENC_A   16
#define PIN_M3_ENC_B   17

// SHARED BRAKE FOR ALL MOTORS
#define PIN_ALL_BRAKE  26

// ==========================================
// 2. HARDWARE PARAMETERS & PD GAINS
// ==========================================

const int PWM_FREQ = 20000;    // 20 kHz - quiet motor operation (outside audible range)
const int PWM_RES  = 8;        // 8-bit resolution (speed range 0 - 255)

// PD controller tuning parameters (initial values, to be tuned on the assembled cube)
float Kp = 15.0;  // Proportional reaction force to tilt angle
float Kd = 1.2;   // Damping force (damper) reaction to rotational speed

unsigned long lastLoopTime = 0;         // Kiedy ostatnio wykonaliśmy pętlę
const unsigned long LOOP_TIME_US = 10000; // Co ile mikrosekund ma ruszać pętla (10 000 us = 10 ms = 100 Hz)

const float DT = LOOP_TIME_US / 1000000.0; // czyli 0.01 sekundy

float currentAngle = 0.0;
float currentGyroRate = 0.0;

bool isArmed = false;             // Czy kostka jest uzbrojona? Na starcie NIE (false)
unsigned long uprightTimer = 0;   // Stoper liczący, od kiedy kostka jest blisko pionu

float gyroOffset = 0.0; // Tu zapiszemy nasz wyliczony błąd zera

// ==========================================
// 3. MOTOR CONTROL (setMotor)
// ==========================================
// motorId: 1, 2 or 3
// speed: 0 (stop) to 255 (maximum speed)
// dir: 1 (right / CW) or 0 (left / CCW)
void setMotor(int motorId, int speed, int dir) {
  // Safety: constrain values below 0 and above 255
  speed = constrain(speed, 0, 255);

  if (motorId == 1) {
    digitalWrite(PIN_M1_DIR, dir);
    ledcWrite(PIN_M1_PWM, speed);
  } else if (motorId == 2) {
    digitalWrite(PIN_M2_DIR, dir);
    ledcWrite(PIN_M2_PWM, speed);
  } else if (motorId == 3) {
    digitalWrite(PIN_M3_DIR, dir);
    ledcWrite(PIN_M3_PWM, speed);
  }
}


// ==========================================
// 4. PD CONTROLLER (balanceControl)
// ==========================================
void balanceControl(float angle, float gyroRate) {

  // STAN 1: Kostka jeszcze nie jest uzbrojona (czeka na pion przez 1 sekundę)
  if (isArmed == false) {
    setMotor(1, 0, 0); // Silnik na pewno ma stać!

    if (abs(angle) < 10.0) {
      // Jeśli trzymasz ją blisko pionu, sprawdzamy, czy minęła już 1 sekunda (1000 ms):
      if (millis() - uprightTimer >= 1000) {
        isArmed = true; // UZBROJONA!
        Serial.println("[SYSTEM] Kostka uzbrojona! Zaczynam balansować.");
      }
    } else {
      // Jeśli jest poza pionem (>= 10 stopni), ciągle zerujemy stoper:
      uprightTimer = millis();
    }

    return; // Wychodzimy z funkcji, żeby nie liczyć PD dopóki się nie uzbroi
  }

  // STAN 2: Kostka JEST uzbrojona, ale sprawdzamy, czy nie upadła
  if (abs(angle) > 35.0) {
    setMotor(1, 0, 0);
    isArmed = false;         // Rozbrajamy ją! Musisz znowu przytrzymać ją 1s w pionie
    uprightTimer = millis(); // Zerujemy stoper
    Serial.println("[SYSTEM] Upadek! Rozbrojono silniki.");
    return;
  }

  
  
  // Calculate power as the sum of proportional and derivative terms:
  float motorPower = (Kp * angle) + (Kd * gyroRate);

  // Check sign: positive = right, negative = left
  if (motorPower >= 0) {
    setMotor(1, (int)motorPower, 1);
  } else {
    setMotor(1, (int)(-motorPower), 0);
  }
}

// ==========================================
// 5. KALIBRACJA ŻYROSKOPU
// ==========================================
void calibrateGyro() {
  digitalWrite(PIN_LED, HIGH); // ZAPALAMY diodę: "Nie ruszaj kostki!"
  Serial.println("[IMU] połóż kostkę nieruchomo - czekaj 2 sekundy na kalibrację...");
  
  delay(2000); // 2 sekundy na spokojne wpięcie XT30 i odłożenie na stół

  float sum = 0.0;
  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x45); // 0x45 to adres rejestru samego żyroskopu osi Y!
    Wire.endTransmission(false);
    Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)2, true); // Bierzemy tylko 2 bajty osi Y

    int16_t rawGyroY = Wire.read() << 8 | Wire.read();
    sum += (float)rawGyroY / 131.0; // Wrzucamy pomiar do skarbonki

    delay(3); // Krótka przerwa 3 ms między pomiarami
  }

  gyroOffset = sum / 200.0;
  digitalWrite(PIN_LED, LOW); // GASIMY diodę: "Gotowe! Można stawiać kostkę."
  Serial.print("[IMU] Skalibrowano! Offset: ");
  Serial.println(gyroOffset);
}


// ==========================================
// 6. MPU-6050 SENSOR INITIALIZATION
// ==========================================
void initMPU() {
  // 1. Pukamy do czujnika pod adresem 0x68
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();

  // 2. Sprawdzamy, czy ktoś otworzył drzwi:
  if (error != 0) {
    Serial.println("[BŁĄD] Nie wykryto MPU-6050! Zatrzymano start.");
    while (1) { 
      delay(100); // Pętla bez wyjścia - bezpieczny zawał procesora
    }
  }

  // 3. Jeśli czujnik odpowiedział (error == 0), wybudzamy go ze snu:
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Rejestr zasilania (PWR_MGMT_1)
  Wire.write(0);    // Wpisujemy 0, żeby go wybudzić
  Wire.endTransmission(true);

  Serial.println("[IMU] MPU-6050 gotowy do pracy.");
}

// ==========================================
// 7. Initial Angle
// ==========================================
void setInitialAngle() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)6, true); // Tu wystarczy tylko 6 bajtów akcelerometru!

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  // Ustawiamy od razu prawdziwy kąt startowy bez filtra 98%/2%:
  currentAngle = atan2((float)rawAccX, (float)rawAccZ) * 180.0 / PI;
}

// ==========================================
// 8. MPU-6050 SENSOR READ
// ==========================================
void readMPU(float &angle, float &gyroRate) {
  // Read raw data from accelerometer and gyroscope (registers starting from 0x3B)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)14, true);

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature registers
  int16_t rawGyroX = Wire.read() << 8 | Wire.read();
  int16_t rawGyroY = Wire.read() << 8 | Wire.read();
  int16_t rawGyroZ = Wire.read() << 8 | Wire.read();

// 1. Pomocniczy kąt tylko z samego akcelerometru:
  float accAngle = atan2((float)rawAccX, (float)rawAccZ) * 180.0 / PI;

  // 2. Prędkość kątowa z żyroskopu (w stopniach na sekundę):
  gyroRate = ((float)rawGyroY / 131.0) - gyroOffset;

  // 3. Filtr Komplementarny (98% żyroskop + 2% akcelerometr):
  angle = 0.98 * (angle + gyroRate * DT) + 0.02 * accAngle;
}


// ==========================================
// 9. SETUP (System Initialization)
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("==================================");
  Serial.println("--- BalanceCube Firmware Start ---");
  Serial.println("==================================");

  // Initialize I2C bus (pins from PCB)
  Wire.begin(PIN_IMU_SDA, PIN_IMU_SCL);

  // Configure digital pins
  pinMode(PIN_M1_DIR, OUTPUT);
  pinMode(PIN_M2_DIR, OUTPUT);
  pinMode(PIN_M3_DIR, OUTPUT);
  pinMode(PIN_ALL_BRAKE, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Release motor brake
  digitalWrite(PIN_ALL_BRAKE, LOW);

  // Configure hardware PWM (LEDC) for motors
  ledcAttach(PIN_M1_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M2_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M3_PWM, PWM_FREQ, PWM_RES);

  // Ensure motors are stopped at startup
  setMotor(1, 0, 0);
  setMotor(2, 0, 0);
  setMotor(3, 0, 0);

  // Initialize gyroscope
initMPU();
  delay(50);          // Dajemy obudzonemu czujnikowi 50 ms na zrobienie pierwszego pomiaru!

  calibrateGyro();

  setInitialAngle();  // Ustawiamy prawdziwy kąt początkowy
  lastLoopTime = micros();

  Serial.println("[SYSTEM] Configuration completed successfully. Starting control loop!");
}


// ==========================================
// 10. MAIN LOOP - 100 Hz
// ==========================================
void loop() {
  if (micros() - lastLoopTime >= LOOP_TIME_US) {
    lastLoopTime = micros(); // Zapisujemy aktualny czas na stoperze


    readMPU(currentAngle, currentGyroRate);
    balanceControl(currentAngle, currentGyroRate);
  }
}
