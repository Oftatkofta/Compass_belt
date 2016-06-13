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


void centerCalibrationMatrix(){
  float mean_x, mean_y, mean_z;
  mean_x = calibrationMatrix[0];
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
