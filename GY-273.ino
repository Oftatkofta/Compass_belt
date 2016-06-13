#include <Wire.h> //I2C Arduino Library

#define addr 0x1E //I2C Address for The HMC5883
int wLEDpin = 9;
int yLEDpin = 8;
int rLEDpin = 3;
int gLEDpin = 2;
int pinArray[] = {wLEDpin, yLEDpin, rLEDpin, gLEDpin};
float He;
int x,y,z; //triple axis data

void setup(){
  
  Serial.begin(9600);
  for (int i = 0; i < (sizeof(pinArray)/sizeof(int)); i++){
    pinMode(pinArray[i], OUTPUT);
    digitalWrite(pinArray[i], HIGH);
    delay(50);                  
    digitalWrite(i, LOW);    
  }
  
  Wire.begin();
  
  
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x02); // Set the Register
  Wire.write(0x00); // Tell the HMC5883 to Continuously Measure
  Wire.endTransmission();
  //Serial.println("X, Y, Z");
}


void loop(){

  //Tell the HMC what regist to begin writing data into
  Wire.beginTransmission(addr);
  Wire.write(0x03); //start with register 3.
  Wire.endTransmission();
  
 
 //Read the data.. 2 bytes for each axis.. 6 total bytes
  Wire.requestFrom(addr, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //MSB  x 
    x |= Wire.read(); //LSB  x
    z = Wire.read()<<8; //MSB  z
    z |= Wire.read(); //LSB z
    y = Wire.read()<<8; //MSB y
    y |= Wire.read(); //LSB y
  }

//  He = sqrt(sq((float)x)+sq((float)y)+sq((float)z));
  
  // Show Values
  Serial.print(x);
  Serial.print(' ');
  Serial.print(y);
  Serial.print(' ');
  Serial.println(z);
  delay(50);
}


