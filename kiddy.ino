#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>
#include <Servo.h>
#include <ESP32Firebase.h>

// Define for Servo
#define SERVO_PIN 27 // pin yang sesuai pada ESP32
#define ON 1
#define OFF 0

// Define for GPS
#define GPS_BAUDRATE 9600  // The default baudrate of NEO-6M is 9600

// Define for Firebase
#define REFERENCE_URL "https://kiddy-65de6-default-rtdb.asia-southeast1.firebasedatabase.app"  // Firebase project reference url

// Define Serial Number
String SERIAL_NUMBER = "Kiddy12345678";

// SSID/Password Wifi
const char* ssid = "Galaxy";
const char* password = "12571257";


// Object GPS
unsigned long lastMsg = 0;
TinyGPSPlus gps; 

// Objek servo
Servo myServo;
int degree = 90;
int state = ON;

// Objek Firebase
Firebase firebase(REFERENCE_URL);

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE);

  // Attach servo ke pin
  myServo.attach(SERVO_PIN);
  
  // Connect to wifi
  setupWifi();
}

void setupWifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int getIsSwing() {
  String path = "devices/" + SERIAL_NUMBER + "/isSwing";
  int isSwing = firebase.getInt(path);
  Serial.print("State motor servo: ");
  Serial.println(isSwing);

  Serial.print("Motor servo path: ");
  Serial.println(path);
  return isSwing;
}

void servoLoop() {
  if (getIsSwing() == ON) {
    if (degree == 90-45 || degree == 90) {
      while (degree < 90+45) {
        degree++;
        myServo.write(degree);
        delay(20);
      }
    } else if (degree == 90+45) {
      while (degree > 90-45) {
        degree--;
        myServo.write(degree);
        delay(20);
      }
    }
  } else {
    // Agar berhenti di tengah
    if (degree < 90) {
      while (degree < 90) {
        degree++;
        myServo.write(degree);
        delay(20);
      }
    } else {
       while (degree > 90) {
        degree--;
        myServo.write(degree);
        delay(20);
      }
    }
  }
}

void gpsLoop() {
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // Read GPS data
    if (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid()) {
          Serial.print(F("latitude: "));
          Serial.println(gps.location.lat());

          Serial.print(F("longitude: "));
          Serial.println(gps.location.lng());

          Serial.print(F("altitude: "));
          if (gps.altitude.isValid())
            Serial.println(gps.altitude.meters());
          else
            Serial.println(F("INVALID"));

          Serial.print(F("speed: "));
          if (gps.speed.isValid()) {
            Serial.print(gps.speed.kmph());
            Serial.println(F(" km/h"));
          } else {
            Serial.println(F("INVALID"));
          }

          // Push GPS data to Firebase
          String path = "devices/" + SERIAL_NUMBER + "/latitude";
          firebase.setFloat(path, gps.location.lat());

          path = "devices/" + SERIAL_NUMBER + "/longitude";
          firebase.setFloat(path, gps.location.lng());

          if (gps.altitude.isValid())
            path = "devices/" + SERIAL_NUMBER + "/altitude";
            firebase.setFloat(path, gps.altitude.meters());

          if (gps.speed.isValid())
            path = "devices/" + SERIAL_NUMBER + "/speed";
            firebase.setFloat(path, gps.speed.kmph());
        }
      }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println(F("No GPS data received: check wiring"));
    }
  }
}

void loop() {
  servoLoop();
  gpsLoop();
}
