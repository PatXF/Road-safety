#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>

//data variables
float lat1,long1,lat2,long2,i1,j1,k1,d_lat1R,d_long1R,d_lat2R,d_long2R,d_lat3R,d_long3R,i2,j2,k2,i3,j3,k3;
int i=0,j=0,l;
int buttonState = 0;

//pin locations
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
int buzzer=8;
const int buttonPin = 2;

void parseAndSave(char *buff){
  char *name = strtok(buff, " =");
  if(name){
    char *junk = strtok(NULL, " ");
    if(junk){
      char *valu = strtok(NULL, " ");
      if(valu){
        float val = atof(valu);
        if(strcmp(name, "d_lat1") == 0){
          d_lat1R = val;
        }
        else if(strcmp(name, "d_long1") == 0){
          d_long1R = val;
        }
        if(strcmp(name, "d_lat2") == 0){
          d_lat2R = val;
        }
        else if(strcmp(name, "d_long2") == 0){
          d_long2R = val;
        }
        if(strcmp(name, "d_lat3") == 0){
          d_lat3R = val;
        }
        else if(strcmp(name, "d_long3") == 0){
          d_long3R = val;
        }
      }
    }
  }
}


//function to make sound from the buzzer
void make_sound(){
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
}

//function to find out the coordinates
float coordinates(float lat1,float long1,float lat2,float long2,float *x1,float *y1,float *z1,float *x2,float *y2,float *z2){
    int r=6371;
    *z1=r*sin(lat1*(3.1415926535/180));
    *y1=r*cos(lat1*(3.1415926535/180))*sin(long1*(3.1415926535/180));
    *x1=r*cos(lat1*(3.1415926535/180))*cos(long1*(3.1415926535/180));
    *z2=r*sin(lat2*(3.1415926535/180));
    *y2=r*cos(lat2*(3.1415926535/180))*sin(long2*(3.1415926535/180));
    *x2=r*cos(lat2*(3.1415926535/180))*cos(long2*(3.1415926535/180));
}

//function to find out the distance
float distance(float lat1,float long1,float lat2,float long2){
    float x1,x2,y1,y2,z1,z2,dist;
    coordinates(lat1,long1,lat2,long2,&x1,&y1,&z1,&x2,&y2,&z2);
    dist=sqrt(pow((z2-z1),2)+pow((y2-y1),2)+pow((x2-x1),2));
    return dist;
}

//function to find out the direction
float direct(float lat1,float long1,float lat2,float long2,float *i,float *j,float *k){
    float d;
    float x1,x2,y1,y2,z1,z2,dist;
    coordinates(lat1,long1,lat2,long2,&x1,&y1,&z1,&x2,&y2,&z2);
    d=distance(lat1,long1,lat2,long2);
    *i=x2-x1;
    *j=y2-y1;
    *k=z2-z1;
}
void alert(float d_lat,float d_long,float lat1,float long1,float lat2,float long2){
  if(distance(lat2,long2,d_lat,d_long)<=0.05&&distance(lat1,long2,lat2,long2)>=0.00694){
    direct(lat1,long1,lat2,long2,&i2,&j2,&k2);
    direct(lat2,long2,d_lat,d_long,&i3,&j3,&k3);
    if(i2<=(i3+0.002)&&i2>=(i3-0.002)&&j2<=(j3+0.002)&&j2>=(j3-0.002)&&k2<=(k3+0.002)&&k3>=(k3-0.002)){
      make_sound();
    }
  }
}
// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

//File object
File myFile;

void setup(){
  SD.begin(10);
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println("Intialising...");
  if(!SD.begin(10)){
    Serial.println("Initialisation Failed");
  }
  else Serial.println("Intialisation done");
  myFile = SD.open("DATA3.txt");
  if (myFile){
    char buffer[40]; // May need to be a bit bigger if you have long names
    byte index = 0;
    while (myFile.available()){
      char c = myFile.read();
      if(c == '\n' || c == '\r'){ // Test for <cr> and <lf>{
        parseAndSave(buffer);
        index = 0;
        buffer[index] = '\0'; // Keep buffer NULL terminated
      }
      else{
        buffer[index++] = c;
        buffer[index] = '\0'; // Keep buffer NULL terminated
      }
    }
  }
  Serial.print(",");
  Serial.print("dlat1 = ");
  Serial.print(d_lat1R,6);
  Serial.print(",");
  Serial.print("dlong1 = ");
  Serial.print(d_long1R,6);
  Serial.print(",");
  Serial.print("dlat2 = ");
  Serial.print(d_lat2R,6);
  Serial.print(",");
  Serial.print("dlong2 = ");
  Serial.print(d_long2R,6);
  Serial.print(",");
  Serial.print("dlat3 = ");
  Serial.print(d_lat3R,6);
  Serial.print(",");
  Serial.print("dlong3 = ");
  Serial.print(d_long3R,6);
  Serial.print("\n");
}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
  if (ss.available() > 0){
    if (gps.encode(ss.read())){
      displayInfo();
    }
    buttonState = digitalRead(buttonPin);
    if (buttonState==HIGH){
      delay(1000);
      Serial.println(",pressed button,");
      myFile=SD.open("DATA3.txt",FILE_WRITE);
      if(myFile){
        Serial.print("Writing to test.txt...");// if the file opened okay, write to it:
        if(j==0){
          myFile.print("d_lat1 = ");myFile.println(gps.location.lat(),6);
          myFile.print("d_long1 = ");myFile.println(gps.location.lng(),6);
        }
        if(j==1){
          myFile.print("d_lat2 = ");myFile.println(gps.location.lat(),6);
          myFile.print("d_long2 = ");myFile.println(gps.location.lng(),6);
        }
        if(j==2){
          myFile.print("d_lat3 = ");myFile.println(gps.location.lat(),6);
          myFile.print("d_long3 = ");myFile.println(gps.location.lng(),6);
        }
        myFile.close();Serial.println("done.");// close the file:
      }
      else{
         Serial.println("error opening test.txt");// if the file didn't open, print an error:
      }
    }
    j+=1;
  }
  if (millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

//function to display data in the serial monitor

void displayInfo(){
  Serial.print(F("Location: ")); 
  if (gps.location.isValid()){ 
    Serial.print(gps.location.lat(),6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(),6);
  }
  else{
    Serial.print(F("INVALID"));
  }
  Serial.println();
  delay(1000);
  
//  the 1st lat and long are stored as lat1 and lat2 and similarly lat2 and long 2, in the proceeding iterations the location1 becomes location2 and location2 becomes the data that is
//  read at the instant and so on

  if(i==0){
    lat1=gps.location.lat();
    long1=gps.location.lng();
    lat2=lat1;
    long2=long1;
  }
  if(i==1){
    lat2=gps.location.lat();
    long2=gps.location.lng();
  }
  else{
    lat1=lat2;
    long1=long2;
    lat2=gps.location.lat();
    long2=gps.location.lng();
  }
  for(l=0;l<3;l++){
    if(l==0){
      alert(d_lat1R,d_long1R,lat1,long1,lat2,long2);
    }
    if(l==1){
      alert(d_lat2R,d_long2R,lat1,long1,lat2,long2);
    }
    if(l==2){
      alert(d_lat3R,d_long3R,lat1,long1,lat2,long2);
    }
  }
//  printing distance per sec and direction per sec
  Serial.print(",Distance:");
  Serial.print(distance(lat1,long1,lat2,long2),8);
  Serial.print(",Direction:");
  direct(lat1,long1,lat2,long2,&i1,&j1,&k1);
  Serial.print(i1,8);
  Serial.print("i+");
  Serial.print(j1,8);
  Serial.print("j+");
  Serial.print(k1,8);
  Serial.print("k,");
  i+=1;
}
