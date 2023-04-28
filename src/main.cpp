#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>

void servoTest();
void servoLeft();
void detectObject();
void detectMetalObject();


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

LiquidCrystal_I2C lcd(0x3F, 16, 2); // 0x3F 
Servo servo, servoBin1, servoBin2;

NewPing sonarCheck(TRIG_CHECK, ECHO_CHECK);
NewPing sonarLeft(TRIG_LEFT, ECHO_LEFT);
NewPing sonarRight(TRIG_RIGHT, ECHO_RIGHT);

int metal_value = 0;
int numLeft , numRight, numAll;

void setup() {
    Serial.begin(9600);
    // pinMode(BUZZER_PIN, OUTPUT);
    pinMode(METAL_PIN, INPUT);
    //pinMode(TRIG_PIN1, OUTPUT);
    //pinMode(ECHO_PIN1, INPUT);
    Serial.println("Initialize Metal Sensor");
    servoBin1.attach(SERVO_CHECK);
    //servoBin2.attach(SERVO_BIN2_PIN);
    lcd.init();
    Serial.println("Initialize LCD Screen");
    lcd.clear();         
    lcd.backlight();      // Make sure backlight is on


    lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
    lcd.print("E-Tapon");
    delay(5000);
    lcd.noDisplay();
    
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
        lcd.setCursor(2,0); 
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


    }else {
        lcd.setCursor(0,0); 
        lcd.print("Non-Metal Detected!");
        Serial.println("Metal not detected");   

        delay(5000);
        /*
        if(!hasDrop()){
            Serial.println("Waiting for object to be dropped");
        } else {
            numRight++;
            Serial.print("Number of objects placed in left bin");
            Serial.println(numRight);
        }
        */

    }
}


