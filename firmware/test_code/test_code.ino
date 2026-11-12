#define PIN_IMU_SDA    21
#define PIN_IMU_SCL    22

#define PIN_M1_PWM     32
#define PIN_M1_DIR      4
#define PIN_M1_ENC_A   35
#define PIN_M1_ENC_B   33

#define PIN_M2_PWM     25
#define PIN_M2_DIR     15
#define PIN_M2_ENC_A   13
#define PIN_M2_ENC_B   14

#define PIN_M3_PWM     18
#define PIN_M3_DIR      5
#define PIN_M3_ENC_A   16
#define PIN_M3_ENC_B   17

#define PIN_ALL_BRAKE  26


const int PWM_FREQ = 20000;    
const int PWM_RES  = 8;       



void setMotor(int motorId, int speed, int dir) {

  speed = constrain(speed, 0, 255);

  if (motorId == 1) {
    digitalWrite(PIN_M1_DIR, dir); 
    ledcWrite(PIN_M1_PWM, speed);   
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



void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("--- BalanceCube Start! ---");


  pinMode(PIN_M1_DIR, OUTPUT);
  pinMode(PIN_M2_DIR, OUTPUT);
  pinMode(PIN_M3_DIR, OUTPUT);
  pinMode(PIN_ALL_BRAKE, OUTPUT);


  digitalWrite(PIN_ALL_BRAKE, LOW);

  ledcAttach(PIN_M1_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M2_PWM, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_M3_PWM, PWM_FREQ, PWM_RES);

  setMotor(1, 0, 0);
  setMotor(2, 0, 0);
  setMotor(3, 0, 0);

  Serial.println("Silniki i generatory PWM 20kHz gotowe do pracy.");
}



void loop() {

  
  Serial.println("Test: Silnik 1 - Prędkość 25% (Prawo)");
  setMotor(1, 64, 1);
  delay(2000);

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
