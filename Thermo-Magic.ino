// This #include statement was automatically added by the Particle IDE.
#include <LedControl-MAX7219-MAX7221.h>

//Include libraries
#include "spark-dallas-temperature.h"
#include <OneWire.h>
// Data wire is plugged into pin D7 on Particle
#define ONE_WIRE_BUS D7
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Set Time Zones
const int centralZone = -6;
const int easternZone = -5;
const int mountainZone = -7;
const int pacificZone = -8;

int GetDesiredTemp(String command); //Defines function to get desired temp from web
int GetSystemMode(String command); //Defines function to gets system mode
int GetEcoFactor(String command); //Defines function to get eco factor
String DesiredTempStr = "72"; //Sets tempertature string
String SystemModeStr = "Off";
int SystemMode = 0; //Sets System mode eg Heat, Cool, Auto, Fan
String EcoFactorStr = "2"; // Sets Eco Factor (for auto mode) default 2 string
String TempStr;
String ActiveMode = "Off";
int DesiredTemp = 72; //Sets tempertature int
int EcoFactor = 2; // Sets Eco Factor (for auto mode) default 2 int
int Temp; //current temp
int TempDebug; //temp read from IC Debugged for EM interference
int Fan = D2; // fan realy hooked to pin D1
int Heat = D0; // heat relay hooked to pin D2
int Cool = D1; // cool relay hooked to pin D3
int MotionPin = D3;
int UpPin = D4;
int DownPin = D5;
int ModePin = D6;
bool HeatOn = false;
bool CoolOn = false;
LedControl* led;
int phase = 0;
char message[64];
int messageLength = 0;
int myUptime = 0;

uint8_t data = A5;
uint8_t load = A4;
uint8_t myclock = A3;

void setup()
{
    Particle.function("DesiredTemp", GetDesiredTemp);
    Particle.function("SystemMode", GetSystemMode);
    Particle.function("EcoFactor", GetEcoFactor); //Initializes Particle API
    Particle.variable("DesiredTemp", DesiredTemp);
    Particle.variable("SystemMode", SystemModeStr);
    Particle.variable("CurrentTemp", Temp);
    Particle.variable("EcoFactor", EcoFactor);
    Particle.variable("ActiveMode", ActiveMode);

    sensors.begin(); //Initilaizes senor reading

    //Initialize Relays and set them to Normally open
    pinMode(Fan, OUTPUT);
    pinMode(Heat, OUTPUT);
    pinMode(Cool, OUTPUT);
    pinMode(MotionPin, INPUT);
    pinMode(UpPin, INPUT_PULLUP);
    pinMode(DownPin, INPUT_PULLUP);
    pinMode(ModePin, INPUT_PULLUP);
    digitalWrite(Fan, HIGH);
    digitalWrite(Heat, HIGH);
    digitalWrite(Cool, HIGH);

    led = new LedControl(data, myclock, load, 4); //DIN,CLK,CS,HowManyDisplays
    led->shutdown(0, false); //Turn it on
    led->setIntensity(0, 1);
    led->shutdown(1, false);
    led->setIntensity(1, 1);
    led->shutdown(2, false);
    led->setIntensity(2, 1);
    led->shutdown(3, false);
    led->setIntensity(3, 1);
}

void loop()
{
    sensors.requestTemperatures();
    TempDebug = sensors.getTempCByIndex(0) * 1.8 + 32;
    if (TempDebug > 1) {
        Temp = TempDebug - 2; //note subracted integer was for personal sensor calibration as mine seemed to be off by 2 degrees
        TempStr = String(Temp);
    }
    TempControl();
    MotionSense();
    CheckButtons();
}

int GetDesiredTemp(String command) //Gets desired temperature from web 
{
    if (command != "") {           // Make sure string is not empty
        DesiredTempStr = command;
        DesiredTemp = command.toInt();
    }
    return DesiredTemp;
}

int GetEcoFactor(String command) //Gets Eco-Factor temperature from web 
{
    if (command != "") {          // Make sure string is not empty
        EcoFactorStr = command;
        EcoFactor = command.toInt();
    }
    return EcoFactor;
}

void TempControl()  // The smarts in controlling relays based on temperature
{
    if (SystemMode == 1) { //CAUTION a small eco-factor in auto mode could cause your systems heat and cool to infititly fight eachother
        if (Temp < DesiredTemp - EcoFactor) {
            HeatOn = true;
            ActiveMode = "Heating";
        }
        else if (Temp > DesiredTemp + EcoFactor) {
            CoolOn = true;
            ActiveMode = "Cooling";
        }
    }
    else if (SystemMode == 2) {
        if (Temp < DesiredTemp - EcoFactor) {
            HeatOn = true;
            ActiveMode = "Heating";
        }
    }
    else if (SystemMode == 3) {
        if (Temp > DesiredTemp + EcoFactor) {
            CoolOn = true;
            ActiveMode = "Cooling";
        }
    }
    else if (SystemMode == 4) {
        digitalWrite(Fan, LOW);
        digitalWrite(Heat, HIGH);
        digitalWrite(Cool, HIGH);
    }
    else if (SystemMode == 0) {
        digitalWrite(Fan, HIGH);
        digitalWrite(Heat, HIGH);
        digitalWrite(Cool, HIGH);
        ActiveMode = "Off";
    }

    //Heat or Cool to Desired temp then turn off (prevents relays from turning on and off too fast for HVAC also energy efficient similar to "pump and glide" concept in a car)
    if (HeatOn == true && Temp < DesiredTemp) {
        digitalWrite(Fan, LOW);
        digitalWrite(Heat, LOW);
        digitalWrite(Cool, HIGH);
    }
    else if (CoolOn == true && Temp > DesiredTemp) {
        digitalWrite(Fan, LOW);
        digitalWrite(Heat, HIGH);
        digitalWrite(Cool, LOW);
    }
    else {
        digitalWrite(Fan, HIGH);
        digitalWrite(Heat, HIGH);
        digitalWrite(Cool, HIGH);
        HeatOn = false;
        CoolOn = false;
        ActiveMode = "Off";
    }
}

void MotionSense()
{
    int MotionVal = digitalRead(MotionPin);
    if (MotionVal == HIGH){
        DisplayCurrentTemp();
        DisplayOn();
    }
    else {
        //DisplayOff();
        
    }
    
}



void CheckButtons()
{
    int UpVal = digitalRead(UpPin);
    int DownVal = digitalRead(DownPin);
    int ModeVal = digitalRead(ModePin);
    if (UpVal == LOW){;
        DesiredTemp += 1;
        DesiredTempStr = String(DesiredTemp);
        DisplayDesiredTemp();
        DisplayOn();
        //delay(100);
        
    }
    else if (DownVal == LOW){
        DesiredTemp -= 1;
        DesiredTempStr = String(DesiredTemp);
        DisplayDesiredTemp();
        DisplayOn();
        //delay(100);
    }
    else if (ModeVal == LOW){
        SystemMode += 1;
        //delay(100);
        if (SystemMode >= 5){
            SystemMode = 0;
        }
        SetSystemModeStr();
        DisplayCurrentMode();
        DisplayOn();
        
        
    }
    
    
}


void DisplayOn()
{
    led->shutdown(0, false); //Turn it on
    led->setIntensity(0, 1);
    led->shutdown(1, false);
    led->setIntensity(1, 1);
    led->shutdown(2, false);
    led->setIntensity(2, 1);
    led->shutdown(3, false);
    led->setIntensity(3, 1);


    //Particle.publish("message",message);
}



void DisplayCurrentTemp(){
    sprintf(message, TempStr + " F"); //update message
    led->setLetter(0, message[phase]);
    led->setLetter(1, message[phase + 1]);
    led->setLetter(2, message[phase + 2]);
    led->setLetter(3, message[phase + 3]);
    
}

void DisplayDesiredTemp(){
    sprintf(message, "S " + DesiredTempStr); //update message
    led->setLetter(0, message[phase]);
    led->setLetter(1, message[phase + 1]);
    led->setLetter(2, message[phase + 2]);
    led->setLetter(3, message[phase + 3]);
    
}

void DisplayCurrentMode(){
        sprintf(message, SystemModeStr);
        led->setLetter(0, message[phase]);
        led->setLetter(1, message[phase + 1]);
        led->setLetter(2, message[phase + 2]);
        led->setLetter(3, message[phase + 3]);
}



void DisplayOff()
{
        led->shutdown(0, true);
        led->shutdown(1, true);
        led->shutdown(2, true);
        led->shutdown(3, true);
}

int GetSystemMode(String command)
{
    
    if (command == "Auto") {
        SystemMode = 1;
        SetSystemModeStr();
        return 1;
    }
    else if (command == "Heat") {
        SystemMode = 2;
        SetSystemModeStr();
        return 1;
    }
    else if (command == "Cool") {
        SystemMode = 3;
        SetSystemModeStr();
        return 1;
    }
    else if (command == "Fan") {
        SystemMode = 4;
        SetSystemModeStr();
        return 1;
    }
    else if (command == "Off") {
        SystemMode = 0;
        SetSystemModeStr();
        return 1;
    }
    else {
        return -1;
    }
}

void SetSystemModeStr(){
    if (SystemMode == 0){
        SystemModeStr = "Off";
    }
    else if (SystemMode == 1){
        SystemModeStr = "Auto";
    }
    else if (SystemMode == 2){
        SystemModeStr = "Heat";
    }
    else if (SystemMode == 3){
        SystemModeStr = "Cool";
    }
    else if (SystemMode == 4){
        SystemModeStr = "Fan";
    }
    else {
        SystemModeStr = "Off";
        SystemMode = 0;
    }
    
    
    
}
