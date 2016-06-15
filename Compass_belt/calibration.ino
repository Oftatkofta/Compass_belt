//void calibration(){
  
//}
/*
Data needed:
 
N: x,y,x <- First calibration point
E: x,y,z <- Second calibration point
S: x,y,z <- Third calibration point
W: x, y, z <- Fourth calibration point
C: x, y, z <- average (center) point Cx = (Nx+Ex+Sx+Wx)/4...etc...

distance fuction sqrt(Xa*Xb+Ya*Yb+Za*Zb)
rotation function, matrix multiplyer
rotation finder: 
 */
void testInCircle(){
  //trigger motors, one at a time in a circle for testing
  digitalWrite(pinArray[test], HIGH);
  test++;
  if (test >8){
    test = 0;
  delay(300);
  }
}

int averageCoordinate(volatile int (*calArray)[3], int idx){
  /*
   * *ptr is a decayed pointer to the calibrationMatrix array
   * idx is the index for the coordinate to average x=0, y=1, z=2
  */
  int n, e, s, w;
  float avg;
  int row_idx = 0;
  n = calArray[0][idx];
  e = calArray[1][idx];
  s = calArray[2][idx];
  w = calArray[3][idx];

  avg = round((n+e+s+w)/(float)4);
  return (int)avg;
  }

float findPhi_z(int x, int y){
  float phi = -atan(-(float)x/(float)y);
  if (x*sin(phi) + y*cos(phi) < 0){
    phi += PI;  
  }
  return phi;
}

float findPhi_x(int z, int y){
  float phi = -atan((float)z/(float)y);
  if (y*cos(phi) - z*sin(phi) < 0){
    phi += PI;  
  }
  return phi;
}

float findPhi_y(int z, int x){
  float phi = -atan(-(float)z/(float)x);
  if (x*cos(phi) + z*sin(phi) > 0){
    phi += PI;  
  }
  return phi;
}
