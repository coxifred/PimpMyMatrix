/* 
  Remind for pin wiring on esp8266

static const uint8_t D0 = 16;
static const uint8_t D1 = 5;
static const uint8_t D2 = 4;
static const uint8_t D3 = 0;
static const uint8_t D4 = 2;
static const uint8_t D5 = 14;
static const uint8_t D6 = 12;
static const uint8_t D7 = 13;
static const uint8_t D8 = 15;
static const uint8_t RX = 3;
static const uint8_t TX = 1;
*/
static const uint8_t D1_MATRIX = 5;

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <Chrono.h>
#include <pgmspace.h>
#include <ESP8266mDNS.h>
#include "MemoryWebResources.h"
#include "EEPROMAnything.h"


#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))


// Data structure
typedef struct {
  char anIP[20];
  char aNetmask[20];
  char aGateway[20];
  char aWifiMode[20];
  char aIpConfig[20];
  char ssid[20];
  char password[20];
  uint16_t crc;
} Configuration __attribute__ ((packed));

// The configuration object
Configuration pimpConfig;

// Ip Adress Object
IPAddress ip;
// The Web server
ESP8266WebServer server(80);
// Some values
String ledValue="off";
String version="1.0";
String host="PimpMyMatrix";
String jsonScanWifi="";
int ledState = LOW;
int wifiFailure=0;

// A Chronograph 
Chrono myChrono;
unsigned long previousMillis = 0;
unsigned long previousMillisWifi = 0;
unsigned long previousMillisReboot = 0;

const long interval = 1000;
const long intervalWifi = 180000;
const long intervalReboot = 1000 * 60 * 60 * 1;
#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) )

/**
 
MATRIX DECLARATION:
Parameter 1 = width of EACH NEOPIXEL MATRIX (not total display)
Parameter 2 = height of each matrix
Parameter 3 = number of matrices arranged horizontally
Parameter 4 = number of matrices arranged vertically
Parameter 5 = pin number (most are valid)
Parameter 6 = matrix layout flags, add together as needed:

NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT: Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.  NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are arranged in horizontal rows or in vertical columns, respectively;  pick one or the other.
NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN EACH MATRIX proceed in the same order, or alternate lines reverse direction; pick one.
NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT: Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick  two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY are arranged in horizontal rows or in vertical columns, respectively; pick one or the other.
NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES (tiles) in the OVERALL DISPLAY proceed in the same order for every line, or alternate lines reverse direction; pick one.  When using zig-zag order, the orientation of the matrices in alternate rows will be rotated 180 degrees (this is normal -- simplifies wiring).

Parameter 7 = pixel type flags, add together as needed:
  NEO_RGB     Pixels are wired for RGB bitstream (v1 pixels)
  NEO_GRB     Pixels are wired for GRB bitstream (v2 pixels)
  NEO_KHZ400  400 KHz bitstream (e.g. FLORA v1 pixels)
  NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)


// Matrix setup params
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, D1,
NEO_MATRIX_TOP    + NEO_MATRIX_RIGHT +
NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE + NEO_MATRIX_ZIGZAG,
NEO_GRB            + NEO_KHZ800);
* 
 */
 Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, D1_MATRIX,NEO_MATRIX_TOP+NEO_MATRIX_LEFT+NEO_MATRIX_COLUMNS+NEO_MATRIX_PROGRESSIVE+NEO_MATRIX_ZIGZAG,NEO_GRB+ NEO_KHZ800);

  uint16_t color =  matrix.Color(255, 0, 255);
  int brightness = 80;

  int pixelPerChar = 4;
  int maxDisplacement;
  int x = matrix.width();
  int mode = 0;
  String message ="Welcome in pimpMyMatrix ";
  String pixels= "";
  String pixz="";
  String scrolling="true";
  int speed=70;

  /**
   * Some Help
  
  matrix.setTextWrap(false); 
  matrix.setBrightness(5);
  matrix.setTextColor(matrix.Color(255, 0, 0));
  matrix.setPixelColor(matrix.Color(255, 0, 0));
  matrix.fillScreen(0);    //Turn off all the LEDs
  matrix.drawPixel(x,y,matrix.Color(255, 0, 0));
  matrix.setPassThruColor(matrix.Color(255, 0, 0));
  https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h
  matrix.setCursor(x, 0);
  matrix.setTextSize(uint8_t s),
  matrix.print("Test");
  matrix.setFont(const GFXfont *f = NULL);

  */
  
  // End Matrix setup params




void setup() {
  Serial.begin(115200);

  Serial.println("Starting pimpMyMatrix...");
  myChrono.restart();
  
  Serial.println("Setup Matrix...");
  matrix.begin();
  matrix.setTextWrap(false);
  
 
  
  Serial.println("Reading Parameters...");
  readParameters();
 
  Serial.println("Setting Wifi...");
  setupWifi();

  Serial.println("Setup finished.");
}

void loop() {
  server.handleClient();
  delay(1);
  unsigned long currentMillis = millis();
  unsigned long currentMillisWifi = millis();
  unsigned long currentMillisReboot = millis();

  if(currentMillis - previousMillis >= interval && ledValue == "blink" ) {
        previousMillis = currentMillis;
        if (ledState == LOW)
        {
          ledState = HIGH;  // Note that this switches the LED *off*
        }
        else
        {
          ledState = LOW;   // Note that this switches the LED *on*
        }
        digitalWrite(LED_BUILTIN, ledState);
   }
   if(currentMillisWifi - previousMillisWifi >= intervalWifi ) {
       previousMillisWifi = currentMillisWifi;
       // Wifi Test, still there ? mode Station only
       if ( strcmp(pimpConfig.aWifiMode, "ST")  == 0) 
        {
          Serial.println("    * Heartbeat (every minute) Checking wifi status, when mode station selected");
          if ( WiFi.waitForConnectResult() )
          {
            Serial.println("    * Heartbeat (every minute) Wifi station Ok, nothing to do.");  
          }else
          {
            Serial.println("    * Heartbeat (every minute) Wifi station seems to be bad, reconnect...");
            setStationWifi();
          }
        }
   }
   if(currentMillisReboot - previousMillisReboot >= intervalReboot ) {
        previousMillisReboot = currentMillisReboot;
        Serial.println("   Reboot now, intervalReboot reached");
        ESP.restart();
     }

  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.setTextColor(color);
  matrix.setBrightness(brightness);
  //scroll(Text[mode],70);
  if ( ! message.equals("") )
  {
    if ( scrolling.equals("true") )
     {
      scroll(message,speed);
     } else
     {
      matrix.setCursor(0, 0);
      matrix.print(message);
      //matrix.show();
     }
  }
  if ( ! pixels.equals("") )
  {
    //matrix.fillScreen(0); 
    pixel();
  }
  matrix.show();
}



// this does the magic of scrolling
//void scroll(char* message,int delays) {
void scroll(String message,int delays) {
  maxDisplacement = message.length() * pixelPerChar + matrix.width();
 // matrix.print(String(message));
  matrix.print(message);
  if(--x < -maxDisplacement) {
    x = matrix.width();
    
    matrix.setTextColor(color);
    
  }
  //matrix.show();
  delay(delays);
}

void pixel() {
  // pixels=14,1,148:58:101;
  // pay load split
  //Serial.println("  Pixels : " + pixels);
  int maxIndex=matrix.width() * matrix.height();
  String aPixel="GO";
  for(int i=0; i<=maxIndex && ! aPixel.equals(""); i++){
    aPixel=getValue(pixels,'|',i);
    //Serial.println(String(i) + "/" + String(maxIndex) + " aPixel : " + aPixel);
    if ( ! aPixel.equals(""))
     {
      String xCoord=getValue(aPixel,',',0);
      int xPos=xCoord.toInt();
      String yCoord=getValue(aPixel,',',1);
      //Serial.println("  xCoord=" + xCoord + " yCoord=" + yCoord);
      int yPos=yCoord.toInt();
      //Serial.println("  xPos=" + String(xPos) + " yPos=" + String(yPos));
      String aColor=getValue(aPixel,',',2);
      //Serial.println("  aColor : " + aColor);
      int rColor=getValue(aColor,':',0).toInt();
      int gColor=getValue(aColor,':',1).toInt();
      int bColor=getValue(aColor,':',2).toInt();
      //Serial.println("  aColor : r=" + String(rColor) + " g=" + String(gColor) + " b=" + String(bColor));
      matrix.drawPixel(xPos,yPos,matrix.Color(rColor, gColor, bColor));
      //matrix.show();
     }
  } 
  
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length();

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}  // END


// Method for setting WIFI (Access Point or Station)
void setupWifi()
{
  scanWifi();
  Serial.println(" - Setting up wifi...");
  if ( strcmp(pimpConfig.aWifiMode, "AP")  == 0) 
  {
    Serial.println("    * Set access point mode");
    setAPWifi();
  }else
  {
    Serial.println("    * Set station mode");
    setStationWifi();
  }
}

// Reset config, to factory settings
void resetConfig()
{
  Serial.println("Reset SSID");
  sprintf_P(pimpConfig.ssid, "PimpMyMatrix-%06X", ESP.getChipId());
  //strcpy_P(pimpConfig.ssid, "PimpMyMatrix" + );
  Serial.println("Reset anIP");
  strcpy_P(pimpConfig.anIP, "192.168.1.1");
  Serial.println("Reset aNetmask");
  strcpy_P(pimpConfig.aNetmask, "255.255.255.0");
  Serial.println("Reset aGateway");
  strcpy_P(pimpConfig.aGateway, "192.168.1.254");
  Serial.println("Reset aWifiMode");
  strcpy_P(pimpConfig.aWifiMode, "AP");
  Serial.println("Reset aIpConfig");
  strcpy_P(pimpConfig.aIpConfig,"STATIC");
  Serial.println("Reset password");
  strcpy_P(pimpConfig.password, "");
}

// Read Parameters
void readParameters()
{
   EEPROM.begin(1024);
   if (readConfig(false)) {
    Serial.println("Good configuration");
   }else
   {
    Serial.println("Bad configuration, restore factory settings");
    memset(&pimpConfig, 0, sizeof(Configuration));
    resetConfig();
    saveConfig();
   }
   showConfig();
}

// Display configuration to COM PORT
void showConfig() 
{
  Serial.println("***** Wifi"); 
  
  Serial.print("ssid     : "); Serial.println(pimpConfig.ssid); 
  Serial.print("passwd   : "); Serial.println(pimpConfig.password); 
  Serial.print("wifiMode : "); Serial.println(pimpConfig.aWifiMode); 
  Serial.println("\r\n****** Network"); 
  Serial.print("ip       : "); Serial.println(pimpConfig.anIP); 
  Serial.print("netmask  : "); Serial.println(pimpConfig.aNetmask); 
  Serial.print("gateway  : "); Serial.println(pimpConfig.aGateway); 
  Serial.print("ipconfig : "); Serial.println(pimpConfig.aIpConfig); 
  Serial.println("");
}

// Save configuration
bool saveConfig (void) 
{
  Serial.println("Saving Configuration");
  uint8_t * pconfig ;
  bool ret_code;
  pconfig = (uint8_t *) &pimpConfig ;
  pimpConfig.crc = ~0;
  for (uint16_t i = 0; i < sizeof (Configuration) - 2; ++i)
    pimpConfig.crc = crc16Update(pimpConfig.crc, *pconfig++);
  pconfig = (uint8_t *) &pimpConfig ;
  for (uint16_t i = 0; i < sizeof(Configuration); ++i) 
  EEPROM.write(i, *pconfig++);
  EEPROM.commit();
  ret_code = readConfig(false);
  Serial.print(F("Write the configuration "));
  if (ret_code)
    Serial.println(F("OK!"));
  else
    Serial.println(F("Error!"));
  return (ret_code);
}

// Read Configuration 
bool readConfig (bool clear_on_error) 
{
  uint16_t crc = ~0;
  uint8_t * pconfig = (uint8_t *) &pimpConfig ;
  uint8_t data ;
  for (uint16_t i = 0; i < sizeof(Configuration); ++i) {
    data = EEPROM.read(i);
    *pconfig++ = data ;
    crc = crc16Update(crc, data);
  }
  if (crc != 0) {
    if (clear_on_error)
      memset(&pimpConfig, 0, sizeof( Configuration ));
    return false;
  }
  return true ;
}

// Update CRC
uint16_t crc16Update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; ++i)  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
}

// Led Request processing, allow on, off and blinking
void processLed()
  {
        ledValue=server.arg("state");
        if (ledValue == "on") digitalWrite(LED_BUILTIN, LOW);
        else if (ledValue == "off") digitalWrite(LED_BUILTIN, HIGH);
        server.send(200, "text/plain", "Led is now " + ledValue);
        Serial.print("Led is now ");
        Serial.println(ledValue);
  }

// Kowned informations process, returns as JSON format
void processInformations()
  {
        long elapsed = myChrono.elapsed();
        String json="{";
        json=json +"\"version\":\"" + version +"\",";
        json=json +"\"starttime\":" + elapsed + ",";
        json=json +"\"ip\":\"" + ipToString(WiFi.localIP()) + "\",";
        json=json +"\"netmask\":\"" + pimpConfig.aNetmask + "\",";
        json=json +"\"gateway\":\"" + pimpConfig.aGateway + "\",";
        json=json +"\"wifimode\":\"" + pimpConfig.aWifiMode + "\",";
        json=json +"\"ipconfig\":\"" + pimpConfig.aIpConfig + "\",";
        json=json +"\"ssid\":\"" + pimpConfig.ssid + "\",";
        json=json +"\"password\":\"" + pimpConfig.password + "\",";
        json=json +"\"currentMessage\":\"" + message + "\"";
        json=json +"}";
        server.send(200, "text/plain", json);
  }

// Set mode to access point wifi
void setAPWifi()
{
  
  Serial.println("    * Setting wifi access point starting by reseting factory settings");
  resetConfig();
  strcpy_P(pimpConfig.aWifiMode, "AP");
  if ( strcmp(pimpConfig.password, "") != 0 )
  {
    WiFi.softAP((const char*)pimpConfig.ssid,(const char*)pimpConfig.password);  
  }else
  {
    WiFi.softAP((const char*)pimpConfig.ssid);
  }
  
  int A_IP=atoi(subStr(pimpConfig.anIP, ".", 1));int B_IP=atoi(subStr(pimpConfig.anIP, ".", 2));int C_IP=atoi(subStr(pimpConfig.anIP, ".", 3));int D_IP=atoi(subStr(pimpConfig.anIP, ".", 4));
  int A_GW=atoi(subStr(pimpConfig.aGateway, ".", 1));int B_GW=atoi(subStr(pimpConfig.aGateway, ".", 2));int C_GW=atoi(subStr(pimpConfig.aGateway, ".", 3));int D_GW=atoi(subStr(pimpConfig.aGateway, ".", 4));
  int A_SN=atoi(subStr(pimpConfig.aNetmask, ".", 1));int B_SN=atoi(subStr(pimpConfig.aNetmask, ".", 2));int C_SN=atoi(subStr(pimpConfig.aNetmask, ".", 3));int D_SN=atoi(subStr(pimpConfig.aNetmask, ".", 4));

  IPAddress ip(A_IP, B_IP, C_IP, D_IP);
  IPAddress gateway(A_GW, B_GW, C_GW, D_GW);
  IPAddress subnet(A_SN, B_SN, C_SN, D_SN);
  WiFi.softAPConfig(ip, gateway, subnet);
  ip=WiFi.softAPIP();
  manageServerHttp();
  Serial.print("    * Ready in AP Wifi mode with SSID: [");
  message =  message + "Ready in AP Wifi mode with SSID:";
  Serial.print(pimpConfig.ssid);
  message =  message + pimpConfig.ssid;
  Serial.print("] password: [");
  Serial.print(pimpConfig.password);Serial.println("]");
  Serial.print(" - Open http://");
  Serial.print(ip);
  message =  message + " - Open http://" +  ipToString(ip) + " in your favorite browser";
  Serial.println(" in your favorite browser");
  server.send(200, "text/plain", ipToString(ip));
}

// Set mode to Station wifi
void setStationWifi()
{
 Serial.println("    * Setting wifi station");
 strcpy_P(pimpConfig.aWifiMode, "ST");
 showConfig();
 WiFi.mode(WIFI_STA);
 if ( strcmp(pimpConfig.password, "") != 0 )
 WiFi.begin((const char*)pimpConfig.ssid, (const char*)pimpConfig.password);

 if ( strcmp(pimpConfig.aIpConfig, "STATIC") == 0 )
 {
  int A_IP=atoi(subStr(pimpConfig.anIP, ".", 1));int B_IP=atoi(subStr(pimpConfig.anIP, ".", 2));int C_IP=atoi(subStr(pimpConfig.anIP, ".", 3));int D_IP=atoi(subStr(pimpConfig.anIP, ".", 4));
  int A_GW=atoi(subStr(pimpConfig.aGateway, ".", 1));int B_GW=atoi(subStr(pimpConfig.aGateway, ".", 2));int C_GW=atoi(subStr(pimpConfig.aGateway, ".", 3));int D_GW=atoi(subStr(pimpConfig.aGateway, ".", 4));
  int A_SN=atoi(subStr(pimpConfig.aNetmask, ".", 1));int B_SN=atoi(subStr(pimpConfig.aNetmask, ".", 2));int C_SN=atoi(subStr(pimpConfig.aNetmask, ".", 3));int D_SN=atoi(subStr(pimpConfig.aNetmask, ".", 4));
 
  IPAddress ip(A_IP, B_IP, C_IP, D_IP);
  IPAddress gateway(A_GW, B_GW, C_GW, D_GW);
  IPAddress subnet(A_SN, B_SN, C_SN, D_SN);
 
  WiFi.config(ip, gateway, subnet);
 }
 
 uint8_t timeout = 600; 
 Serial.print("    * Waiting connection aknowledge .");    
 while ( WiFi.waitForConnectResult() != WL_CONNECTED && timeout )
   {
    Serial.print(".");
    delay(500);
    --timeout;
   }
 Serial.println(".");
 if ( timeout <= 0)
 {
  Serial.println("Wifi connection failed, check ssid and password, reseting to access point");
  color =  matrix.Color(255, 0, 0);
  brightness=20;
  message="wifi ko";
  server.send(200, "text/plain", "KO");
 }else
 {
  MDNS.begin((const char*)host.c_str());
  manageServerHttp();
  MDNS.addService("http", "tcp", 80);
  ip=WiFi.localIP();
  Serial.print("    * Ready in Station Wifi mode with SSID: [");
  strcpy_P(pimpConfig.aNetmask,ipToString(WiFi.subnetMask()).c_str());
  strcpy_P(pimpConfig.aGateway,ipToString(WiFi.gatewayIP()).c_str());
  
  Serial.print(pimpConfig.ssid);
  Serial.print("] password: [");
  Serial.print(pimpConfig.password);Serial.println("]");
  Serial.print(" - Open http://");
  Serial.print(ip);
  saveConfig();
  server.send(200, "text/plain", ipToString(ip));
  brightness=10;
  color =  matrix.Color(0, 255, 0);
  message="wifi Ok";
 }
   
}

/**
 * Return a String from an IpObject
 */
String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

// HTTP Server request Management
void manageServerHttp()
{
    // Head HTTP Management
    server.on("/", HTTP_GET, [](){
      //server.sendHeader("Connection", "close");
      //server.setContentLength(strlen_P(DATA_index));
      //server.sendHeader("Cache-Control", "max-age=290304000, public");
      //server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("content-encoding","gzip");
      //server.sendContent_P( DATA_index, sizeof(DATA_index) );
      server.send_P(200,"text/html",DATA_index,sizeof(DATA_index));
    });
    
    // When Matrix Logo is requested
    server.on ("/logo.png", HTTP_GET, [](){
      server.setContentLength(strlen_P(DATA_logo));
      server.sendHeader("Cache-Control", "max-age=290304000, public");
      server.sendContent_P( DATA_logo, sizeof(DATA_logo) );
    });

     // When matrix POST is requested
    server.on ("/matrix", HTTP_POST, [](){
      if( server.hasArg("pixels") ) 
      {
        //Serial.print("Pixels parameters " + server.arg("pixels"));
        pixels=server.arg("pixels");
        pixels=getValue(pixels,'=',1);
        message="";
        server.send(200, "text/plain", "OK " + pixels);
      }
    });

     // When matrix GET is requested
     server.on ("/matrix", HTTP_GET, [](){
      if( server.hasArg("pixels")) 
      {
        //Serial.print("Pixels parameters " + server.arg("pixels"));
        pixels=server.arg("pixels");
        message="";
        server.send(200, "text/plain", "OK " + pixels);
      }
    });

    // Clean reboot request
    server.on("/reboot", HTTP_GET, [](){
      Serial.println("Reboot asked..");
      ESP.restart();
    });

    // Led management request
    server.on("/led", [](){
      processLed();
    });

    // Display knowned informations
    server.on("/info", [](){
      processInformations();
    });
    
    // Display knowned informations JSON
    server.on("/json", [](){
      processInformations();
    });

    // Reset factory settings
    server.on("/factory", [](){
      resetConfig();
      saveConfig();
      showConfig();
    });

    // Command to matrix
    server.on("/command",[](){
      if( server.hasArg("message") ) 
      {
       message=server.arg("message");
       
      }
      if( server.hasArg("scrolling") ) 
      {
       scrolling=server.arg("scrolling");
       
      }
      if( server.hasArg("speed") ) 
      {
       speed=server.arg("speed").toInt(); 
       
      }
      if( server.hasArg("color") ) 
      {
       int rColor=getValue(server.arg("color"),',',0).toInt();
       int gColor=getValue(server.arg("color"),',',1).toInt();
       int bColor=getValue(server.arg("color"),',',2).toInt();
       color =  matrix.Color(rColor, gColor, bColor);
       
      }

      if( server.hasArg("brightness") ) 
      {
       brightness=server.arg("brightness").toInt();
       
      }
     
      server.send(200, "text/plain", "OK " + message + " speed=" + String(speed) + " scrolling=" + scrolling + " color=" + String(color) + " brightness=" + String(brightness));
    });

    server.on("/scanWifi", [](){
      server.send(200, "text/plain", scanWifi());
    });
    
    // Wifi Access point management 
    server.on("/wifiAp", [](){
      strcpy_P(pimpConfig.ssid, server.arg("ssid").c_str());
      strcpy_P(pimpConfig.password, server.arg("passwd").c_str());
      strcpy_P(pimpConfig.aIpConfig, server.arg("ipconfig").c_str());
      setAPWifi();
    });

    // Wifi Station management
    server.on("/wifiStation",[](){
      strcpy_P(pimpConfig.ssid, server.arg("ssid").c_str());
      strcpy_P(pimpConfig.password, server.arg("passwd").c_str());
      strcpy_P(pimpConfig.aIpConfig, server.arg("ipconfig").c_str());
      setStationWifi();
    });

     
    
    // Network management (staticIP)
    server.on("/staticIp", [](){
      strcpy_P(pimpConfig.aGateway, server.arg("gateway").c_str());
      strcpy_P(pimpConfig.aNetmask, server.arg("netmask").c_str());
      strcpy_P(pimpConfig.anIP, server.arg("ip").c_str());
      strcpy_P(pimpConfig.aIpConfig, "STATIC");
      saveConfig();
      showConfig();
      Serial.println("Rebooting");
      ESP.restart();
    });

     // Network management (dhcpIP)
    server.on("/dhcpIp", [](){
      strcpy_P(pimpConfig.aGateway, "");
      strcpy_P(pimpConfig.aNetmask, "");
      strcpy_P(pimpConfig.anIP, "");
      strcpy_P(pimpConfig.aIpConfig, "DHCP");
      saveConfig();
      showConfig();
      Serial.println("Rebooting");
      ESP.restart();
    });


    // On update request, with upload file
    server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });

    // Starting webServer
    server.begin();
}

/**
 * Method for scanning wifi network SSID
 */
String scanWifi()
{
  Serial.println(" - Scanning wifi...");
  int Tnetwork = WiFi.scanNetworks();
  Serial.printf("    * %d network(s) found\n", Tnetwork);
  jsonScanWifi="{\"wifiNetworks\":[";
  for (int i = 0; i < Tnetwork; i++)
  {
    jsonScanWifi=jsonScanWifi + "{";
    jsonScanWifi=jsonScanWifi + "\"index\":" + (i+1) + ",";
    jsonScanWifi=jsonScanWifi + "\"name\":\"" + WiFi.SSID(i).c_str() + "\",";
    jsonScanWifi=jsonScanWifi + "\"channel\":" + WiFi.channel(i) + ",";
    jsonScanWifi=jsonScanWifi + "\"rssi\":" + WiFi.RSSI(i) + ",";
    jsonScanWifi=jsonScanWifi + "\"encrypt\":" + WiFi.encryptionType(i) + ",";
    jsonScanWifi=jsonScanWifi + "},";
  }
  jsonScanWifi=jsonScanWifi +"]}";
  jsonScanWifi.replace(",}","}");
  jsonScanWifi.replace(",]","]");
  Serial.println("    * " + jsonScanWifi);
  return jsonScanWifi;
}

/**
 * Allow a substring from a String
 */
char* subStr (char* input_string, char *separator, int segment_number) {
 char *act, *sub, *ptr;
 static char copy[20];
 int i;
 strcpy(copy, input_string);
 for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
  sub = strtok_r(act, separator, &ptr);
  if (sub == NULL) break;
 }
 return sub;
}
