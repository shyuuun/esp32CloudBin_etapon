#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <HTTPClient.h>



void openBin1();
void openBin2();
void openSort();
boolean checkMetal();
boolean checkNonBio();
void objectDetection();

void RestartGSMModem();
String GSMRegistrationStatus(RegStatus state);
String SIMStatus(SimStatus state);
String GSMSignalLevel(int level);
void Init_GSM_SIM800();


// metal sensor
#define METAL_PIN  25

// Check is the servo motor that goes in left and right position of the bin while eject is makes the object falls to the bin 

#define SERVO_BIN1 16
#define SERVO_BIN2 4
#define SERVO_SORT 18
#define SERVO_DROP 19

//#define BUZZER_PIN 13

/*
    Left and Right are the ultrasonic sensors place in right and left side of the trash bin
    Check is the ultrasonic sensor that check if there is any object placed on the cylinder 

*/
#define TRIG_CHECK 33
#define ECHO_CHECK 32
#define TRIG_LEFT 17
#define ECHO_LEFT 5

#define IR_PIN 23

// RX/TX Pins for sim 800;

#define RXD2 27
#define TXD2 14

LiquidCrystal_I2C lcd(0x3F, 16, 2); // 0x3F 
Servo servoBin1, servoBin2, servoSort, servoDrop;


NewPing sonarCheck(TRIG_CHECK, ECHO_CHECK);
NewPing sonarLeft(TRIG_LEFT, ECHO_LEFT);



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
int ir_value = 0;


void setup() {
    Serial.begin(9600);
    // pinMode(BUZZER_PIN, OUTPUT);
    //pinMode(TRIG_PIN1, OUTPUT);
    //pinMode(ECHO_PIN1, INPUT);
    Serial.println("Initialize Metal Sensor");
    pinMode(METAL_PIN, INPUT);
    pinMode(IR_PIN, INPUT);

    // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

    servoSort.setPeriodHertz(50);
    servoSort.attach(SERVO_SORT);

    servoSort.write(180);


    // Initializing LCD screen
    lcd.init();
    Serial.println("Initialize LCD Screen");
    lcd.clear();         
    lcd.backlight();      // Make sure backlight is on


    lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
    lcd.print("E-Tapon");
    delay(5000);

    // Init_GSM_SIM800(); // turn on the sim800l module
    
}

void loop() {
    // make sure the sort is always off
    servoSort.write(0);

    objectDetection();
    delay(500);
}

// For Metal Detection
boolean checkMetal(){
    metal_value = digitalRead(METAL_PIN);
    Serial.print("Metal Value: "); // 1 if metal is detected and 0 is not
    Serial.println(metal_value);

    if(metal_value == LOW){
        Serial.println("This object is metal");
        return true;        
    } else {
        return false;
    }
}

// For Non-metal Detection
boolean checkNonBio(){
    Serial.println("This is non bio");
    ir_value = digitalRead(IR_PIN);
    Serial.print("IR Value: "); // 1 if nonmetal is detected and 0 is not
    Serial.println(ir_value);

    if(ir_value == LOW){
        Serial.println("This object is non-metal");
        return true;        
    } else {
        return false;
    }
}

// This function will tell if the object is present within in the sorting area of the bin
void objectDetection(){
    ir_value = digitalRead(IR_PIN);
    Serial.println("Object Detected");
    Serial.println("Checking...");
    delay(500);

    if(checkMetal()){
        Serial.println("Rotating to the left side of the bin");
        openSort();
    } else if (checkNonBio()){
        Serial.println("Rotating to the right side of the bin");
        openSort();
    } else {
        Serial.println("Object doesnt detect properly");
    }
}

// For Servo motors

void openBin1(){
    // Set period of hertz to the servo motor 
	servoBin1.setPeriodHertz(50);    // standard 50 hz servo
	//servoBin1.attach(SERVO_BIN1); // attaches the servo on pin 18 to the servo object

    Serial.print("Bin 1 opened: ");
    servoBin1.write(180);
    delay(1500);
    servoBin1.write(0);
    delay(1500);
}

void openBin2(){
    // Set period of hertz to the servo motor 
	servoBin2.setPeriodHertz(50);    // standard 50 hz servo
	servoBin2.attach(SERVO_BIN2); // attaches the servo on pin 18 to the servo object

    Serial.print("Bin 2 opened: ");
    servoBin2.write(180);
    delay(1500);
    servoBin2.write(0);
    delay(1500);
}

void openSort(){
    servoSort.setPeriodHertz(50);
    servoSort.attach(SERVO_SORT);

    Serial.println("Sort door opened: ");
    servoSort.write(0);
    delay(1500);
    servoSort.write(200);
    delay(1500);
}

// For SIM800L Module 

void Init_GSM_SIM800()
{

    Serial.println("Initialize GSM modem...");
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial GSM Txd is on GPIO" + String(TXD2));
    Serial.println("Serial GSM Rxd is on GPIO" + String(RXD2));


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