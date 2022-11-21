// library for I2C devices
#include "Wire.h" 

// variables for ultrasonic sensors 
int trigPinMid = 9;      
int echoPinMid = 8;     
int trigPinRight = A0;
int echoPinRight = A1;
int trigPinLeft = A2;
int echoPinLeft = A3;

// measured distance by three ultrasonic sensors
long durationMid = 0; 
long distanceMid = 0;
long durationRight = 0; 
long distanceRight = 0;
long durationLeft = 0; 
long distanceLeft = 0;

// vars for buttons
const int buttonPin = 3;     
int button = 0; // is pressed?

// vars for motors
int backwardRightMotor = 4;       
int forwardRightMotor = 5;        
int backwardLeftMotor = 6;        
int forwardLeftMotor = 7;         
int speedLeftMotor = 10;
int speedRightMotor = 11;

// vars for gyro
const int MPU_ADDR = 0x68; // I2C address MPU-6050
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t temperature; // temperature
char tmp_str[7]; // help variable for conversion

// vars for leds
int ledZ = 12;
int ledC = 13;

// convert int16 to string
char* convert_int16_to_str(int16_t i) { 
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

// void setup is called after microcontroller start up
void setup() {
  Serial.begin (9600);
  
  // gyro config
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0); 
  Wire.endTransmission(true);
  
  // ultrasonic sensors pins config
  pinMode(trigPinMid, OUTPUT);
  pinMode(echoPinMid, INPUT);
  pinMode(trigPinRight, OUTPUT);
  pinMode(echoPinRight, INPUT);
  pinMode(trigPinLeft, OUTPUT);
  pinMode(echoPinLeft, INPUT);
  
  // led config
  pinMode(ledZ, OUTPUT);
  pinMode(ledC, OUTPUT);
  
  // button pin config
  pinMode(buttonPin, INPUT);
  
  // motor pins config
  pinMode(backwardRightMotor, OUTPUT);      
  pinMode(forwardRightMotor, OUTPUT);
  pinMode(backwardLeftMotor, OUTPUT);
  pinMode(forwardLeftMotor, OUTPUT);
  pinMode(speedLeftMotor, OUTPUT);
  pinMode(speedRightMotor, OUTPUT);
}

void loop() {
  // gets slope data, stored in global vars, if the device is tilted, motors stop
  getSlope();

  // gets distance from all three ultrasonic sensors, stored in global vars
  getDistance();
  
  // get button pin value (1 if pressed, else 0)
  button = digitalRead(buttonPin);  

  // test conditions for move functions (distance in cm, maximum slope +- 30)
  if (accelerometer_x > 9000 or accelerometer_y > 9000 or accelerometer_x < -9000 or accelerometer_y < -9000 or accelerometer_z < 0) {
    // device is tilted, stopping
    stopMotor();
  } else if ((distanceMid < 25 and distanceLeft >= 25 and distanceLeft >= 25) or button == 1) {
    // there is an obstacle, seen by mid sensor, or the device bumped into it and the button was pressed
    stopMotor();  
    goBackward();
    delay(100);
    turnAround(); 
    delay(200);
  } else if (distanceLeft < 25) {
    // there is an obstacle on the left side
    stopMotor();
    delay(20);
    turnRight();
    delay(20);  
  } else if (distanceRight < 25) {
    // there is an obstacle on the right side
    stopMotor();
    delay(20);
    turnRight();
    delay(20);    
  } else if (distanceMid >= 25 and distanceLeft >= 25 and distanceRight >= 25 and button == 0) {
    // no obstacle, can go forward
    goForward();  
  }
  // do this every 50 miliseconds
  delay(50);
}

void getSlope() {
  // https://mschoeffler.com/2017/12/09/example-application-gy-521-module-mpu-6050-breakout-board-and-arduino-uno/
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) a 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) a 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) a 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) a 0x42 (TEMP_OUT_L)

  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
  Serial.println();
}

void getDistance() {
  // https://www.instructables.com/Using-a-SR04/
  digitalWrite(trigPinMid, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinMid, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinMid, LOW);
  durationMid = pulseIn(echoPinMid, HIGH);
  distanceMid = (durationMid/2) / 29.1;
  
  digitalWrite(trigPinRight, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinRight, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinRight, LOW);
  durationRight = pulseIn(echoPinRight, HIGH);
  distanceRight = (durationRight/2) / 29.1;
  
  digitalWrite(trigPinLeft, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinLeft, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinLeft, LOW);
  durationLeft = pulseIn(echoPinLeft, HIGH);
  distanceLeft = (durationLeft/2) / 29.1;
  
  Serial.print("dS = "); Serial.print(distanceMid);
  Serial.print(" | dP = "); Serial.print(distanceRight);
  Serial.print(" | dL = "); Serial.print(distanceLeft);
  Serial.println(); 
}

void goBackward() {
  Serial.println(" goint back ");
  
  // signals to motor controller, left back and right back
  digitalWrite(backwardLeftMotor, HIGH);              
  digitalWrite(forwardLeftMotor, LOW);
  digitalWrite(backwardRightMotor, HIGH);                                
  digitalWrite(forwardRightMotor, LOW);
  
  // signal with leds
  digitalWrite(ledC, HIGH);
  digitalWrite(ledZ, HIGH);
  
  // do this for 1/10 second
  delay(100);
}

void turnLeft() { 
  Serial.println(" turning slightly left ");
  
  // send PWM signal to motors - (pin number, speed 0-255)
  analogWrite(speedLeftMotor, 255);
  analogWrite(speedRightMotor, 255);
  
  // signals to motor controller, left back and right forward
  digitalWrite(backwardLeftMotor, HIGH);               
  digitalWrite(forwardLeftMotor, LOW);
  digitalWrite(backwardRightMotor, LOW);                                
  digitalWrite(forwardRightMotor, HIGH);
  
  // signal with leds
  digitalWrite(ledC, HIGH);
  digitalWrite(ledZ, HIGH);
  
  // do this for 1/10 second
  delay(100);
  stopMotor();
}

void turnRight() { 
  Serial.println(" turning slightly right ");
  
  // send PWM signal to motors - (pin number, speed 0-255)
  analogWrite(speedLeftMotor, 255); 
  analogWrite(speedRightMotor, 255);
  
  // signals to motor controller, left forward and right back
  digitalWrite(backwardLeftMotor, LOW);
  digitalWrite(forwardLeftMotor, HIGH);
  digitalWrite(backwardRightMotor, HIGH);                                
  digitalWrite(forwardRightMotor, LOW);
  
  // signal with leds
  digitalWrite(ledC, HIGH);
  digitalWrite(ledZ, HIGH);
  
  // do this for 1/10 second
  delay(100);
  stopMotor();
}

void turnAround() { 
  Serial.println(" turning around ");
  
  // send PWM signal to motors - (pin number, speed 0-255)
  analogWrite(speedLeftMotor, 255); 
  analogWrite(speedRightMotor, 255);
  
  // signals to motor controller, left forward and right back
  digitalWrite(backwardLeftMotor, LOW);
  digitalWrite(forwardLeftMotor, HIGH);
  digitalWrite(backwardRightMotor, HIGH);                                
  digitalWrite(forwardRightMotor, LOW);
  
  // signal with leds
  digitalWrite(ledC, HIGH);
  digitalWrite(ledZ, HIGH);
  
  // do this for 350 - 700 miliseconds, the device is turning with added random element
  delay(random(350, 700));
  stopMotor();
}

void stopMotor() { 
  Serial.println(" stopMotor ");
  
  // signals to motor controller, nothing
  digitalWrite(forwardLeftMotor, LOW);
  digitalWrite(backwardLeftMotor, LOW);
  digitalWrite(forwardRightMotor, LOW);                                
  digitalWrite(backwardRightMotor, LOW);
  
  // signal with leds
  digitalWrite(ledC, HIGH);
  digitalWrite(ledZ, LOW);
}

void goForward() { 
  Serial.println(" going forward ");
  
  // send PWM signal to motors - (pin number, speed 0-255)
  analogWrite(speedLeftMotor, 100); 
  analogWrite(speedRightMotor, 100); 
  
  // signals to motor controller, left forward and right forward
  digitalWrite(forwardLeftMotor, HIGH);
  digitalWrite(backwardLeftMotor, LOW);
  digitalWrite(forwardRightMotor, HIGH);                                
  digitalWrite(backwardRightMotor, LOW);
  
  // signal with leds
  digitalWrite(ledC, LOW);
  digitalWrite(ledZ, HIGH);
}