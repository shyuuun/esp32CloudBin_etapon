#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>



void openBin1();
void openBin2();
void openSort();
boolean checkMetal();
boolean checkNonBio();
void objectDetection();
void rotateLeft();
void rotateRight();
void checkBin();
bool SendTextByPOST();
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
#define TRIG_RIGHT 33
#define ECHO_RIGHT 32
#define TRIG_LEFT 17
#define ECHO_LEFT 5

#define IR_PIN 23

// RX/TX Pins for sim 800;

#define RXD2 27
#define TXD2 14

LiquidCrystal_I2C lcd(0x3F, 16, 2); // 0x3F 
Servo servoBin1, servoBin2, servoSort, servoDrop;

NewPing sonarRight(TRIG_RIGHT, ECHO_RIGHT);
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

HttpClient *http;

int metal_value = 0;
int ir_value = 0;
int sonarLeftVal, sonarRightVal;

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
    servoDrop.setPeriodHertz(50);
    servoDrop.attach(SERVO_DROP);


    // Initializing LCD screen
    lcd.init();
    Serial.println("Initialize LCD Screen");
    lcd.clear();         
    lcd.backlight();      // Make sure backlight is on


    lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
    lcd.print("E-Tapon");
    delay(5000);

    Init_GSM_SIM800(); // turn on the sim800l module
    
}

void loop() {
    // make sure the sort door is always closed 
    servoSort.write(0);
    // default position of the servoDrop motor
    servoDrop.write(120);

    objectDetection();
    delay(150);
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
    delay(500);
    Serial.println("Waiting for Object");

    lcd.setCursor(3,0);
    lcd.print("Waiting for");
    lcd.setCursor(5 ,1);
    lcd.print("Object");

    if(checkMetal()){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Object: Metal");
        Serial.println("Rotating to the left side of the bin");

        rotateLeft();
        openSort();

        // verifies if the metal object placed to the bin
        if(checkMetal() == true){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Object doesn't fit");
            delay(1500);
            lcd.clear();

            Serial.println("Object doesnt fit to the sorting tray");
            Serial.println("Please arrange it properly");
        } else {
            
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Object placed at");
            lcd.setCursor(0,1);
            lcd.print("Metal bin");
            delay(1500);
            lcd.clear();

            Serial.println("Object placed at Metal Bin");

            checkBin();
        }

    } else if (checkNonBio()){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Object: Non-Metal");

        Serial.println("Rotating to the right side of the bin");
        rotateRight();
        openSort();

        // verifies if the non-metal object placed to the bin
        if(checkNonBio() == true){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Object doesn't fit");
            delay(1500);
            lcd.clear();

            Serial.println("Object doesnt fit to the sorting tray");
            Serial.println("Please arrange it properly");
        } else {

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Object placed at");
            lcd.setCursor(0,1);
            lcd.print("Non-metal bin");
            delay(1500);
            lcd.clear();

            Serial.println("Object placed at Non-metal Bin");
        }
    } 
}

// For Servo motors

void openBin1(){
    // Set period of hertz to the servo motor 
	servoBin1.setPeriodHertz(50);    // standard 50 hz servo
	servoBin1.attach(SERVO_BIN1);    // attaches the servo on pin 18 to the servo object

    Serial.print("Bin 1 opened: ");
    servoBin1.write(180);
    delay(1500);
    servoBin1.write(0);
    delay(1500);
}

void openBin2(){
    // Set period of hertz to the servo motor 
	servoBin2.setPeriodHertz(50);    // standard 50 hz servo
	servoBin2.attach(SERVO_BIN2);   // attaches the servo on pin 18 to the servo object

    Serial.print("Bin 2 opened: ");
    servoBin2.write(180);
    delay(1500);
    servoBin2.write(0);
    delay(1500);
}

void openSort(){
    Serial.println("Sort door opened: ");
    servoSort.write(0);
    delay(2000);
    servoSort.write(200);
    delay(1500);
}

void rotateLeft(){
    servoDrop.setPeriodHertz(50);
    servoDrop.attach(SERVO_DROP);

    Serial.println("Rotating to the left side of the bin");
    servoDrop.write(30);
    delay(1000);
}

void rotateRight(){
    Serial.println("Rotating to the right side of the bin");
    servoDrop.write(180);
    delay(1000);
}

// Checking the conditons of the bin
void checkBin(){
    sonarLeftVal = sonarLeft.ping_cm();
    sonarRightVal = sonarRight.ping_cm();

    Serial.println("Sonar Left: ");
    Serial.print(sonarLeftVal);
    Serial.println("Sonar Right: ");
    Serial.print(sonarRightVal);
    
    /*
        the width of the trash bin is 36cm
        if the objects are near the ultrasonic sensor e.g. 10cm
        it is considered full
    */
    
    if(sonarLeftVal <= 10 || sonarLeftVal > 40){
        Serial.println("Trash bin on the left is full");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.println("Metal Bin: Full");

        sonarLeftVal = 1;

        delay(1500);
        
    } else {
        Serial.println("Trash bin on the left is not full");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.println("Metal Bin:");
        lcd.setCursor(0,1);
        lcd.println("Not Full");
        sonarLeftVal = 0;
        delay(1500);
    }

    if(sonarRightVal <= 10 || sonarRightVal > 40){
        Serial.println("Trash bin on the right is full");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.println("NonMtl Bin: Full");
        sonarRightVal = 1;
        delay(1500);
    } else {
        Serial.println("Trash bin on the right is not full");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.println("NonMtl Bin:");
        lcd.setCursor(0,1);
        lcd.println("Not Full");
        sonarRightVal = 0;
        delay(1500);
    }
    lcd.clear();
}

// For SIM800L Module 

void Init_GSM_SIM800()
{

    Serial.println("Initialize GSM modem...");
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial GSM Txd is on GPIO" + String(TXD2));
    Serial.println("Serial GSM Rxd is on GPIO" + String(RXD2));

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Init GSM Modem");
    delay(1500);

    String info = modemGSM.getModemInfo();

    Serial.println(info);

    if (!modemGSM.restart())
    {
        RestartGSMModem();
    }
    else
    {
        Serial.println("Modem restart OK");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Modem restarting");
        delay(1500);
    }


    if (!modemGSM.waitForNetwork())
    {
        Serial.println("Failed to connect to network");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Failed Network");
        delay(1500);

        RestartGSMModem();
    }
    else
    {
        RegStatus registration = modemGSM.getRegistrationStatus();
        Serial.println("Registration: [" + GSMRegistrationStatus(registration) + "]");
        Serial.println("Modem network OK");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Modem network:");
        lcd.setCursor(5,1);
        lcd.print("OK");
        delay(1500);
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

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("GPRS: ");
    lcd.print(state);
    lcd.setCursor(0,1);
    lcd.print("Signal: ");
    lcd.print(csq);
    delay(1500);

    lcd.clear();
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