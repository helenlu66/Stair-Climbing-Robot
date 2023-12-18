#include <VL53L0X.h>
#include <Wire.h>
#include <JY901.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <string>

// set up server

const char *SSID = "Tufts_Robot";
const char *PWD = "";

WebServer server(80);
StaticJsonDocument<250> jsonDocument;
char buffer[250];

// Sensor setup
VL53L0X lidar;
int distance_threshold = 300;

int imuRx = 17;
int imuTx = 16;
struct GyroData {
  float gyro_x;
  float gyro_y;
  float gyro_z;
};

struct AngleData {
  float angle_x;
  float angle_y;
  float angle_z;
};

float angle_safety_threshold = 70.0;
float angle_flat_floor_threshold = 25;

float old_anlge = 0.0;
float old_angular_vel = 0.0;
float alpha = 0.9; // alpha for low-pass filter

// Front Motor 
int frontMotorPin1 = 27; 
int frontMotorPin2 = 26; 
int enable1Pin = 14;

// Back Motor
int backMotorPin1 = 32; 
int backMotorPin2 = 33; 
int backEnable1Pin = 25;

// Setting PWM properties
const int freq = 1000;
const int pwmChannel = 0;
const int resolution = 8;

void setup() {
  // set up sensors
   // WT901 baud, serial, rx, tx
  Serial1.begin(9600, SERIAL_8N1, imuRx, imuTx);
  Wire.begin();
  lidar.init();
  lidar.setAddress(0x29);  // Use the default address if not specified.
  lidar.setTimeout(0);
  lidar.startContinuous(); 
  // sets the pins as outputs:
  pinMode(frontMotorPin1, OUTPUT);
  pinMode(frontMotorPin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(backMotorPin1, OUTPUT);
  pinMode(backMotorPin2, OUTPUT);
  pinMode(backEnable1Pin, OUTPUT);
  
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(backEnable1Pin, pwmChannel);

  Serial.begin(115200);

  // connect to WIFI
  Serial.println("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());   
  setup_routing();

}

void setup_routing() {          
  server.on("/", HTTP_POST, handlePost);    
          
  server.begin();    
}

// read angles from WT901
// SENSOR READING
void read_wt901(){

  while (Serial1.available()) 
  {
    JY901.CopeSerialData(Serial1.read()); //Call JY901 data cope function
  }

}

GyroData readAngularRates1k() {
  GyroData gyroValues;
  gyroValues.gyro_x = ((float)JY901.stcGyro.w[0] / 32768 * 2000);
  gyroValues.gyro_y = ((float)JY901.stcGyro.w[1] / 32768 * 2000);
  gyroValues.gyro_z = ((float)JY901.stcGyro.w[2] / 32768 * 2000);

  //Serial.print("Gyro:");Serial.print(gyroValues.gyro_x, 4); Serial.print(","); Serial.print(gyroValues.gyro_y, 4); Serial.print(","); Serial.println(gyroValues.gyro_z, 4); 

  return gyroValues;
}

// read angles from WT901
AngleData readAngles1k(){
  AngleData angleValues;

  // output of angle readings (look at the example serial file in the JY901 library for some other outputs if desired (like accel or gyro) but probably you just need the angle)
  angleValues.angle_x = (((float)JY901.stcAngle.Angle[0] / 32768 * 180));
  angleValues.angle_y = (((float)JY901.stcAngle.Angle[1] / 32768 * 180));
  angleValues.angle_z = (((float)JY901.stcAngle.Angle[2] / 32768 * 180));


  // output the desired angles 
  Serial.print("Angles:");Serial.print(angleValues.angle_x, 4); Serial.print(","); Serial.print(angleValues.angle_y, 4); Serial.print(","); Serial.println(angleValues.angle_z, 4); 
  return angleValues;
}


float lowPassFilter(float new_val, float old_val){
  return alpha * new_val + (1 - alpha) * old_val;
}

void stop_front_motor() {
  Serial.println("Stopping the front motors");
  digitalWrite(frontMotorPin1, LOW);
  digitalWrite(frontMotorPin2, LOW);
  digitalWrite(backMotorPin1, LOW);
  digitalWrite(backMotorPin2, LOW);
}

void drive_front_motor_forward(int speed){
  // Move the DC motor forward at speed for 2 seconds
  Serial.print("Moving Forward front motor at speed ");
  Serial.println(speed);
  ledcWrite(pwmChannel, speed);
  digitalWrite(frontMotorPin1, LOW);
  digitalWrite(frontMotorPin2, HIGH);
  digitalWrite(backMotorPin1, LOW);
  digitalWrite(backMotorPin2, HIGH); 
  delay(500);
}

void drive_front_motor_backward(int speed){
  // Move the DC motor backward at speed for 2 seconds
  Serial.print("Moving backward front motor at speed ");
  Serial.println(speed);
  ledcWrite(pwmChannel, speed);
  digitalWrite(frontMotorPin1, HIGH);
  digitalWrite(frontMotorPin2, LOW);
  digitalWrite(backMotorPin1, HIGH);
  digitalWrite(backMotorPin2, LOW);
  delay(500);
}

void handlePost(){
  if (server.hasArg("plain") == false) {
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  String command = jsonDocument["command"];
  Serial.println(command);
  String action = actBasedOnState(command);

  // Create a JSON document for the response to the client
  DynamicJsonDocument responseDoc(200);
  responseDoc["executed action"] = action;
  // Convert the JSON document to a string
  serializeJson(responseDoc, buffer);

  // Send the JSON response back to the client
  server.send(200, "application/json", buffer);
}

String actBasedOnState(String command) {

  // process the command
  if (command == "forward") {
      // read sensor data to determine state
    int distance = lidar.readRangeContinuousMillimeters();
    Serial.print("lidar reading: ");
    Serial.println(distance);
    read_wt901();
    AngleData angleData = readAngles1k();

    if (distance >= distance_threshold & angleData.angle_x <= angle_flat_floor_threshold) { // move fast when far from the stairs
      drive_front_motor_forward(1000);
      return "fast forward";
    } else if (distance < distance_threshold & angleData.angle_x <= angle_flat_floor_threshold) { // move slow when approaching stairs
      drive_front_motor_forward(150);
      return "slow forward";
    } else if (angleData.angle_x > angle_flat_floor_threshold & angleData.angle_x <= angle_safety_threshold) { // move fast when on the stairs
      drive_front_motor_forward(1000);
      return "fast forward";
    } else if (angleData.angle_x > angle_safety_threshold) {
      // check if current angle is in the danger zone after moving forward
      drive_front_motor_backward(1000);
      return "backward";
    }

  } else if (command == "stop") {
    stop_front_motor();
    return "stop";
  } else if (command == "backward") {
    drive_front_motor_backward(1000);
    return "backward";
  } else {
    Serial.println("unrecognized command!");
  }
  
}

void loop() {
  server.handleClient();
}