#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <HTTPClient.h>



void servoRight();
void servoLeft();
void detectObject();
void detectMetalObject();
void RestartGSMModem();
String GSMRegistrationStatus(RegStatus state);
String SIMStatus(SimStatus state);
String GSMSignalLevel(int level);
void Init_GSM_SIM800();


// metal sensor
#define METAL_PIN  25

// Check is the servo motor that goes in left and right position of the bin while eject is makes the object falls to the bin 

#define SERVO_CHECK  16
#define SERVO_EJECT 4

//#define BUZZER_PIN 13

/*
    Left and Right are the ultrasonic sensors place in right and left side of the trash bin
    Check is the ultrasonic sensor that check if there is any object placed on the cylinder 

*/
#define TRIG_CHECK 33
#define ECHO_CHECK 32
#define TRIG_LEFT 17
#define ECHO_LEFT 5
#define TRIG_RIGHT 18
#define ECHO_RIGHT 19

// RX/TX Pins for sim 800;

#define RXD2 27
#define TXD2 14

LiquidCrystal_I2C lcd(0x3F, 16, 2); // 0x3F 
Servo servoCheck, servoEject;


NewPing sonarCheck(TRIG_CHECK, ECHO_CHECK);
NewPing sonarLeft(TRIG_LEFT, ECHO_LEFT);
NewPing sonarRight(TRIG_RIGHT, ECHO_RIGHT);


TinyGsm modemGSM(Serial2);
TinyGsmClient clientGSM(modemGSM);

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

// our server
String server = "etaponcloud.azurewebsites.net";
const int port = 80;

int Modem_Reboots_Counter = 0;

HTTPClient *http;

int metal_value = 0;
int numLeft , numRight, numAll;

void setup() {
    Serial.begin(9600);
    // pinMode(BUZZER_PIN, OUTPUT);
    pinMode(METAL_PIN, INPUT);
    //pinMode(TRIG_PIN1, OUTPUT);
    //pinMode(ECHO_PIN1, INPUT);
    Serial.println("Initialize Metal Sensor");
    servoCheck.attach(SERVO_CHECK);
    servoEject.attach(SERVO_EJECT);
    //servoBin2.attach(SERVO_BIN2_PIN);
    lcd.init();
    Serial.println("Initialize LCD Screen");
    lcd.clear();         
    lcd.backlight();      // Make sure backlight is on


    lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
    lcd.print("E-Tapon");
    delay(5000);

    Init_GSM_SIM800();
    
}

void loop() {
    detectObject();
}


// this will ping the two ultrasonic sensors in two bins
boolean hasDrop(){
    if(sonarLeft.ping_cm() <= 10 || sonarRight.ping_cm() <= 10){
        Serial.println("Object Fall Detected");
        numAll++;
        Serial.print("Number of objects placed in bin");
        Serial.println(numAll);
        return true;
    } else 
        Serial.println("Object Fall Not Detected");
        return false;
}

void detectObject(){
    delay(1500);
    if (sonarCheck.ping_cm() < 5){
        lcd.display();
        lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
        lcd.print("Object Detected");
        delay(1500);
        Serial.println("Object in cylinder detected");

        detectMetalObject();    
    }else {

        Serial.println("Object in cylinder not detected");
    
    }
}

void detectMetalObject(){
    metal_value = digitalRead(METAL_PIN);
    if(metal_value == 0){
        lcd.setCursor(0,0); 
        lcd.print("Metal Detected!");
        Serial.println("Metal Detected");

        // tone(BUZZER_PIN, 5000);
        /*
        Testing ko lang ito need pa i-program yung servo motor
        
        if(!hasDrop()){
            Serial.println("Waiting for object to be dropped");
        } else {
            numLeft++;
            Serial.print("Number of objects placed in left bin");
            Serial.println(numLeft);
        }
        */
        servoLeft();

    }else {
        lcd.setCursor(0,0); 
        lcd.print("Non-Metal Detected!");
        Serial.println("Metal not detected");   


        /*
        if(!hasDrop()){
            Serial.println("Waiting for object to be dropped");
        } else {
            numRight++;
            Serial.print("Number of objects placed in left bin");
            Serial.println(numRight);
        }
        */
        servoRight();
    }
}

void servoLeft(){

    // go to the left bin
    servoCheck.writeMicroseconds(1000);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);
    servoCheck.writeMicroseconds(1000);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);

    delay(1500);
    servoEject.write(180);
    delay(1500);

    // back to normal 

    servoCheck.writeMicroseconds(2000);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);
    servoCheck.writeMicroseconds(2000);
    delay(150);
    servoCheck.writeMicroseconds(1500);


    delay(1500);
    servoEject.write(0);
    delay(1500);
}

void servoRight(){
    // go to right bin
    servoCheck.writeMicroseconds(2000);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);
    servoCheck.writeMicroseconds(2000);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);

    delay(1500);
    servoEject.write(180);
    delay(1500);

    servoCheck.writeMicroseconds(0);
    delay(150);
    servoCheck.writeMicroseconds(1500);
    delay(150);
    servoCheck.writeMicroseconds(0);
    delay(175);
    servoCheck.writeMicroseconds(1500);
    delay(150);

    delay(1500);
    servoEject.write(0);
    delay(1500);
}

void Init_GSM_SIM800()
{

    Serial.println("Initialize GSM modem...");
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial GSM Txd is on GPIO" + String(TXD2));
    Serial.println("Serial GSM Rxd is on GPIO" + String(RXD2));

    delay(2000);

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