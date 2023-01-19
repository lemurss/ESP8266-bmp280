/////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                 //
//                                                                                                 //
//               MeteoStation by me. Sketch prepared for using with ESP8266                        //
//               1. WebServer shouws online info from sensors                                      //
//               2. Posting to MQTT server data from sensors                                       //
//                              out/bmp280/temperature - MQTT Path for temperature sensor          //
//                              out/bmp280/humidity - MQTT Path for humidity sensor                //
//                              out/bmp280/pressure - MQTT Path for pressure sensor                //
//               3. Serial port duplicate info                                                     //
//                                                                                                 //
//               Using AHTX0                                                                       //
//                     BMP280                                                                      //
//                     ESP-12E or other NodeMCU comptable board                                    //
//               First start is runing WiFi AirPoint named START-CONNECT,                          // 
//               After connection, go 192.168.4.1 and lunch manager for configure settings:        //
//                     WiFi SSID                                                                   //
//                     WiFi Password                                                               //
//                     MQTT server adress                                                          //
//                     MQTT login                                                                  //
//                     MQTT Password                                                               //
//                     NarodMonitor* path                                                          //
//               *NarodMonitor - free monitoring system, collected info from many users sensors    //
//                https://narodmon.ru/ - main adress                                               //
//                                                                                                 //
//                                                                                                 //
//                                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <FS.h> 

String ssid = "Meteo";    //Ap SSID
String pass = "1234";     //Ap Password

String MQTTserv = "";     //MQTT Server
String MQTTlogin = "";    //MQTT Login
String MQTTpass = "";     //MQTT Password

String NarodMon = "";     //Info to Narodniy monitoring

//files for storing configuration in files area in ESP file system
const char* ssidPath = "/ssid.txt"; 
const char* passPath = "/pass.txt";

const char* MQTTservPath = "/MQTTserv.txt";
const char* MQTTloginPath = "/MQTTlogin.txt";
const char* MQTTpassPath = "/MQTTpass.txt";
const char* NarodMonPath = "/NarodMon.txt";

boolean restart = false;

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Set web server port number to 80
AsyncWebServer server(80);              
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Variable to store the HTTP request
String header;

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP280 bmp;

float temperature, humidity, pressure, altitude;

unsigned long lastMsg = 0;
unsigned long lastMsgToNM = 0;
unsigned long previousMillis = 0;
const long interval = 10000; //Сколько ждем перед открытием портала

#define debug true // вывод отладочных сообщений


void listDir(const char * dirname){
    Serial.printf("Listing directory: %s\r\n", dirname);
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
        Serial.print(dir.fileName());
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
}

const char main_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>Weather Station</title>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>
<style>
html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}
body{margin: 0px;} 
h1 {margin: 50px auto 30px;} 
.side-by-side{display: table-cell;vertical-align: middle;position: relative;}
.text{font-weight: 600;font-size: 19px;width: 200px;}
.reading{font-weight: 300;font-size: 50px;padding-right: 25px;}
.temperature .reading{color: #F29C1F;}
.humidity .reading{color: #3B97D3;}
.pressure .reading{color: #26B99A;}
.altitude .reading{color: #955BA5;}
.superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}
.data{padding: 10px;}
.container{display: table;margin: 0 auto;}
.icon{width:65px}
</style>
</head>
<body>
<h1>Weather Station</h1>
<div class='container'>
<div class='data temperature'>
<div class='side-by-side icon'>
<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982
C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718
c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833
c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22
s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>
</div>
<div class='side-by-side text'>Temperature</div>
<div class='side-by-side reading'>
%TEMPERATURE%
<span class='superscript'>&deg;C</span></div>
</div>
<div class='data humidity'>
<div class='side-by-side icon'>
<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>
</div>
<div class='side-by-side text'>Humidity</div>
<div class='side-by-side reading'>
%HUMIDITY%
<span class='superscript'>%</span></div>
</div>
<div class='data pressure'>
<div class='side-by-side icon'>
<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>
</div>
<div class='side-by-side text'>Pressure</div>
<div class='side-by-side reading'>%PRESURE%<span class='superscript'>mmHg</span></div>
</div>
<div class='data altitude'>
<div class='side-by-side icon'>
<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>
</div>
<div class='side-by-side text'>Altitude</div>
<div class='side-by-side reading'>120<span class='superscript'>m</span></div>
</div>
</div>
</body>
</html>)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
    <head>
        <title>Wi-Fi Manager</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
          html {
            font-family: Arial, Helvetica, sans-serif; 
            display: inline-block; 
          }

          h1 {
            font-size: 1.8rem; 
            color: white;
          }

          p { 
            font-size: 1.4rem;
          }

          .topnav { 
            overflow: hidden; 
            background-color: #0A1128;
          }

          body {  
            margin: 10;
          }

          .content { 
            padding: 5%;
          }

          .card-grid { 
            max-width: 800px; 
            margin: 0 auto; 
            display: grid; 
            grid-gap: 2rem; 
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
          }

          .card { 
            background-color: white; 
            box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
          }

          .card-title { 
            font-size: 1.2rem;
            font-weight: bold;
            color: #034078
          }

          input[type=submit] {
            border: none;
            color: #FEFCFB;
            background-color: #034078;
            padding: 15px 15px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            width: 100px;
            margin-right: 10px;
            border-radius: 4px;
            transition-duration: 0.4s;
            }

          input[type=submit]:hover {
            background-color: #1282A2;
          }

          input[type=text], input[type=password], input[type=number], select {
            width: 50%;
            padding: 12px 20px;
            margin: 18px;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
          }

          label {
            font-size: 1.2rem; 
          }
          .value{
            font-size: 1.2rem;
            color: #1282A2;  
          }
          .state {
            font-size: 1.2rem;
            color: #1282A2;
          }
          button {
            border: none;
            color: BLUE;
            padding: 15px 32px;
            text-align: center;
            font-size: 16px;
            width: 100px;
            border-radius: 4px;
            transition-duration: 0.4s;
          }
          .button:hover {
            background-color: #1282A2;
          }
        </style>
    </head>
    <body class="bg-dark text-light">
        <div class="position-absolute top-50 start-50 translate-middle">
            <div class="container-fluid">
                <div class="card text-dark bg-info mb-3">
                    <div class="card-header"><h2><center><b>Wi-Fi Manager</b></center></h2></div>
                    <div class="card-body">
                        <h5 class="card-title">
                            <form method="POST">
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Найдены сети. SSID</span><br>
                                    %LISTSSID%
                                </div>
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Пароль к WiFi сети:</span><br>
                                    <input type="password" name="pass" id="pass">
                                </div>
                                <h3>Для передачи данных по MQTT.(Заполнение по желанию)</h3>
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Сервер MQTT, IP:</span><br>
                                    <input type="text" name="mqttip" placeholder="192.168.0.55">
                                </div>
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Сервер MQTT, Login:</span><br>
                                    <input type="text" name="mqttlogin" placeholder="MqqtName">
                                </div>
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Сервер MQTT, Password:</span><br>
                                    <input type="password" name="mqttpass" id="pass">
                                </div>
                                <div class="input-group mb-3">
                                    <span class="input-group-text">Сервер Narod Mon, путь:</span><br>
                                    <input type="text" name="narodmon" id="pass">
                                </div>
                                <center><button type="submit" class="btn btn-success">Submit</button></center>
                            </form>
                        </h5>
                    </div>
                </div>
            </div>
        </div>
    </body>
</html>)rawliteral";

String st="";
// Replaces placeholder in HTML
String processor(const String& var){
  //Serial.println(var);
  if(var == "LISTSSID"){
    return st;
  }
  return String();
}

String processor_sensor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  if(var == "HUMIDITY"){
    return String(humidity);
  }
  if(var == "PRESURE"){
    return String(pressure);
  }
  return String();
}

String listSSID(){
  int Tnetwork=0,i=0,len=0;
  String s="";                    //String array to store the SSID's of available networks
  Tnetwork = WiFi.scanNetworks();       //Scan for total networks available
  st = "<select id=\"selectID\"  name=\"ssid\">";
  for (int i = 0; i < Tnetwork; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<option value=\""+WiFi.SSID(i)+"\">";
      st +=i + 1;
      st += ": ";
      st += WiFi.SSID(i);
      Serial.print("SSDI: ");
      Serial.println(WiFi.SSID(i));
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</option>";
    }
      st += "</select>";
  return st;  
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return "NotAble";
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}


bool SendToNarodmon() { // Собственно формирование пакета и отправка.
  String buf;
  buf = "#POKROV1OUT\n"; //mac адрес для авторизации датчика
  buf = buf + "#TEMPC#" + String(temperature) + "#Датчик температуры BMx280\n"; //показания температуры
  buf = buf + "#HUMID#" + String(humidity) + "#Датчик влажности BMx280\n"; //показания влажности
  buf = buf + "#PRESS#" + String(pressure) + "#Датчик давления BMx280\n"; //показания давления
  buf = buf + "#VCC#" + String(ESP.getVcc() + 350) + "#Напряжение батареи\n"; //показания температуры
  int WIFIRSSI=constrain(((WiFi.RSSI()+100)*2),0,100);
  buf = buf + "#WIFI#"  + String(WIFIRSSI) + "\n"; // уровень WIFI сигнала
  String worctime=String(millis());
  float WTime=worctime.toInt();WTime/=1000;
  buf = buf + "#WORKTIME#"  + String(WTime) + "#Время передачи данных" + "\n"; // уровень WIFI сигнала
  buf = buf + "##\n"; //окончание передачи

  // попытка подключения
  if (!wifiClient.connect("narodmon.ru", 8283)) {
    Serial.println("connection failed"); return false; // не удалось;
  }
  else  {
    wifiClient.print(buf); // и отправляем данные
    if (debug) Serial.print(buf);
    delay(100);// сделать 100 если нужен ответ или 10 если не нужен . Время активности увеличивается в 2 раза
    while (wifiClient.available()) {
      String line = wifiClient.readStringUntil('\r'); // если что-то в ответ будет - все в Serial
        if (debug){Serial.println(line);}
    }
  }
  return true; //ушло
}

void initMQTT(){
  Serial.println("Initializing MQTT");
  randomSeed(micros());
  client.setServer(MQTTserv.c_str(), 1883);
  connectMQTT();
}

void connectMQTT(){
    uint8_t retries=0;
    while(!client.connected() && retries<3)
    {
      Serial.print(".");
      retries++;
      String clientId = "Client-";
      clientId += String(random(0xffff), HEX);

      if(client.connect(clientId.c_str(),MQTTlogin.c_str(), MQTTpass.c_str())){
        Serial.println("Successfully connected MQTT");
      } else {
        Serial.println("Error!");
        Serial.println(client.state());
      }
      retries++;
      delay(1000);

  }
}

void setup() {
  uint8_t retries=0;
  Serial.begin(115200);
//  SendHTML(temperature,humidity,pressure,altitude)
  if (SPIFFS.begin()) {
      Serial.println("mounted file system");

      ssid = readFile(SPIFFS, ssidPath);
      pass = readFile(SPIFFS, passPath);

      MQTTserv = readFile(SPIFFS, MQTTservPath);
      MQTTlogin = readFile(SPIFFS, MQTTloginPath);
      MQTTpass = readFile(SPIFFS, MQTTpassPath);
      NarodMon = readFile(SPIFFS, NarodMonPath);
  } else {
      Serial.println("NOT mounted file system");
  }

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  Serial.println("BMP280 found");

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

//  listDir("/");

  if ( ssid.length() > 3 ) {  
    Serial.println(ssid);                             //Print SSID
    Serial.println(pass);                            //Print Password
    WiFi.begin(ssid.c_str(), pass.c_str());   //c_str()
    //Wait for WiFi to connect for a maximum timeout of 20 seconds
    while(WiFi.status()!=WL_CONNECTED && retries<20)
    {
      Serial.print(".");
      retries++;
      delay(1000);
    }
  
    Serial.println();
    //Inform the user whether the timeout has occured, or the ESP8266 is connected to the internet
    if(retries==20)//Timeout has occured
    {
      Serial.print("Unable to Connect to ");
      Serial.println(ssid);
    }
    
    if(WiFi.status()==WL_CONNECTED)//WiFi has succesfully Connected
    {
      Serial.print("Successfully connected to ");
      Serial.println(ssid);
      Serial.print("IP Address: ");
      IPAddress ip = WiFi.localIP();           //Get ESP8266 IP Adress
      Serial.println(ip);
      initMQTT();
      delay(1000);
        //response->printf(SendHTML(temperature,humidity,pressure,altitude));
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", main_html, processor_sensor);
      });
      Serial.println("Connected.");

    } else {
      WiFi.softAP("START-CONNECT", NULL); // Создаем открытую точку доступа с именем ESP-CONNECT
        
      IPAddress IP = WiFi.softAPIP();
      st = listSSID();
      Serial.print("AP IP address: ");
      Serial.println(IP); 
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
      });
      Serial.println("Connected.");
    
      server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == "ssid") {
              ssid = p->value().c_str();
              Serial.print("SSID set to: ");
              Serial.println(ssid);
              // Write file to save value
              writeFile(SPIFFS, ssidPath, ssid.c_str());
            }
            // HTTP POST pass value
            if (p->name() == "pass") {
              pass = p->value().c_str();
              Serial.print("Password set to: ");
              Serial.println(pass);
              // Write file to save value
              writeFile(SPIFFS, passPath, pass.c_str());
            }
            // HTTP POST MQTTserv value
            if (p->name() == "mqttip") {
              MQTTserv = p->value().c_str();
              Serial.print("MQTT Server Address set to: ");
              Serial.println(MQTTserv);
              // Write file to save value
              writeFile(SPIFFS, MQTTservPath, MQTTserv.c_str());
            }
            // HTTP POST MQTTlogin value
            if (p->name() == "mqttlogin") {
              MQTTlogin = p->value().c_str();
              Serial.print("MQTTlogin set to: ");
              Serial.println(MQTTlogin);
              // Write file to save value
              writeFile(SPIFFS, MQTTloginPath, MQTTlogin.c_str());
            }
            // HTTP POST MQTTpass value
            if (p->name() == "mqttpass") {
              MQTTpass = p->value().c_str();
              Serial.print("MQTTpass set to: ");
              Serial.println(MQTTpass);
              // Write file to save value
              writeFile(SPIFFS, MQTTpassPath, MQTTpass.c_str());
            }
            // HTTP POST NarodMon value
            if (p->name() == "narodmon") {
              NarodMon = p->value().c_str();
              Serial.print("NarodMon set to: ");
              Serial.println(NarodMon);
              // Write file to save value
              writeFile(SPIFFS, NarodMonPath, NarodMon.c_str());
            }
          }
        }
        restart = true;
        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address ");
      });    
    }
    server.begin();
  }
}

void loop(){
  if(WiFi.status()==WL_CONNECTED)//WiFi has succesfully Connected
  {
    unsigned long now = millis();
    if (now - lastMsg > 10000) {
      sensors_event_t h, temp;
      aht.getEvent(&h, &temp);// populate temp and humidity objects with fresh data
      
      temperature = bmp.readTemperature();
    
      humidity = h.relative_humidity;
      pressure = bmp.readPressure() * 0.007501;
      altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
      lastMsg = now;
      Serial.println("Publish: "); 
      Serial.print("Temperature: "); Serial.println(String(temperature).c_str());
      Serial.print("Humidity: "); Serial.println(String(humidity).c_str());
      Serial.print("Pressure: "); Serial.println(String(pressure).c_str());
      if(client.connected()){
        client.publish("out/bmp280/temperature", String(temperature).c_str());
        client.publish("out/bmp280/humidity", String(humidity).c_str());
        client.publish("out/bmp280/pressure", String(pressure).c_str());
      }
    }
    if (now - lastMsgToNM > 420000) {
      Serial.println("---------------------------------------------"); 
      Serial.println("Publish to NM: "); 
      if(WiFi.status() == WL_CONNECTED) {
        lastMsgToNM = now;
        SendToNarodmon();
      }
    }
  }
}
