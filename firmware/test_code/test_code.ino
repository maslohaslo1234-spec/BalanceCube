/*
 * BalanceCube - Reaction Wheel Balancing Cube
 * Controller: ESP32 DevKit 30-pin
 * Actuators: 3x Nidec 24H Brushless Motors
 * Sensor: MPU-6050 (GY-521)
 */

// ==========================================
// 1. DEFINICJE PINÓW Z TWOJEGO SCHEMATU PCB
// ==========================================

#define PIN_IMU_SDA    21
#define PIN_IMU_SCL    22

// SILNIK 1 (Złącze J3)
#define PIN_M1_PWM     32
#define PIN_M1_DIR      4
#define PIN_M1_ENC_A   35
#define PIN_M1_ENC_B   33

// SILNIK 2 (Złącze J4)
#define PIN_M2_PWM     25
#define PIN_M2_DIR     15
#define PIN_M2_ENC_A   13
#define PIN_M2_ENC_B   14

// SILNIK 3 (Złącze J5)
#define PIN_M3_PWM     18
#define PIN_M3_DIR      5
#define PIN_M3_ENC_A   16
#define PIN_M3_ENC_B   17

#define PIN_ALL_BRAKE  26

// Parametry generatora PWM
const int PWM_FREQ = 20000;    // 20 kHz - niesłyszalna dla ucha częstotliwość
const int PWM_RES  = 8;        // Rozdzielczość 8 bitów (wartości prędkości od 0 do 255)


// ==========================================
// 2. FUNKCJA STEROWANIA SILNIKIEM
// ==========================================
// motorId: 1, 2 lub 3
// speed: 0 (stop) do 255 (maksymalne obroty)
// dir: 1 (prawo / CW) lub 0 (lewo / CCW)
void setMotor(int motorId, int speed, int dir) {
  // Zabezpieczenie: ograniczamy prędkość do zakresu 0 - 255
  speed = constrain(speed, 0, 255);

  if (motorId == 1) {
    digitalWrite(PIN_M1_DIR, dir);   // Ustawiamy stan na pinie kierunku
    ledcWrite(PIN_M1_PWM, speed);    // Wystawiamy sygnał PWM o danej mocy
  } 
  else if (motorId == 2) {
    digitalWrite(PIN_M2_DIR, dir);
    ledcWrite(PIN_M2_PWM, speed);
  } 
  else if (motorId == 3) {
    digitalWrite(PIN_M3_DIR, dir);
    ledcWrite(PIN_M3_PWM, speed);
  }
}


// ==========================================
// 3. SETUP (Inicjalizacja)
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("--- BalanceCube Start! ---");

  // 1. Ustawienie pinów kierunku i hamulca jako wyjścia
  pinMode(PIN_M1_DIR, OUTPUT);
  pinMode(PIN_M2_DIR, OUTPUT);
  pinMode(PIN_M3_DIR, OUTPUT);
  pinMode(PIN_ALL_BRAKE, OUTPUT);

  // Zwalniamy hamulec
  digitalWrite(PIN_ALL_BRAKE, LOW);

  // 2. Konfiguracja sprzętowego PWM (LEDC) dla 3 pinów silników
  // W nowym standardzie ESP32 przypisujemy pin bezpośrednio do częstotliwości i rozdzielczości:
  ledcAttach(PIN_M1_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M2_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M3_PWM, PWM_FREQ, PWM_RES);

  // Na starcie upewniamy się, że wszystkie silniki stoją (prędkość 0)
  setMotor(1, 0, 0);
  setMotor(2, 0, 0);
  setMotor(3, 0, 0);

  Serial.println("Silniki i generatory PWM 20kHz gotowe do pracy.");
}


// ==========================================
// 4. PĘTLA GŁÓWNA - TEST LOGICZNY
// ==========================================
void loop() {
  // Przeprowadzamy "wirtualny test" sterowania na sucho:
  
  Serial.println("Test: Silnik 1 - Prędkość 25% (Prawo)");
  setMotor(1, 64, 1);
  delay(2000); // czekamy 2 sekundy

  Serial.println("Test: Silnik 1 - Prędkość 75% (Prawo)");
  setMotor(1, 192, 1);
  delay(2000);

  Serial.println("Test: Silnik 1 - Zmiana kierunku (Lewo 50%)");
  setMotor(1, 128, 0);
  delay(2000);

  Serial.println("Test: Zatrzymanie wszystkich silników");
  setMotor(1, 0, 0);
  setMotor(2, 0, 0);
  setMotor(3, 0, 0);
  delay(3000);
}