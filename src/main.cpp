#include <Arduino.h>
#include <ArduinoHttpClient.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <NewPing.h>
#include <DallasTemperature.h>
#include "Fonts/TomThumb.h"


// testing
#include <WiFi.h>

int adcAverage(int batteryPin, int read);
float adcToVolts(int batteryPin);
float voltToBatteryPercent(float voltage, float in_min, float in_max, float out_min, float out_max);
int getThreshold(int distance);
void RestartGSMModem();
void getValue();
String GSMSignalLevel(int level);
String GSMRegistrationStatus(RegStatus state);
String SIMStatus(SimStatus state);
bool SendTextByPOST(String server, String url, String postData);
void Init_GSM_SIM800();
void initWiFi();
void searchWiFi();
void startDisplay();
void soundhelena();
void soundLeftErr();
void soundRightErr();
void goSleep();



const char *ssid = "GlobeAtHome_B01FB";
const char *password = "48550858";

const char *ntpServer = "pool.ntp.org";

const long gmtOffset_sec = 8;
// Pin Numbers
#define RXD2 23
#define TXD2 16

#define ultraTrig1 17
#define ultraEcho1 5
#define ultraTrig2 18
#define ultraEcho2 19

#define BUTTON_PIN_1 14
#define BUTTON_PIN_2 27

#define ADC_PIN 12
#define CONV_FACTOR 1.75

NewPing sonar1(ultraTrig1, ultraEcho1);
NewPing sonar2(ultraTrig2, ultraEcho2);

#define BUZZZER_PIN 33

#define OLED_RESET -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ONE_WIRE_BUS 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Place your APN here
#define APN_NAME "internet.globe.com.ph"
#define APN_USER ""
#define APN_PSWD ""

// auto baud range
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 38400

#define Max_Modem_Reboots 5

// set your cad pin (optional)
#define SIM_PIN ""

// Others
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 900 // 15 mins 
#define S_TO_MIN_FACTOR 60

TinyGsm modemGSM(Serial2);

TinyGsmClient clientGSM(modemGSM);

String server = "etaponcloud.azurewebsites.net";
const int port = 80;

int Modem_Reboots_Counter = 0;

HttpClient *http;

String name = "Cloudbin_ITECH2";

const unsigned char trashbin[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x07, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x07, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x7F, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xEF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x39, 0xCF, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};





float distance1, distance2, factoredDis, duration1, duration2;
int temperature, distanceThresh1, distanceThresh2, bat_percentage, sensorValue;
int currentState1,
    currentState2;
float voltage;
float factor = sqrt(1 + temperature / 273.15) / 60.368;

void setup()
{
    Serial.begin(9600);
    delay(500);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    pinMode(BUZZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);

    currentState1 = digitalRead(BUTTON_PIN_1);
    currentState2 = digitalRead(BUTTON_PIN_2);

    Serial.print("BTN 1 state: ");
    Serial.print(currentState1);
    Serial.println("");
    Serial.print("BTN 2 state: ");
    Serial.print(currentState2);
    Serial.println("");

    if(currentState1 && currentState2 != 0){
        Serial.println("Please close both lids");
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(25, 15);
        display.println("Please close both lids");
        display.setTextSize(1);
        display.display();
        soundLeftErr();
        soundRightErr();

        delay(5000);
        goSleep();

    } else if (currentState1 != 0){
        Serial.println("Please close the trashbin lid at left side");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(15, 10);
    display.print("Please Close the");
    display.setCursor(15, 25);
    display.print("trash bin lid at");
    display.setCursor(15, 40);
    display.print("left side");
    display.display();
        soundLeftErr();
        delay(5000);
        goSleep();
    } else if (currentState2 != 0){
        Serial.println("Please close the trashbin lid at right side");
        display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(15, 10);
    display.print("Please Close the");
    display.setCursor(15, 25);
    display.print("trash bin lid at");
    display.setCursor(15, 40);
    display.print("right side");
    display.display();
        soundRightErr();
        delay(5000);
        goSleep();        
    } else {

    }

    startDisplay();
    delay(2000);

    Init_GSM_SIM800();



    // int distance1 = 20 + (int)random(20)/10;
    // int distance2 = 30 + (int)random(30)/10;

    // String data = "{\"binName\":\"" + name + "\",\"battery1\": 60,\"bin1\": " + distance1 + ",\"bin2\": " + distance2 + "}";

    // Serial.println("Send data [" + data + "] to [" + server + "].");
    // SendTextByPOST(server, "/update_data", data);
}

void loop()
{
    getValue();
    /* Code for no temp and with temp

    Serial.print("Temperature for Device 1 is: ");
    Serial.println(temperature);
    float speedOfSound = 331.3 * (sqrt(1 + temperature / 273.15));

    Serial.print("Speed of sound: ");
    Serial.println(speedOfSound);

    duration1 = sonar1.ping_median(5, 50);
    Serial.print("Ping Median: ");
    Serial.println(duration1);
    duration1 = duration1 / 1000000;
    Serial.print("Duration: ");
    Serial.println(duration1);
    distance1 = (speedOfSound * duration1) / 2;
    distance1 = distance1 * 1000;

    Serial.print("Distance w/ temp: ");
    Serial.println(distance1);
    duration2 = sonar2.ping_median(5, 50);
    Serial.print("Ping Median2: ");
    Serial.println(duration2);
    duration2 = duration2 / 1000000;
    Serial.print("Duration2: ");
    Serial.println(duration2);
    distance2 = (340 * duration2) / 2;
    distance2 = distance2 * 1000;

    Serial.print("Distance w/o temp: ");
    Serial.println(distance2);

    */
    goSleep();
    Serial.println("This will never be printed");
}

void goSleep(){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(&TomThumb);
    display.setCursor(0, 10);
    display.println("Device will sleep");
    display.display();
    delay(2000);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                   " Seconds");
    Serial.println("Going to sleep now");
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.flush();
    esp_deep_sleep_start();
}

void getValue()
{
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    temperature = sensors.getTempCByIndex(0);

    float speedOfSound = 331.3 + (0.6 * temperature);
    Serial.print("Speed of Sound: ");
    Serial.println(speedOfSound);


    duration1 = sonar1.ping_median(10);
    duration1 /= uS_TO_S_FACTOR;
    distance1 = (duration1 * speedOfSound) / 2;
    distance1 *= 100;

    Serial.println("Distance1: ");
    Serial.print(distance1);
    distanceThresh1 = getThreshold(distance1);

    Serial.println("Distance Threshold 1: ");
    Serial.print(distanceThresh1);
    delay(100);

    /*
        old computation
    distance1 = sonar1.ping_cm();
    distanceThresh1 = getThreshold(distance1);
    Serial.println("Distance1: ");
    Serial.print(distance1);
    distance2 = sonar2.ping_cm();
    distanceThresh2 = getThreshold(distance2);
    Serial.println("Distance2: ");
    Serial.print(distance2);

    */
    duration2 = sonar2.ping_median(10);
    duration2 /= uS_TO_S_FACTOR;
    distance2 = (duration2 * speedOfSound) / 2;
    distance2 *= 100;

    Serial.println("Distance2: ");
    Serial.print(distance2);
    distanceThresh2 = getThreshold(distance2);

    Serial.println("Distance Threshold 2: ");
    Serial.print(distanceThresh2);
    delay(100);
    

    sensorValue = analogRead(14);
    voltage = adcToVolts(sensorValue);
    bat_percentage = voltToBatteryPercent(voltage, 2.5, 4.2, 0, 100);
    Serial.println("My Computation");
    Serial.print("Value from pin: ");
    Serial.println(sensorValue);
    Serial.print("Voltage read: ");
    Serial.println(voltage);
    Serial.print("Battery Percentage: ");
    Serial.println(bat_percentage);
    Serial.println("");

    delay(5000);
    display.clearDisplay();
    display.drawBitmap(5, 8, trashbin, 128, 64, WHITE);
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(58, 8);
    display.print("Bin 1:");
    display.setCursor(64, 20);
    display.print(distanceThresh1);
    display.setCursor(58, 40);
    display.print("Bin 2:");
    display.setCursor(64, 52);
    display.print(distanceThresh2);
    display.display();
    
    delay(5000);

    display.clearDisplay();
    display.setCursor(22, 23);
    display.print("Sending Values");
    display.setCursor(25, 35);
    display.println("to the server");
    display.display();

    String data = "{\"binName\":\"" + name + "\",\"battery2\": " + bat_percentage + ",\"bin3\": " + distanceThresh1 + ",\"bin4\": " + distanceThresh2 + "}";

    Serial.println("Send data [" + data + "] to [" + server + "].");

    SendTextByPOST(server, "/update_data", data);
    tone(BUZZZER_PIN, 164.81, 500);
    tone(BUZZZER_PIN, 220.00, 500);
    noTone(BUZZZER_PIN);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(1, 3);
    display.print("Success");
    display.display();
    delay(5000);
}

int adcAverage(int batteryPin, int read)
{
    int totalADC = 0;
    int averageADC = 0;
    for (int i = 0; i <= read; i++)
    {
        totalADC += analogRead(batteryPin);
    }
    averageADC = totalADC / read;
    return averageADC;
}

float adcToVolts(int batteryPin)
{
    float voltage = ((batteryPin * CONV_FACTOR) / 1000) - 2.30;
    return voltage;
}

float voltToBatteryPercent(float voltage, float in_min, float in_max, float out_min, float out_max)
{
    float batteryPercent = (voltage - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if (batteryPercent >= 100)
    {
        return 100;
    }
    else if (batteryPercent < 0)
    {
        return 0;
    }
    else
    {
        return batteryPercent;
    }
}

int getThreshold(int distance)
{
    int distanceThresh = 0;

    if (distance <= 2)
    {
        distanceThresh = 100;
    }
    else if (distance <= 8)
    {
        distanceThresh = 75;
    }
    else if (distance <= 16)
    {
        distanceThresh = 50;
    }
    else if (distance <= 24)
    {
        distanceThresh = 25;
    }
    else if (distance <= 33)
    {
        distanceThresh = 0;
    }
    return distanceThresh;
}


void RestartGSMModem()
{
    Serial.println("Restarting GSM...");
    if (!modemGSM.restart()) // orignally restart()
    {
        Serial.println("\tFailed. :-(\r\n");
        // ESP.restart();
    }
    if (Modem_Reboots_Counter < Max_Modem_Reboots)
    {
        Init_GSM_SIM800();
    }
    Modem_Reboots_Counter++;
}

String GSMSignalLevel(int level)
{
    switch (level)
    {
    case 0:
        return "-115 dBm or less";
    case 1:
        return "-111 dBm";
    case 31:
        return "-52 dBm or greater";
    case 99:
        return "not known or not detectable";
    default:
        if (level > 1 && level < 31)
            return "-110... -54 dBm";
    }
    return "Unknown";
}

String GSMRegistrationStatus(RegStatus state)
{
    switch (state)
    {
    case REG_UNREGISTERED:
        return "Not registered, MT is not currently searching a new operator to register to";
    case REG_SEARCHING:
        return "Not registered, but MT is currently searching a new operator to register to";
    case REG_DENIED:
        return "Registration denied";
    case REG_OK_HOME:
        return "Registered, home network";
    case REG_OK_ROAMING:
        return "Registered, roaming";
    case REG_UNKNOWN:
        return "Unknown";
    }
    return "Unknown";
}

String SIMStatus(SimStatus state)
{
    switch (state)
    {
    case SIM_ERROR:
        return "SIM card ERROR";
    case SIM_READY:
        return "SIM card is READY";
    case SIM_LOCKED:
        return "SIM card is LOCKED. PIN/PUK needed.";
    }
    return "Unknown STATUS";
}

bool SendTextByPOST(String server, String url, String postData)
{
    bool stateGPRS = modemGSM.isGprsConnected();
    bool res = false;
    if (stateGPRS)
    {
        http = new HttpClient(clientGSM, server, port);
        Serial.println("Send POST request...");
        http->beginRequest();
        http->post(url);
        http->sendHeader("Content-Type", "application/json");
        http->sendHeader("Content-Length", postData.length());
        // http.sendHeader("X-Custom-Header", "custom-header-value");
        http->beginBody();
        http->print(postData);
        http->endRequest();

    
        // read the status code and body of the response
        int statusCode = http->responseStatusCode();

        if (statusCode == 200)
        {
            String response = http->responseBody();
            Serial.println("Status code: " + String(statusCode));
            Serial.println("Response: " + response);
            res = false;
        }
        else
        {
            Serial.println("Error code: " + String(statusCode));
            res = true;
        }

        http->stop();
    }
    return res;
}

void Init_GSM_SIM800()
{
    Serial.println("Initialize GSM modem...");
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial GSM Txd is on GPIO" + String(TXD2));
    Serial.println("Serial GSM Rxd is on GPIO" + String(RXD2));

    // pinMode(LED_PIN, OUTPUT);

    delay(3000);

    TinyGsmAutoBaud(Serial2, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);

    String info = modemGSM.getModemInfo();

    Serial.println(info);

    if (!modemGSM.restart())
    {
        RestartGSMModem();
    }
    else
    {
        Serial.println("Modem restart OK");
    }

    if (modemGSM.getSimStatus() != SIM_READY)
    {
        Serial.println("Check PIN code for the SIM. SIM status: " + SimStatus(modemGSM.getSimStatus()));
        if (SIM_PIN != "")
        {
            Serial.println("Try to unlock SIM PIN.");
            modemGSM.simUnlock(SIM_PIN);
            delay(3000);
            if (modemGSM.getSimStatus() != 3)
            {
                RestartGSMModem();
            }
        }
    }

    if (!modemGSM.waitForNetwork())
    {
        Serial.println("Failed to connect to network");
        RestartGSMModem();
    }
    else
    {
        RegStatus registration = modemGSM.getRegistrationStatus();
        Serial.println("Registration: [" + GSMRegistrationStatus(registration) + "]");
        Serial.println("Modem network OK");
    }

    Serial.println(modemGSM.gprsConnect(APN_NAME, APN_USER, APN_PSWD) ? "GPRS Connect OK" : "GPRS Connection failed");

    bool stateGPRS = modemGSM.isGprsConnected();
    if (!stateGPRS)
    {
        RestartGSMModem();
    }

    String state = stateGPRS ? "connected" : "not connected";
    Serial.println("GPRS status: " + state);
    Serial.println("CCID: " + modemGSM.getSimCCID());
    Serial.println("IMEI: " + modemGSM.getIMEI());
    Serial.println("Operator: " + modemGSM.getOperator());

    IPAddress local = modemGSM.localIP();
    Serial.println("Local IP: " + local.toString());

    int csq = modemGSM.getSignalQuality();
    if (csq == 0)
    {
        Serial.println("Signal quality is 0. Restart modem.");
        RestartGSMModem();
    }
    Serial.println("Signal quality: " + GSMSignalLevel(csq) + " [" + String(csq) + "]");
    int battLevel = modemGSM.getBattPercent();
    Serial.println("Battery level: " + String(battLevel) + "%");

    float battVoltage = modemGSM.getBattVoltage() / 1000.0F;
    Serial.println("Battery voltage: " + String(battVoltage));

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(&TomThumb);
    display.setCursor(1, 7);
    display.println("Initialize GSM modem...");
    display.setTextSize(1);
    display.setCursor(1, 15);
    display.println("Status: ");
    display.setCursor(45, 15);
    display.print(state);
    display.setCursor(1, 23);
    display.println("Sim:");
    display.setCursor(45, 23);
    display.print(modemGSM.getOperator());
    display.setCursor(1, 31);
    display.println("RSSI:");
    display.setCursor(45, 31);
    display.print(GSMSignalLevel(csq));
    display.setCursor(1, 39);
    display.println("IP:");
    display.setCursor(45, 39);
    display.print(local.toString());
    display.display();
    
    delay(5000);

}

void initWiFi()
{

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("Wifi Connected");
    Serial.println(WiFi.localIP());
}

void searchWiFi()
{
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");

    // Wait a bit before scanning again
    delay(5000);
}

void startDisplay(){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(25, 15);
    display.println("E-Tapon");
    display.setTextSize(1);
    display.setCursor(20, 40);
    display.println("Trash bin 3 & 4");
    display.display();

}

void soundhelena(){
    
  tone(BUZZZER_PIN, 783.99);
  delay(1000);
  tone(BUZZZER_PIN, 739.99);
  delay(500);
  tone(BUZZZER_PIN , 987.77);
  delay(500);
  tone(BUZZZER_PIN , 659.25);
  delay(1000);
noTone(BUZZZER_PIN);
  delay(1000);
}

void soundLeftErr(){
  tone(BUZZZER_PIN, 659); // E4
  delay(200);
  tone(BUZZZER_PIN, 700); // F4
  delay(200);
  tone(BUZZZER_PIN, 987.77); //B4
  delay(200);

  noTone(BUZZZER_PIN);
  delay(1500);
}
void soundRightErr(){
  // alert bin 2 
  tone(BUZZZER_PIN, 659); // E4
  delay(200);
  tone(BUZZZER_PIN, 700); // E4
  delay(200);
  tone(BUZZZER_PIN, 523.25); // C4
  delay(200);

  noTone(BUZZZER_PIN);
  delay(1500);
}

