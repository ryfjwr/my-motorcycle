//Make sure to install the adafruit GPS library from https://github.com/adafruit/Adafruit-GPS-Library
#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library form the adafruit site above
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
#include <SakuraIO.h>

#define CMD_ERROR_NONE             0x01

SakuraIO_I2C sakuraio;

SoftwareSerial mySerial(3, 2); //Initialize SoftwareSerial, and tell it you will be connecting through pins 2 and 3
Adafruit_GPS GPS(&mySerial); //Create GPS object
 
String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence
char c;       //Used to read the characters spewing from the GPS module
uint32_t count;

// プラスかマイナスかを区別する
// 北緯か東経なら プラスになる
// https://qiita.com/tag1216/items/0b38ee5aedea0ef4a058#%E5%9F%BA%E7%A4%8E%E7%9F%A5%E8%AD%98
int north = 0;
int east = 0;
 
void setup(){
  Serial.begin(115200);  //Turn on the Serial Monitor
  GPS.begin(9600);       //Turn GPS on at baud rate of 9600
  GPS.sendCommand("$PGCMD,33,0*6D"); // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  delay(1000);  //Pause
}
 
 
void loop()                     // run over and over again
{
  delay(5000);
  readGPS();  //This is a function we define below which reads two NMEA sentences from GPS
}

void readGPS(){  //This function will read and remember two NMEA sentences from GPS
  
  clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out  
  while(!GPS.newNMEAreceived()) { //Keep reading characters in this loop until a good NMEA sentence is received
    c = GPS.read(); //read a character from the GPS
  }
  
  GPS.parse(GPS.lastNMEA());  //Once you get a good NMEA, parse it
  
  NMEA1 = GPS.lastNMEA();      //Once parsed, save NMEA sentence into NMEA1
  
  while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
    c = GPS.read();
  }
  
  GPS.parse(GPS.lastNMEA());
  
  NMEA2 = GPS.lastNMEA();
   
  Serial.println("=====NMEA1======↓");
  Serial.println(NMEA1);

  char *lat = grepLatOrLng('A', 'V', 'S', 'N');
  char *lng = grepLatOrLng('S', 'N', 'W', 'E');
  
  char *dummyLat = "39.93847";
  char *dummyLng = "138.9283";
  
  Serial.println("LAT");
  Serial.println(lat);

  Serial.println("LNG");
  Serial.println(lng);

  float _dummyLat = strtod(dummyLat, NULL);
  float _dummyLng = strtod(dummyLng, NULL);

  if(north == 0) {
    _dummyLat = _dummyLat * (-1);
  }

  if(east == 0) {
    _dummyLng = _dummyLng * (-1);
  }
  
  sendIO(_dummyLat, _dummyLng);
}

void sendIO(float lat, float lng){
  checkQueue();
  
  count++;
  sakuraio.enqueueTx(0, lat);
  sakuraio.enqueueTx(1, lng);
  sakuraio.enqueueTx(2, count);
  sakuraio.send();
}

void checkQueue(){
  uint8_t available;
  uint8_t queued;
  uint8_t ret = sakuraio.getTxQueueLength(&available, &queued);
  
  if (ret != CMD_ERROR_NONE){
    Serial.println("[ERR] enqueue error");
  }
  
  if (ret != 0){
    sakuraio.clearTx();
  }
}

void clearGPS() { 
  //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
  while(!GPS.newNMEAreceived()) {
    c = GPS.read();
  }
  
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived()) {
    c = GPS.read();
  }
  
  GPS.parse(GPS.lastNMEA());
}

char *grepLatOrLng(char s1, char s2, char e1, char e2){
  const char *origin = NMEA1.c_str();

  int loopNumber = strlen(origin) + strlen("\n");
  
  int startIndex = 0;
  int finishIndex = 0;

  for(int i = 0; i < loopNumber; i++){
    if(origin[i] == s1 || origin[i] == s2) {
      startIndex = i + 2;
      if(origin[i] == 'N'){
        north = 1;
      }
    }

    if(origin[i] == e1 || origin[i] == e2){
      finishIndex = i - 1;
      if(origin[i] == 'E'){
        east = 1;  
      }
      break;  
    }
  }
  if(startIndex == 0 || finishIndex == 0){  
      return;
  }
  Serial.println('hoge');
  int lSize = finishIndex - startIndex;
  char *l = (char *)malloc(lSize) + 1;
  
  memcpy(l, &origin[startIndex], lSize);  
  return l;
}


