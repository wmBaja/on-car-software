/* ---- Includes ---- */
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

/* ---- Basic Variables ---- */
// Mode Variables
boolean testing = true;  //This gives true=more/false=less data via serial communication (set to false to do live data in Matlab) <-- False is currently broekn
boolean logging = true;

//Data Logger Shield Variables
const int chipSelect = 10;  // This is the chip set to use for an Adafriud SD Shield - This is also the digital pin being used
String delimiter = ", ";
boolean SDExists = false;
boolean fileSucessfullySaved = false;
String fileName = "dataLog.csv";
File myFile;

//Temperature Variables
#define ThermistorNumber 1   //Number of thermistors in the system
int SeriesResistor = 10000;  //Measured in ohms
float ThermistorRaw[ThermistorNumber];
double ThermistorResistance[ThermistorNumber];

//RPM Variables
// number of over threshold hall readings
int readings;
unsigned long startTime;
unsigned long endTime;
int hallCount = 0;
// sensor value of no magnetic field
int NO_FIELD = 505;
// Magnetic sensor threshold
int MAG_THRESH = 10;

//Telemetry Variables

// Multiple Use Variables
int Num_Samples = 5;  // This value is used for averaging in multiple functions
int primaryLoopDelay = 1000;  //This is the 1 second loop placed on the primary loop
int loopNum = 0;      // This variable is used to count the number of loops so itme can be estimated

//Pin Definition
int USBSerial = 9600;  // Speed of USB serial communications
int ThermistorPin[ThermistorNumber] = {A0};
int hallSensorPin = 0; // pin for hall sensor
int isDataWritingNo = 3;  //If this is on then the system is not recording data
int SDNoError = 4; //If this is on then there is no error in the SD Card System

/* ---- Initial Setup Code ---- */
void setup() {
  //Pin Definition
  pinMode(isDataWritingNo, OUTPUT);
  pinMode(SDNoError, OUTPUT);
  
  // Serial Communication Setup
  if (testing == true) {
    Serial.begin(USBSerial);
    Serial.println("!-S-USB Serial Started-!");
  }

  // Logging File Setup
  Serial.println("!-S-Initalizing SD card-!");
  if(SD.begin(chipSelect))
  {
    //Card exists and everything is good
    myFile = SD.open(fileName, FILE_WRITE);
    myFile.println("");
    myFile.close();
    SDExists = true;
    digitalWrite(isDataWritingNo,HIGH);
    digitalWrite(SDNoError,HIGH);
  }
  else
  {
    //Error Handling
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    //Card does not exist and everything is not good
    digitalWrite(isDataWritingNo,HIGH);
    digitalWrite(SDNoError,LOW);
  }
  
}

/* ---- Variable Definition Post Setup ---- */
//SD Card Data

/* ---- Additional Functions ---- */
// returns 0 for no field, negative for south pole, positive for north pole
int hallSense(){
  // sensor measurement (add correct input port)
  int raw = analogRead(hallSensorPin);

  long scaledMeasurement = raw - NO_FIELD;

  if(scaledMeasurement < MAG_THRESH && scaledMeasurement > -MAG_THRESH){
    return 0;
  } else {
    return scaledMeasurement;
  }
}

// Function handeling temperature sensor data collection
void temperature() {
  // Temp Variable
  int sample_count = 0;
  int i;
  float part1;
  float part2;
  
  // Get Data
  for (i = 0; i < ThermistorNumber; i++) {
    while (sample_count < Num_Samples) {
      ThermistorRaw[i] += analogRead(ThermistorPin[i]);
      sample_count++;
    }
  }

  //Calculate Values
  for (i = 0; i < ThermistorNumber; i++) {
    //---> Voltage
    ThermistorRaw[i] = ((ThermistorRaw[i]/sample_count)*(5.13/1023.0));   // Input voltage --> Ref voltage 5.13 this is a measured value
    //---> This part is broken // Temperature Conversion
    part1 = (SeriesResistor*ThermistorRaw[i]);
    part2 = (ThermistorRaw[i]-5.13);
    ThermistorResistance[i] = -1*(part1/part2);    //Modified voltage division equation to figure out the resistance of R1
  }
}

// functions for getting rpm from hall sensor
void logHallSensor(){
  if(hallSense != 0){
    readings++;
  }
  hallCount++;
}
void resetRPM(){
  hallCount = 0;
  startTime = millis();  
}

int calculateRPM() {
  endTime = millis();
  double elapsedTime = (endTime - startTime) / 1000.0;
  double rounds = readings / 2;
  double roundsPerSecond = rounds/elapsedTime;
  return (int) roundsPerSecond * 3600;  
}

// Function handeling serial (USB) communicatoin
// Message Structure !-Type of Message-Message-!
// Type of Message - (S, D) --> S = System Wide; D --> Car Data
void comsTesting() {
  //Loop Variable
  int i;

  //Header
  Serial.print("! - Loop: ");
  Serial.println(loopNum);

  //Data
  
  // Temperature Output
  Serial.println("!-D-Thermistor Data-!");
  for (i = 0; i < ThermistorNumber; i++) {
    Serial.print("Thermistor ");
    Serial.println(i);
    Serial.print("Thermistor ");
    Serial.print(i);
    Serial.print(" Raw - Volts: ");
    Serial.println(ThermistorRaw[i]);
    Serial.print("Thermistor ");
    Serial.print(i);
    Serial.print(" Resistance - Ohms: ");
    Serial.println(ThermistorResistance[i]);
  }

  //System Status
  
  //Status of Data Writing
  Serial.println("!-S-SD Card-!");  
  Serial.print("SD Exists: ");
  if(SDExists){
    Serial.println("true");
  }
  else
  {
    Serial.println("false");
  }
  if (fileSucessfullySaved)
  {
    Serial.println("Card Status: Data Saved");
  }
  else
  {
    Serial.println("Card Status: Data Not Saved");
  }
  
  //End of Message Break
  Serial.println("");
}

//Serial Coms used for live data
void comsNonTesting() {
  //Loop Variable
  int i;

  //Header
  Serial.print("*");
  Serial.print(loopNum);
  Serial.print(delimiter);

  //Data
  
  // Temperature Output
  for (i = 0; i < ThermistorNumber; i++) {
    Serial.print("TID");
    Serial.print(i);
    Serial.print(delimiter);
    Serial.print("V");
    Serial.print(ThermistorRaw[i]);
    Serial.print(delimiter);
    Serial.print("R");
    Serial.print(ThermistorResistance[i]);
    Serial.print(delimiter);
  }

  //System Status
  
  //Status of Data Writing
  Serial.print("SD");
  Serial.print(delimiter);  
  Serial.print("P");
  Serial.print(delimiter);
  if(SDExists){
    Serial.println("1"); // Exists
  }
  else
  {
    Serial.println("0"); //Does not exist
  }
  Serial.print(delimiter);
  Serial.print('S');
  Serial.print(delimiter);
  if (fileSucessfullySaved)
  {
    Serial.println("1");  // Data saved
  }
  else
  {
    Serial.println("0");  // Data not saved
  }
  Serial.print(delimiter);
  //End of Message Break
  Serial.println("");
}

//Save Data
void saveData(){
  //Variables
  int i = 0;
  fileSucessfullySaved = false;

  //Check to see if the file still exists and open it if it does
  if(myFile = SD.open(fileName, FILE_WRITE))
  {
    //LoopIndex
    myFile.print(loopNum);
    myFile.print(delimiter);
    
    //Add Thermistor Data
    for (i = 0; i < ThermistorNumber; i++) {
      myFile.print("Thermistor_");
      myFile.print(i);
      myFile.print(delimiter);
      myFile.print(ThermistorRaw[i]);
      myFile.print(delimiter);
      myFile.print(ThermistorResistance[i]);
      myFile.print(delimiter);
    }
    
    //Add RPM Data
  
    //Start new row and close the file
    myFile.println("");
    myFile.close();
  
    //Chage system states to show that everything is all good and writing
    digitalWrite(isDataWritingNo,LOW);
    fileSucessfullySaved = true;
  }
  else
  {
    //File could not be opened
     digitalWrite(isDataWritingNo,HIGH);
     digitalWrite(SDNoError,LOW);
  }
}

/* ---- Primary Loop ---- */
void loop() {
  //Reset Loop Num at 10000
  if(loopNum==10000)
  {
    loopNum = 0;
  }
  
  // Sensor Collection
  temperature();

  // Data Formatting


  // Data Saving (onboard SD Card)
  if (SDExists == true) { 
    saveData();
  }

  // Serial Communications
  if (testing == true) {
    comsTesting();
  }
  else
  {
    comsNonTesting();
  }

  //Delay and increase loop num
  delay(primaryLoopDelay);
  loopNum++;
}
