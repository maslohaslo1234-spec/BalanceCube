/*
 * BalanceCube - Reaction Wheel Balancing Cube Firmware (Baseline)
 * Microcontroller: ESP32 DevKit 30-pin
 * Actuators: 3x Nidec 24H Brushless DC Motors
 * Sensor: MPU-6050 (GY-521)
 *
 */

#include <Wire.h>

// ==========================================
// 1. PCB PIN DEFINITIONS
// ==========================================

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

// MPU-6050 sensor presence flag
bool imuConnected = false;


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
// 5. MPU-6050 SENSOR INITIALIZATION
// ==========================================
void initMPU() {
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();

  if (error == 0) {
    // Sensor detected - wake it up from sleep (write 0 to power management register 0x6B)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    imuConnected = true;
    Serial.println("[IMU] MPU-6050 detected and woken up successfully!");
  } else {
    imuConnected = false;
    Serial.println("[IMU] No physical MPU-6050 sensor on I2C (starting simulation mode)");
  }
}


// ==========================================
// 6. MPU-6050 SENSOR READ
// ==========================================
void readMPU(float &angle, float &gyroRate) {
  if (imuConnected) {
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

    // Convert gravity to tilt angle (in degrees)
    angle = atan2((float)rawAccX, (float)rawAccZ) * 180.0 / PI;

    // Convert raw gyro data to degrees per second (divisor 131.0 for 250 dps scale)
    gyroRate = (float)rawGyroY / 131.0;
  } else {
    // Dry test mode (generate small tilt for loop testing)
    static float fakeTime = 0;
    fakeTime += 0.05;
    angle = sin(fakeTime) * 5.0; // Virtual swing +/- 5 degrees
    gyroRate = cos(fakeTime) * 10.0;
  }
}


// ==========================================
// 7. SETUP (System Initialization)
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

  Serial.println("[SYSTEM] Configuration completed successfully. Starting control loop!");
}


// ==========================================
// 8. MAIN LOOP - 100 Hz
// ==========================================
void loop() {
  float currentAngle = 0.0;
  float currentGyroRate = 0.0;

  // 1. Read angle and angular velocity
  readMPU(currentAngle, currentGyroRate);

  // 2. Calculate PD controller response and control the motor
  balanceControl(currentAngle, currentGyroRate);

  // 3. Data preview in Serial Monitor
  Serial.print("Angle: ");
  Serial.print(currentAngle);
  Serial.print(" deg | Angular Velocity: ");
  Serial.print(currentGyroRate);
  Serial.println(" deg/s");

  delay(10); // Loop runs at approx. 100 Hz (every 10 ms)
}
