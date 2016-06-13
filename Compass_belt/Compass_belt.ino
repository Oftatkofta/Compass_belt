#include <Wire.h> //I2C Arduino Library

#define addr 0x1E //7-bit I2C Address for The HMC5883L
#define configurationRegisterA B01110100  //8-sample average, 30Hz readout, normal configuration
#define configurationRegisterB B00100000 // Default gain
#define modeRegister B00000000 // Continous mode
const int calibrationInterruptPin = 2;
const int Npin = 13; //5
const int NEpin = 12;
const int Epin = 11;
const int SEpin = 10;
const int Spin = 4;
const int SWpin = 5;
const int Wpin = 6;
const int NWpin = 7;
const int pinArray[] = {Npin, NEpin, Epin, SEpin, Spin, SWpin, Wpin, NWpin};
const int numberOfPins = sizeof(pinArray)/sizeof(int);
const float degreesPerPin = 360/(float)numberOfPins;
const float radiansPerPin = 2*PI/numberOfPins;
int x,y,z; //triple axis data
const float declinationAngle = 0.05; // about +2.75E for Oslo, in radians
float heading = 0;
float headingDegrees = 0;
//volatile float calibration = 0;
volatile int calibrationMatrix[4][3] = {
                              {0, 0, 0}, //N: {x,y,z}
                              {0, 0, 0}, //E: {x,y,z}
                              {0, 0, 0}, //S: {x,y,z}
                              {0, 0, 0} //W: {x,y,z}
                              };
volatile int calMatrixRowPointer = 0;
unsigned long button_time = 0;
unsigned long last_button_time = 0;
int i, j;
int test = 0;

void turnOffAllPins(){
    for (int i = 0; i < numberOfPins; i++){                  
      digitalWrite(pinArray[i], LOW);  
    }
}

void turnOnAllPins(){
    for (int i = 0; i < numberOfPins; i++){                  
      digitalWrite(pinArray[i], HIGH);  
    }
}

int headingToIndex(float heading){
  heading += radiansPerPin/2; //rotate to get clean breaks
  if (heading >= 2*PI) {
    return 0; //
  }
  else {
    return (int)round(heading/radiansPerPin);
  }
}


void calibrate(){
  /*Updates the calibration matrix one point at a time in the order
   * N, E, S, W, after which it wraps around to N again */
  button_time = millis();
  if (button_time - last_button_time > 250){
      calibrationMatrix[calMatrixRowPointer][0] = x;
      calibrationMatrix[calMatrixRowPointer][1] = y;
      calibrationMatrix[calMatrixRowPointer][2] = z;
      calMatrixRowPointer++;
      last_button_time = button_time;
        if (calMatrixRowPointer >= 4){
          calMatrixRowPointer = 0;
          printCalibrationMatrix();
        }
    }
}

void getCompassData(){
  //Tell the HMC what regist to begin writing data into
  Wire.beginTransmission(addr);
  Wire.write(0x03); //start with register 3.
  Wire.endTransmission();
 
 //Read the data.. 2 bytes for each axis.. 6 total bytes
  Wire.requestFrom(addr, 6);
  if (6<=Wire.available()) {
    x = Wire.read()<<8; //MSB  x 
    x |= Wire.read(); //LSB  x
    z = Wire.read()<<8; //MSB  z
    z |= Wire.read(); //LSB z
    y = Wire.read()<<8; //MSB y
    y |= Wire.read(); //LSB y
  }
}

void printCalibrationMatrix(){
  for (i=0; i<4; i++){
    for (j=0; j<3; j++){
      Serial.print(calibrationMatrix[i][j]);
      Serial.print("\t");
    }
    Serial.println();
  }
  //Serial.println();
}

void setup() {
  //Save Power by writing all (unused) Digital IO LOW - note that pins just need to be tied one way or another, do not damage devices!
  for (int i = 0; i < 18; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  
  //Add an interrupt fot the North calibration
  pinMode(calibrationInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(calibrationInterruptPin), calibrate, FALLING);
  
  /* The following saves some extra power by disabling some 
  peripherals not used. Disable the ADC by setting the ADEN
  bit (bit 7)  of the ADCSRA register to zero. */
  ADCSRA = ADCSRA & B01111111;

  /* Disable the analog comparator by setting the ACD bit
  (bit 7) of the ACSR register to one. */
  ACSR = B10000000;

  /* Disable digital input buffers on all analog input pins
  by setting bits 0-5 of the DIDR0 register to one.
  Of course, only do this if you are not using the analog 
  inputs for your project*/
  DIDR0 = DIDR0 | B00111111;
    
  Serial.begin(115200);

  turnOnAllPins();
  delay(500);
  
  Wire.begin();
  
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x00); // Set the Register
  //Write to configuration and mode registers
  Wire.write(configurationRegisterA); 
  Wire.write(configurationRegisterB);
  Wire.write(modeRegister);
  Wire.endTransmission();
  Serial.println("Start");
}

void loop() {

  //getCompassData();
  
  //heading = atan2(y, x); //Y-axis of magnetometer is pointing up
  
  //Correct for declination and calibration
  //heading -= calibration + declinationAngle;
  
  // Correct for when signs are reversed.
  //if(heading < 0)
    //heading += 2*PI;
    
  // Check for wrap due to addition of declination and calibration.
  //if(heading > 2*PI)
    //heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  //headingDegrees = heading * 180/PI; 
  
  turnOffAllPins();
  
  //int pinIndex = headingToIndex(heading);
  
  digitalWrite(pinArray[test], HIGH);
  test++;
  if (test >8){
    test = 0;
  }
  //Serial.print("x: ");
  //Serial.print(x);
  //Serial.print("\t");
  //Serial.print(" ");
  //Serial.print(y);
  //Serial.print("\t");
  //Serial.print(" ");
  //Serial.println(z);
  //Serial.print(" ");
  //Serial.println(headingDegrees);
  //Serial.print("\t");
  //Serial.print(pinIndex);
  //Serial.print("\t");
  //Serial.println(headingDegrees);
  delay(200);
  
}
