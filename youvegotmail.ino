// This #include statement was automatically added by the Particle IDE.
#include <LedControl-MAX7219-MAX7221.h>

//Include libraries
#include "spark-dallas-temperature.h"
#include <OneWire.h>
// Data wire is plugged into pin D6 on Particle
#define ONE_WIRE_BUS D6
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.  
DallasTemperature sensors(&oneWire);

int GetDesiredTemp(String command); //Defines function to get desired temp from web
int GetSystemMode(String command); //Defines function to gets system mode
int GetEcoFactor(String command); //Defines function to get eco factor
String DesiredTempStr = "72";  //Sets tempertature string
String SystemMode = "Auto";//Sets System mode eg Heat, Cool, Auto, Fan 
String EcoFactorStr = "2";// Sets Eco Factor (for auto mode) default 2 string
String TempStr;
int DesiredTemp = 72; //Sets tempertature int
int EcoFactor = 2; // Sets Eco Factor (for auto mode) default 2 int
int Temp; //current temp
int TempDebug; //temp read from IC Debugged for EM interference
int Fan = D1; // fan realy hooked to pin D1
int Heat = D2; // heat relay hooked to pin D2
int Cool = D3; // cool relay hooked to pin D3

LedControl *led;
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
Particle.function("EcoFactor", GetEcoFactor);//Initializes Particle API

Particle.variable("DesiredTemp", DesiredTemp);
Particle.variable("SystemMode", SystemMode);
Particle.variable("CurrentTemp", Temp);
Particle.variable("EcoFactor", EcoFactor);


  sensors.begin(); //Initilaizes senor reading
  
  //Initialize Relays and set them to Normally open
  pinMode(Fan, OUTPUT);
  pinMode(Heat, OUTPUT);
  pinMode(Cool, OUTPUT);
  digitalWrite(Fan, HIGH);
  digitalWrite(Heat, HIGH);
  digitalWrite(Cool, HIGH);
  
    led = new LedControl(data,myclock,load,4); //DIN,CLK,CS,HowManyDisplays
    led-> shutdown(0,false); //Turn it on
    led-> setIntensity(0,1);
    led-> shutdown(1,false);
    led-> setIntensity(1,1);
    led-> shutdown(2,false);
    led-> setIntensity(2,1);
    led-> shutdown(3,false);
    led-> setIntensity(3,1);
}


void loop()
{ 
  sensors.requestTemperatures();  
  TempDebug = sensors.getTempCByIndex(0)*1.8+32;
  if (TempDebug > 1){
    Temp = TempDebug;
    TempStr = String(Temp);
  }
  
  
  
  //Particle.publish("Temperature",TempStr);
  //Particle.publish("Desired Temperature",DesiredTempStr);
  //Particle.publish("EcoFactor",EcoFactor);
  
  
  TempControl();
  Display();
}





int GetDesiredTemp(String command){
    if (command != ""){
    DesiredTempStr = command;
    DesiredTemp = command.toInt();
    
    }
    return DesiredTemp;
}

int GetEcoFactor(String command){
    if (command != ""){
    EcoFactorStr = command;
    EcoFactor = command.toInt();
    }
    return EcoFactor;
}

void TempControl(){
        if (SystemMode == "Auto")
{
  if (Temp < DesiredTemp - EcoFactor){
      digitalWrite(Fan, LOW);
      digitalWrite(Heat, LOW);
      digitalWrite(Cool, HIGH);
  }
    else if (Temp > DesiredTemp + EcoFactor){
      digitalWrite(Fan, LOW);
      digitalWrite(Heat, HIGH);
      digitalWrite(Cool, LOW);
  }
    else{
  digitalWrite(Fan, HIGH);
  digitalWrite(Heat, HIGH);
  digitalWrite(Cool, HIGH);
  }
}
else if (SystemMode == "Heat")
{
  if (Temp < DesiredTemp - EcoFactor){
      digitalWrite(Fan, LOW);
      digitalWrite(Heat, LOW);
      digitalWrite(Cool, HIGH);
  }
  else{
  digitalWrite(Fan, HIGH);
  digitalWrite(Heat, HIGH);
  digitalWrite(Cool, HIGH);
  }
  
}
else if (SystemMode == "Cool")
{
    if (Temp > DesiredTemp  + EcoFactor){
      digitalWrite(Fan, LOW);
      digitalWrite(Heat, HIGH);
      digitalWrite(Cool, LOW);
  }
  else{
    digitalWrite(Fan, HIGH);
    digitalWrite(Heat, HIGH);
    digitalWrite(Cool, HIGH);
  }
}
else if (SystemMode == "Fan")
{
    digitalWrite(Fan, LOW);
    digitalWrite(Heat, HIGH);
    digitalWrite(Cool, HIGH);
}
else
{
  
}
    
    
}

void Display(){


        sprintf(message,TempStr + " F"); //update message
        led->setLetter(0,message[phase]);
        led->setLetter(1,message[phase+1]);
        led->setLetter(2,message[phase+2]);
        led->setLetter(3,message[phase+3]);

        //Particle.publish("message",message);
    
}



int GetSystemMode(String command){
    SystemMode = command;

    if (command == "Auto")
{
    return 1;
}
else if (command == "Heat")
{
 return 1;
  
}
else if (command == "Cool")
{
   return 1;
}
else if (command == "Fan")
{
    return 1;
}
else
{
  return -1;
}
}



