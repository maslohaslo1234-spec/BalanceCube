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

#define PIN_LED 2    // Built-in blue LED on the ESP32 board

// I2C pins for MPU6050 gyroscope
#define PIN_IMU_SDA     21
#define PIN_IMU_SCL     22
#define MPU_ADDR        0x68

// MOTOR 1 (Connector J3 on PCB)
#define PIN_M1_PWM      32
#define PIN_M1_DIR       4
#define PIN_M1_ENC_A    35
#define PIN_M1_ENC_B    33

// MOTOR 2 (Connector J4 on PCB)
#define PIN_M2_PWM      25
#define PIN_M2_DIR      15
#define PIN_M2_ENC_A    13
#define PIN_M2_ENC_B    14

// MOTOR 3 (Connector J5 on PCB)
#define PIN_M3_PWM      18
#define PIN_M3_DIR       5
#define PIN_M3_ENC_A    16
#define PIN_M3_ENC_B    17

// SHARED BRAKE FOR ALL MOTORS
#define PIN_ALL_BRAKE   26

// ==========================================
// 2. HARDWARE PARAMETERS & PD GAINS
// ==========================================

const int PWM_FREQ = 20000;    // 20 kHz - quiet motor operation (outside audible range)
const int PWM_RES  = 8;        // 8-bit resolution (speed range 0 - 255)

// PD controller tuning parameters (initial values, to be tuned on the assembled cube)
float Kp = 15.0;  // Proportional reaction force to tilt angle
float Kd = 1.2;   // Damping force (damper) reaction to rotational speed

unsigned long lastLoopTime = 0;          // When the loop was last executed
const unsigned long LOOP_TIME_US = 10000; // How often the loop should run in microseconds (10,000 us = 10 ms = 100 Hz)

const float DT = LOOP_TIME_US / 1000000.0; // i.e., 0.01 seconds

float currentAngle = 0.0;
float currentGyroRate = 0.0;

bool isArmed = false;             // Is the cube armed? FALSE at startup
unsigned long uprightTimer = 0;   // Stopwatch tracking how long the cube has been close to vertical

float gyroOffset = 0.0; // Here we will store our calculated zero offset

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

  // STATE 1: Cube is not armed yet (waiting for vertical position for 1 second)
  if (isArmed == false) {
    setMotor(1, 0, 0); // The motor must definitely stop!

    if (abs(angle) < 10.0) {
      // If you are holding it close to vertical, check if 1 second (1000 ms) has passed:
      if (millis() - uprightTimer >= 1000) {
        isArmed = true; // ARMED!
        Serial.println("[SYSTEM] Cube armed! Starting to balance.");
      }
    } else {
      // If it is out of vertical (>= 10 degrees), continuously reset the timer:
      uprightTimer = millis();
    }

    return; // Exit the function so PD is not calculated until armed
  }

  // STATE 2: Cube IS armed, but let's check if it has fallen
  if (abs(angle) > 35.0) {
    setMotor(1, 0, 0);
    isArmed = false;           // Disarming! You must hold it vertical for 1s again
    uprightTimer = millis(); // Reset the timer
    Serial.println("[SYSTEM] Fall detected! Motors disarmed.");
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
// 5. GYROSCOPE CALIBRATION
// ==========================================
void calibrateGyro() {
  digitalWrite(PIN_LED, HIGH); // Turn ON LED: "Do not move the cube!"
  Serial.println("[IMU] Place the cube still - wait 2 seconds for calibration...");
  
  delay(2000); // 2 seconds to calmly plug in XT30 and place it on the table

  float sum = 0.0;
  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x45); // 0x45 is the register address of the Y-axis gyroscope itself!
    Wire.endTransmission(false);
    Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)2, true); // Take only 2 bytes of Y axis

    int16_t rawGyroY = Wire.read() << 8 | Wire.read();
    sum += (float)rawGyroY / 131.0; // Accumulate the measurement

    delay(3); // Short 3 ms break between measurements
  }

  gyroOffset = sum / 200.0;
  digitalWrite(PIN_LED, LOW); // Turn OFF LED: "Ready! You can place the cube."
  Serial.print("[IMU] Calibrated! Offset: ");
  Serial.println(gyroOffset);
}


// ==========================================
// 6. MPU-6050 SENSOR INITIALIZATION
// ==========================================
void initMPU() {
  // 1. Knock on the sensor at address 0x68
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();

  // 2. Check if the sensor responded:
  if (error != 0) {
    Serial.println("[ERROR] MPU-6050 not detected! Startup halted.");
    while (1) { 
      delay(100); // Infinite loop - safe processor freeze
    }
  }

  // 3. If the sensor responded (error == 0), wake it up from sleep:
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management register (PWR_MGMT_1)
  Wire.write(0);    // Write 0 to wake it up
  Wire.endTransmission(true);

  Serial.println("[IMU] MPU-6050 ready for operation.");
}

// ==========================================
// 7. Initial Angle
// ==========================================
void setInitialAngle() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)6, true); // Only 6 bytes of accelerometer are needed here!

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  // Set the actual starting angle immediately without the 98%/2% filter:
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

  // 1. Auxiliary angle from the accelerometer alone:
  float accAngle = atan2((float)rawAccX, (float)rawAccZ) * 180.0 / PI;

  // 2. Angular velocity from the gyroscope (in degrees per second):
  gyroRate = ((float)rawGyroY / 131.0) - gyroOffset;

  // 3. Complementary filter (98% gyroscope + 2% accelerometer):
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
  delay(50);          // Give the awakened sensor 50 ms to make its first measurement!

  calibrateGyro();

  setInitialAngle();  // Set the actual initial angle
  lastLoopTime = micros();

  Serial.println("[SYSTEM] Configuration completed successfully. Starting control loop!");
}


// ==========================================
// 10. MAIN LOOP - 100 Hz
// ==========================================
void loop() {
  if (micros() - lastLoopTime >= LOOP_TIME_US) {
    lastLoopTime = micros(); // Save the current time on the stopwatch

    readMPU(currentAngle, currentGyroRate);
    balanceControl(currentAngle, currentGyroRate);
  }
}
