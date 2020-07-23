
/**************************************************************************
 * 
 * ESP8266 NodeMCU Extractor Miel nokia 5110 LCD 
 * AP == Punto de acceso. No necesita wifi
 * No tiene tiempo real 
 * 
 * No hay relé adicional D4 temporizado
 * 
 * NodeMCU      Nokia NK5110 LCD
 * D4 ----------> CLK
 * D3 ----------> Din
 * D2 ----------> DC
 * D1 ----------> CE
 * D0 ----------> RST
 * 
 * 
 * 
 *  PIN   ARDUINO 
 *  RST   Reset 3 
 *  CE       Chip Enable 4 
 *  DC       Data/Command  5 
 *  DIN      SPI IN  6 
 *  CLK      SPI Clock 7 
 *  VCC      3,3V  3.3V  Cuidado aquí, son 3,3 y no 5V
 *  BackLit  GND = Máximo brillo
 *  GND       Ground  GND 
 * 
 * 
 * 
 * D5 -----> prog.giro Derecha
 * D6 -----> prog.giro Izquierda
 * D7 -----> Botón
 * D8 -----> PWM output Referencia de frecuencia para giro 
 * 
 *************************************************************************/
 
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_PCD8544.h>  // include Adafruit PCD8544 (Nokia 5110) library
#include <FS.h>  
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>       
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <WiFiClient.h>

// Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);

// Defines
#define      DefaultName       "Extractor de Miel"  // Default device name
#define      Version           "1.00"             // Firmware version

// I/O config
//-----------

#define      Relay_dcha        D5   // (D4) 
#define      Relay_izda        D6   // (D5) 
#define      buttonPin         D7
#define      PWMPin            D8

// Variables
//----------


struct StoreStruct {
  int           modo;                       
  int           giro[10];    // variable de tiempo de retraso/adelanto para sunrise y sunset.
  int           activo[10]; // variable de tiempo aleatorio.
  int           tiempo[10]; //tiempos aleatorios procesados para control
  int           velocidad[10]; //tiempos aleatorios procesados para control
 } prog ;

boolean     estados[2];
boolean      WebButton       = false;
boolean      PgmPrev         = false;
boolean      PgmNext         = false;
boolean      PgmSave         = false;
boolean      Run=false;
boolean      Stop =true;
int          j;
boolean      ProgMielSave;
String        string1;
String        string2;
int           numero_paso;
long          tiempo_transcurrido;
boolean       Output_dcha = false;
boolean       Output_izda = false;
int           paso;
long         timeNow         = 0;
long         timeOld         = 300000;
long         timeOld2         = 0;
long         timeOld3         =0;
long         timeOld4         =0;
int buttonState = 0; 
int ref;
 
//SSID and Password to your ESP Access Point
const char* ssid = "Extractor_Miel_Fernando";
const char* password = "12345678";
 
ESP8266WebServer server(80); //Server on port 80
  
void setup(void)
{
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  setTime(DEFAULT_TIME); //tiempo simulado
  loadConfig();
  prog.modo=0;
  display.begin();
  display.setContrast(50);
  display.clearDisplay();   
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  Serial.begin(9600);
  
  pinMode(Relay_dcha,OUTPUT);
  pinMode(Relay_izda,OUTPUT);
  pinMode(PWMPin,OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(Relay_dcha,HIGH); //en los reles va ala revés
  digitalWrite(Relay_izda,HIGH);
 
  Serial.println("Comenzando programa");
 
  Serial.println("");
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
 
  IPAddress myIP = WiFi.softAPIP(); //Get IP address
  Serial.print("HotSpt IP:");
  Serial.println(myIP);
  server.on("/sendprog.json", sendprog);
  server.on("/saveprog", saveprog);
  server.on("/gpio", updateGpio);
  server.on("/readgpio.json", sendGPIO);
 
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.serveStatic("/html", SPIFFS, "/html");
  server.serveStatic("/", SPIFFS, "/index.html");
  server.begin();
  Serial.println ( "HTTP server started" );
  
}
 
void loop() {
  server.handleClient();  
  buttonState = digitalRead(buttonPin);

  
   if (buttonState == LOW && (millis()-timeOld3) > 1000) {
    timeOld3=millis();
    Serial.println("boton pulsado");
    Run=!Run;
    Stop=!Stop;
    prog.modo=Run;
  } 
  
  DoTimeCheckMiel();
  Miel_display();    // display estado de operación en el display Nokia 5110
  //sendDataProg();
}





void DoTimeCheckMiel() {

  timeNow = millis()/1000;

  if (prog.modo==2){
    numero_paso=0;
    timeOld2 = timeNow;
    tiempo_transcurrido=0;
    return;
    }
  
  // Stop mode
  if (Stop== true || Run!=true) {
    digitalWrite(Relay_dcha,LOW);estados[0]=0;
    digitalWrite(Relay_izda,LOW);estados[1]=0;
    analogWrite(PWMPin, 0);
    numero_paso=0;
    timeOld2 = timeNow;
    tiempo_transcurrido=0;
    return;
  }

 
     Output_dcha = prog.giro[numero_paso];
     Output_izda = !prog.giro[numero_paso];
   
    tiempo_transcurrido=timeNow-timeOld2;
    
    
    if( tiempo_transcurrido>prog.tiempo[numero_paso] ||!prog.activo[numero_paso] ){
     
      Serial.print("Numero de paso:");Serial.println(numero_paso);
      Serial.print("Velocidad de referencia map:"); Serial.println(ref);
      Serial.print("Velocidad de referencia:"); Serial.println(prog.velocidad[numero_paso]);
      numero_paso++;
        
      timeOld2 = timeNow;
      if(numero_paso>9){
        Stop=true;
        Run=false;
        prog.modo=0;
        Serial.println("Secuencia finalizada, modo Stop");
        }
        
        
        
        if (prog.giro[numero_paso]){
          Output_dcha = true;
          Output_izda = false;
          }
          else{
            Output_dcha = false;
            Output_izda = true;
            }
        }

        ref=prog.velocidad[numero_paso]*1023/100;
        analogWrite(PWMPin, ref);
         // Set output
        if (Output_dcha == true){
          digitalWrite(Relay_dcha,HIGH);estados[0]=1;
          digitalWrite(Relay_izda,LOW);estados[0]=0;
        }
        if (Output_izda == true){
          digitalWrite(Relay_dcha,LOW);estados[0]=0;
          digitalWrite(Relay_izda,HIGH);estados[1]=1;
        }
    
  
}
 
void Miel_display()
{


  if(millis()- timeOld4 <4000){
  return;
  }
  
  timeOld4=millis();
  display.clearDisplay();   // clear the screen and buffer
  display.setCursor(0, 0);

  if (paso){
  display.println("http://");
  display.println(WiFi.softAPIP());
  //display.println(" ");
  //display.println(" ");
  display.print("Estado: "); 
  if (Run) {display.println(" Run"); 
    display.print("Npaso:"); 
    display.println(numero_paso);
    display.print("Velocidad:"); display.println(prog.velocidad[numero_paso]);
  } else {
    display.println("Stop");}
    display.print("Modo: ");
    display.println(prog.modo);
    
  paso=false;
  }
  else{
    paso=true;
     //display.println(" ");
     //display.println(" ");
    display.print("Estado: "); 
    if (Run) {
        display.println(" Run");
        display.print("T_trans:"); display.println(tiempo_transcurrido);
        display.print("T_paso:"); display.println(prog.tiempo[numero_paso]);
        display.print("Npaso:"); display.println(numero_paso);
        display.print("Giro:"); display.println(prog.giro[numero_paso]);
        display.print("Vel.:"); display.println(prog.velocidad[numero_paso]);
      } else {
        display.println("Stop");
        display.println("Entre en la web para activar el programa");
        }
 } 
  display.display();
}


void sendprog(){
  DynamicJsonDocument doc(9500);
  JsonArray programas = doc.createNestedArray("prog");
          doc["modo"]= prog.modo; //0:stop,1:run,2:Manual

          for (j=0;j<10;j++){ 
            DynamicJsonDocument progs(1900);
            
            progs["tiempo"] =prog.tiempo[j];
            progs["giro"] =prog.giro[j];
            progs["activo"] =prog.activo[j];
            progs["velocidad"] =prog.velocidad[j];
            programas.add(progs);
          }
         String json;
         serializeJson(doc, json);
 
  server.send(200, "application/json", json);
  Serial.println("Datos de progrmas enviados");
  Serial.println(json);
        
 }


 void saveprog(){
        Serial.println("Recibido comando saveprog");
http://192.168.1.84/
//save?//Request URL:
//saveprog?cmd=saveprog&modo=1&tiempo0=123&giro0=0&activo0=1&tiempo1=124&giro1=1&activo1=1&tiempo2=0&giro2=0&activo2=0&tiempo3=0&giro3=0&activo3=0&tiempo4=0&giro4=0&activo4=0&tiempo5=0&giro5=0&activo5=0&tiempo6=0&giro6=0&activo6=0&tiempo7=0&giro7=0&activo7=0&tiempo8=0&giro8=0&activo8=0&tiempo9=0&giro9=0&activo9=0
int f;

String cadena;

  for(j=0;j<10;j++){

    cadena="tiempo";    cadena+=j;
    prog.tiempo[j] = server.arg(cadena).toInt();
    cadena="giro";    cadena+=j;
    prog.giro[j] = server.arg(cadena).toInt();
    cadena="activo";    cadena+=j;
    prog.activo[j] = server.arg(cadena).toInt();
    cadena="velocidad";    cadena+=j;
    prog.velocidad[j] = server.arg(cadena).toInt();
    
    Serial.print(" prog.tiempo[j]");Serial.println( prog.tiempo[j]);
  }
  prog.modo = server.arg("modo").toInt();
  if (prog.modo==0){Stop=true;Run=false; Serial.println("Recibido modo STOP");}
  if (prog.modo==1){Run=true;Stop=false;Serial.println("Recibido modo RUN");}
  if (prog.modo==2){Stop=true;Run=false;Serial.println("Recibido modo MANUAL");}
     
    saveConfig();
  
    
  String success = "1";
  String json = "{\"name\":\"" + String(prog.giro[0]) + "\",";
  json += "\"success\":\"" + String(success) + "\",";
  json += "\"prog.tiempo[0]\":\"" + String(prog.tiempo[0]) + "\"}";
  server.send(200, "application/json", json);
}

void loadConfig() {
       if(!SPIFFS.begin()){
        Serial.println("Card Mount Failed");
        return;
        }
        File myFile = SPIFFS.open("/conf.txt", "r");
        myFile.read((byte *)&prog, sizeof(prog));
        myFile.close();
        Serial.println("leyendo configuracion");
}


void saveConfig() {
  Serial.println("saving config");
   if(!SPIFFS.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    File myFile = SPIFFS.open("/conf.txt", "w");
    myFile.write((byte *)&prog, sizeof(prog));
    myFile.close(); 
    Serial.println("guardando configuracion");
  
}


void updateGpio(){
  String gpio = server.arg("id");
  String estado = server.arg("estado");
  String success = "1";
  int pin = D5;
  int num_pin=0;

  prog.modo=2; Stop=true; Run=false; //se pasaa a modo manual
  Serial.print("Recibido Estado"); Serial.println(estado);
 if ( gpio == "D5" ) {
        if (estado== "1"){
          digitalWrite(Relay_dcha,HIGH);estados[0]=1;
          digitalWrite(Relay_izda,LOW);estados[1]=0;
          }else{
            digitalWrite(Relay_dcha,LOW);estados[0]=0;
            }
 }
 if ( gpio == "D6" ) {
  if (estado== "1"){  
    digitalWrite(Relay_dcha,LOW);estados[0]=0;
    digitalWrite(Relay_izda,HIGH);estados[1]=1;
    }else{
            digitalWrite(Relay_izda,LOW);estados[1]=0;
            }
  }
  
  
  String json = "{\"gpio\":\"" + String(gpio) + "\",";
  json += "\"estado\":\"" + String(estado) + "\",";
  json += "\"success\":\"" + String(success) + "\"}";
    
  server.send(200, "application/json", json);
  Serial.println("GPIO Salidas digitales actuadas");
}

void sendGPIO() {
  String json = "{\"D5\":\"" + String(estados[0]) + "\",";
  json += "\"D6\":\"" + String(estados[1]) + "\"}";

  server.send(200, "application/json", json);
  Serial.println("GPIO enviado");
  Serial.println(json);
}
