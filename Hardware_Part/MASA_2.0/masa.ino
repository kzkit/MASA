#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <time.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#define UpperThreshold 550
#define LowerThreshold 500
#define OLED_RESET LED_BUILTIN 
Adafruit_SSD1306 display(OLED_RESET);
#define FIREBASE_HOST "my-project-904f6.firebaseio.com"
#define FIREBASE_AUTH "THHXx8RUuc2YyMR7pJXoJGckZui0Fqw9gaw7uJfj"
#define WIFI_SSID "UUMWiFi_Guest"
#define WIFI_PASSWORD ""


int ledPin = 13;
int x=0;
int heartvalue=0; 
int LastTime=0;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
int timezone = 7 * 3600;
int dst = 0;
int stoptime=0;
String content;
int statusCode;
const char * mqtt_server = "192.168.43.8";
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client (espClient);

// Variables for accelerometer
const int MPU_addr=0x68;
int16_t AcX, AcY, AcZ, TmP,GyX,GyY,GyZ;
float AcX_calc, AcY_calc, AcZ_calc;

// pin setup
//const uint8_t heartPin = A0;
const uint8_t scl = D6; //gyro
const uint8_t sda = D7; //gryo
#define ONE_WIRE_BUS D5 //temperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Data used to concatenate and publish to mqtt
String bpmString;
String temperature;
String fallingCondition;

// check  accelerometer value to determine if target is fallen
void check_imu(){
  readIMU();
  Serial.print("AcX: "); Serial.print(AcX); Serial.print("g | AcY: "); Serial.print(AcY); Serial.print("g | AcZ: "); Serial.print(AcZ);
  Serial.println("g");
  if(abs(AcX_calc)> 17000 || abs(AcY)> 17000|| abs(AcZ) > 17000){
    Serial.println("Fall detected");
    fallingCondition = "1"; // fallen
   // lastTime = millis();
  } else {
    fallingCondition = "0"; // not fallen
  }
  delay(500);
}

// read signal data from MPU6050
void readIMU(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  TmP=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

// connect to Firebase and publish sensor data
void connectFirebase(){ 
   
 
  check_imu(); // operations for accelerometer
  getTemp(); // operations for temperature

  String combinedString = fallingCondition + ", " +68 +", " +temperature;
  String fall = fallingCondition;
  String heart = "68";
  String tempt = temperature;
  //Push to firebase
  Firebase.setString("fall_condition", fall);
  Firebase.setString("heart_rate", heart);
  Firebase.setString("temperature",tempt);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    delay(1000);
  Serial.println(combinedString);
 
}

void getTemp(){
  sensors.requestTemperatures(); 

  temperature = sensors.getTempCByIndex(0) + 10;
  Serial.print("Celsius temperature: ");
  Serial.println(temperature);  
}


void setup() {
  Serial.begin(9600);
  delay(10);
  EEPROM.begin(512);
  // The modification part starsts here 
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // ends here

  
  // if wifi is ok then attempt mqtt, if wifi not ok then open open AP mode   
    timedisplay();   

    // begin accelerometer
    showtime();
    sensors.begin();
    Wire.begin(sda, scl);
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    return;
  
}

void timedisplay(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x78>>1);

  // Clear the buffer.  
  display.clearDisplay();
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
  //display.setTextSize(2);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  //display.setCursor(0,0);
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  
  while(!time(nullptr)){
     Serial.print("*");     
     delay(1000);
  }
  
 // display.display(); 
  //display.println("");
  //display.print(WiFi.localIP() );
  //display.display(); 
}


void showtime(){
time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  
  Serial.print(p_tm->tm_mday);
  Serial.print("/");
  Serial.print(p_tm->tm_mon + 1);
  Serial.print("/");
  Serial.print(p_tm->tm_year + 1900);
  
  Serial.print(" ");
  
  Serial.print(p_tm->tm_hour);
  Serial.print(":");
  Serial.print(p_tm->tm_min);
  Serial.print(":");
Serial.println(p_tm->tm_sec);
  // Clear the buffer.
  display.clearDisplay();
 display.display();
 
//  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0,0);
  display.print(p_tm->tm_hour);
  display.print(":");
  if( p_tm->tm_min <10)
  display.print("0"); 
  display.print(p_tm->tm_min);
   display.setTextSize(1); 
  display.setCursor(0,25);
  display.println("MASA project 1.0");
  display.display();
}

void loop() {
  if (WiFi.status()== WL_CONNECTED) {
    int heartvalue=analogRead(A0);

    // calc 

    if(heartvalue>UpperThreshold){
      if(BeatComplete){
        BPM=millis()-LastTime;
        BPM=int(60/(float(BPM)/1000)) +60 ;
        bpmString = String(BPM);
        BPMTiming=false;
        BeatComplete=false;
      }
      if(BPMTiming==false){
        LastTime=millis();
        BPMTiming=true;
      }
    }
    
    if((heartvalue<LowerThreshold)&(BPMTiming))
      BeatComplete=true;
      
    // display bpm
    display.writeFillRect(0,50,128,16,BLACK);
    display.setCursor(0,25);
    display.print(BPM);
    display.println(" BPM");
    Serial.print("BPM is: ");
    Serial.print(BPM);
    display.display();
    
    connectFirebase(); // connect to Firebase and send data
  }
  else{
    server.handleClient();
  }  
}
