#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>

// QMC5883L Compass Library
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h".



#define REPORTING_PERIOD_MS 1000

#define WIFI_SSID "Slt fiber zz"
#define WIFI_PASSWORD "gl36185gl"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB_Sah0jQumAI5CnGVpC1W8HPXW_qnrgKs"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://digital-compass-4725d-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
unsigned long sendDataPrevMillis = 0;


const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
int glucose = 0;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define sensor A0
#define RED1 D3
//#define RED2 D4
#define IR D6

void setup() {

  Serial.begin(115200);
  Serial.println("Initializing...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  // Display static text
  display.println("Welcome to");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  // Display static text
  display.println("DiaBeta");

  int count = 0;
  float bar = 0;
  while (count < 100) {

    if (count == 40) {

      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(30, 10);
      // Display static text
      display.println("Connecting to");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(60, 30);
      // Display static text
      display.println(WIFI_SSID);
      display.drawRect(0, 50, 120, 10, WHITE);
      display.fillRect(2, 52, bar , 6, WHITE);
      display.display();

      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());


      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(30, 10);
      // Display static text
      display.println("Connected");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 30);
      // Display static text
      display.println(WiFi.localIP());
      display.drawRect(0, 50, 120, 10, WHITE);
      display.fillRect(2, 52, bar , 6, WHITE);
      display.display();
      /* Assign the api key (required) */
      config.api_key = API_KEY;

      /* Assign the RTDB URL (required) */
      config.database_url = DATABASE_URL;

      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(30, 25);
      // Display static text
      display.println("Connecting to");
      display.setCursor(50, 40);
      // Display static text
      display.println("Database");
      display.drawRect(0, 50, 120, 10, WHITE);
      display.fillRect(2, 52, bar , 6, WHITE);
      display.display();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 30);
      // Display static text

      // Initialize the Compass.
      compass.init();

      /* Sign up */
      if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
      }
      else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
      }

      if (signupOK) {
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(30, 10);
        // Display static text
        display.println("Welcome to");
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 30);
        // Display static text
        display.println("DiaBeta");
      } else {
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(30, 10);
        // Display static text
        display.println("Database connction failed");
      }

      /* Assign the callback function for the long running token generation task */
      config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);


    }

    display.drawRect(0, 50, 120, 10, WHITE);
    display.fillRect(2, 52, bar , 6, WHITE); // initailize the graphics fillRect(int x, int y, int width, int height)
    delay(1);
    bar += 1.2;
    count++;
    Serial.println("Bar ");
    Serial.println(bar);
    Serial.println("Count ");
    Serial.println(+count);
    display.display();
  }
  //  delay(2000);
}

void loop() {
  int x, y, z;
  
  // Read compass values
  compass.read();

  // Return XYZ readings
  x = compass.getX();
  y = compass.getY();
  z = compass.getZ();
  
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.print(z);
  Serial.println();
  
  float heading = atan2(y, x);
  
    // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 2.10;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
//  display.drawRect(0, 1, 120, 120, WHITE);
//        n = 40 
//        e = 115
//        s = -140
//        w = -27
  if (Firebase.RTDB.setFloat(&fbdo, "User/angle", headingDegrees)) {
    Serial.println("Glucose Level PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
  headingDegrees -= 170;
  if(18 < headingDegrees && headingDegrees <= 65){
        // Display static text
        display.println("N");
  }else if(65 < headingDegrees && headingDegrees <= 90){
        // Display static text
        display.println("NE");
  }else if(90 < headingDegrees && headingDegrees <= 150){
        // Display static text
        display.println("E");
  }else if(150 < headingDegrees && headingDegrees <= 180){
        // Display static text
        display.println("SE");
  }else if(-180 <= headingDegrees && headingDegrees <= -175){
        // Display static text
        display.println("SE");
  }else if(-175 < headingDegrees && headingDegrees <= -103){
        // Display static text
        display.println("S");
  }else if(-103 < headingDegrees && headingDegrees <= -76){
        // Display static text
        display.println("SW");
  }else if(-76 < headingDegrees && headingDegrees <= -4){
        // Display static text
        display.println("W");
  }else if(-4 < headingDegrees && headingDegrees <= 0){
        // Display static text
        display.println("NW");
  }else if(0 < headingDegrees && headingDegrees <= 18){
        // Display static text
        display.println("NW");
  }
  display.display();
  delay(250);
}
