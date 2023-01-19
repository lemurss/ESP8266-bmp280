# ESP8266
Sketches for NodeMCU, ESP8266

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

![image](https://user-images.githubusercontent.com/75520956/213541662-a1823afd-71b7-4cad-8736-c49e84792fee.png)
