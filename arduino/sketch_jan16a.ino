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
  //delay(1000);  //Pause
}
 
 
void loop()                     // run over and over again
{
  //delay(5000);
  readGPS();  //This is a function we define below which reads two NMEA sentences from GPS
}

void readGPS(){  //This function will read and remember two NMEA sentences from GPS
  
  clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out  
  while(!GPS.newNMEAreceived()) { //Keep reading characters in this loop until a good NMEA sentence is received
    c = GPS.read(); //read a character from the GPS
  }
  
  GPS.parse(GPS.lastNMEA());  //Once you get a good NMEA, parse it
  
  NMEA1 = GPS.lastNMEA();      //Once parsed, save NMEA sentence into NMEA1
  
  
  /**
  while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
    c = GPS.read();
  }
  
  GPS.parse(GPS.lastNMEA());  
  NMEA2 = GPS.lastNMEA();
  **/
  
  if(NMEA1[1] != '$' || NMEA1[2] != 'G' || NMEA1[3] != 'P' || NMEA1[4] != 'G'){
    Serial.println("invalid format!↓");
    Serial.println(NMEA1);
    return;
  }
  
  Serial.println("now this parse strings");
  Serial.println(NMEA1);

  // 
  char latBuf[20];
  char lngBuf[20];
  memset(latBuf, '\0', strlen(latBuf));
  memset(lngBuf, '\0', strlen(lngBuf));
  
  Serial.println("buf allocated");
  grepLat(',', 'S', 'N', latBuf);
  grepLng('S', 'N', 'W', 'E', lngBuf);
  
  /**
  char *lat = "39.93847";
  char *dummyLng = "138.9283";
  **/
  
  Serial.println("LAT");
  Serial.println(latBuf);

  Serial.println("LNG");
  Serial.println(lngBuf);
  double _tmplat = atof(latBuf);
  double _tmplng = atof(lngBuf);
  
  Serial.println("_tmplat");
  Serial.println(_tmplat);
  Serial.println("_tmplng");
  Serial.println(_tmplng);
  
  //double lat = round(_tmplat, 4);
  //double lng = round(_tmplng, 4);

  float lat = atof(latBuf);
  float lng = atof(lngBuf);
  Serial.println("double lat");
  Serial.println(lat);

  Serial.println("double lng");
  Serial.println(lng);
  
  //free(latBuf);
  //free(lngBuf);
  
  if(north == 0) {    
    lat = lat * (-1);
  }

  if(east == 0) {
    lng = lng * (-1);
  }
  
  sendIO(lat, lng);
}

void sendIO(float lat, float lng){
  Serial.println("sendIO start");
  checkQueue();
  
  count++;
  Serial.println("send lat...");
  Serial.println(lat);
  Serial.println("send lng...");
  Serial.println(lng);
  sakuraio.enqueueTx(0, lat);
  sakuraio.enqueueTx(1, lng);
  sakuraio.enqueueTx(2, count); 
  sakuraio.send();
}

double round( double val, int precision )
{
    /* ***** 内部変数定義 ***** */
    double    ret;
    char    buf[256] = {'\0'};
    char    *p;

    /* ***** 丸めを行い文字列に変換 ***** */
    sprintf( buf, "%.*f", precision, val );
    /* ***** 文字列から数値に再変換 ***** */
    ret = strtod( buf, &p );

    return ret;
}

void checkQueue(){
  Serial.println('checkQueue');
  uint8_t available;
  uint8_t queued;
  uint8_t ret = sakuraio.getTxQueueLength(&available, &queued);
  
  if (ret != CMD_ERROR_NONE){
    Serial.println("[ERR] enqueue error");
  }
  
  if (ret != 0){
    Serial.println("[ERR] queue stacked");
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

char *grepLng(char s1, char s2, char e1, char e2, char *buf){  
  //Serial.println("grepLng");
  const char *origin = NMEA1.c_str();

  int loopNumber = strlen(origin) + strlen("\n");
  
  int startIndex = 0;
  int finishIndex = 0;
  int connmaCount = 0;

  for(int i = 0; i < loopNumber; i++){
    if(origin[i] == ','){
      connmaCount++;  
    }
    if(origin[i] == ',' && connmaCount == 4){     
      startIndex = i + 1;
      if(origin[i - 1] == 'N'){
        north = 1;
      }
    }

    if(origin[i] == e1 || origin[i] == e2){
      if (connmaCount == 5){
        finishIndex = i - 1;
        if(origin[i] == 'E'){
          east = 1;  
        }
        break;  
      }
    }
  }

  //Serial.println("grepLng Start index decided...");
  //Serial.println(startIndex);

  //Serial.println("grepLng FinishIndex decided...");
  //Serial.println(finishIndex);
  int lSize = finishIndex - startIndex;
  
  //Serial.println("grep Lng lSize...");
  //Serial.println(lSize);
  
  //Serial.println("before buf");
  //Serial.println(buf);
  memcpy(buf, &origin[startIndex], lSize);  
  return buf;
}

char *grepLat(char s1, char e1, char e2, char *buf){
  //Serial.println("grepLat");
  const char *origin = NMEA1.c_str();

  int loopNumber = strlen(origin) + strlen("\n");
  int connmaCount = 0;
  int startIndex = 0;
  int finishIndex = 0;

  for(int i = 0; i < loopNumber; i++){
    if(origin[i] == ','){
      connmaCount++;
    }
    if(origin[i] == s1) {
      if (connmaCount == 2){  
        //Serial.println("startIndex Charactor...");
        //Serial.println(origin[i]);
        startIndex = i + 1;
      }
    }

    if(origin[i] == e1 || origin[i] == e2){
      //Serial.println("finishIndex Charactor...");
      //Serial.println(origin[i]);
      if (connmaCount == 3){
        finishIndex = i - 1;
        break;
      }
    }
  }
  //Serial.println("grepLat Start index decided...");
  //Serial.println(startIndex);

  //Serial.println("grepLat FinishIndex decided...");
  //Serial.println(finishIndex);
  if(startIndex == 0 || finishIndex == 0){  
      Serial.println("Invalid Start or Finish Index");
      return;
  }
  int lSize = finishIndex - startIndex;
  //Serial.println("before lat buf");
  //Serial.println(buf);
  //Serial.println("GrepLat lSize....");
  //Serial.println(lSize);
  memcpy(buf, &origin[startIndex], lSize);  
  return buf;
}


