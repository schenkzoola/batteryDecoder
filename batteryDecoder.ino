#include <SPI.h>
#include <SD.h>

//Declare variables.

const int chipSelect = 10;

int i = 0;
long microsNow = 0;
long microsLast = 0;
long microsDiff = 0;
long millisNow = 0;
long millisLast = 0;
long millisDiff = 0;
const int batteryPin = 2;
bool batteryPinState = 0;
bool batteryPinLastState = 0;
byte dataFromBattery[3];
byte dataFromDevice[2];
bool resetPulseDetected = 0;
String dataString = "";

void setup() {
  pinMode(batteryPin, INPUT);  
  attachInterrupt(digitalPinToInterrupt(batteryPin), readPin, CHANGE);  
   
  Serial.begin(9600);   //start serial port at 9600bps
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
}

void readPin() {
  //interrupt service routine
    microsNow = micros();
    microsDiff = microsNow - microsLast;   //get the time difference since the last edge
    microsLast = microsNow;
    if (digitalRead(batteryPin) == 1) {
      switch (microsDiff) { //analyze the time to determine the data content
        case 50 ... 149:
          //This is a zero
          switch (i) {
            case 0 ... 7:
              bitClear(dataFromBattery[0], i);
            break;
            case 8 ... 15:
              bitClear(dataFromBattery[1], i - 8);
            break;
            case 16 ... 23:
              bitClear(dataFromBattery[2], i - 16);
            break;
            case 24 ... 31:
              bitClear(dataFromDevice[0], i - 24);
            break;
            case 32 ... 39:
              bitClear(dataFromDevice[1], i - 32);
            break;
          }
          i++;
        break;
        case 150 ... 259:
          //This is a one
          switch (i) {
            case 0 ... 7:
              bitSet(dataFromBattery[0], i);
            break;
            case 8 ... 15:
              bitSet(dataFromBattery[1], i - 8);
            break;
            case 16 ... 23:
              bitSet(dataFromBattery[2], i - 16);
            break;
            case 24 ... 31:
              bitSet(dataFromDevice[0], i - 24);
            break;
            case 32 ... 39:
              bitSet(dataFromDevice[1], i - 32);
            break;
          }
          i++;
        break;
        case 350 ... 450:
          //Reset Pulse
          resetPulseDetected = 1;
          i = 0;
        break;
      }
    }
}

void loop() {
    batteryPinState = digitalRead(batteryPin);
  if (batteryPinState != batteryPinLastState) {
    batteryPinLastState = batteryPinState;
    millisNow = millis();
    millisDiff = millisNow - millisLast;   //get the time difference since the last edge
    millisLast = millisNow;
  }
  if ((resetPulseDetected == 1) && (millisDiff > 150)) {
    dataString = (String("Battery:,")+dataFromBattery[0]+String(", ")+dataFromBattery[1]+String(", ")+dataFromBattery[2]+String(", Device:,")+dataFromDevice[0]+String(", ")+dataFromDevice[1]);
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }    
    dataFromBattery[0] = 0;
    dataFromBattery[1] = 0;
    dataFromBattery[2] = 0;
    dataFromDevice[0] = 0;
    dataFromDevice[1] = 0;
    resetPulseDetected = 0;
  }

  


    
}
