#include "Utilities.h"
#include <ArduinoJson.h> // para empacotar os dados em json pra enviar pra tago.io 
#include "EspMQTTClient.h" // pra fazer a conexão via protocolo MQTT
#include <Wire.h> // biblioteca pra fazer a coleta dos dados

#define SerialMon  Serial
#define SerialAT  Serial1

static const uint32_t ATBaud = 115200;
static const uint32_t MONBaud = 115200;

const int sensorIn = 32;
int mVperAmp = 100;
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
float vetM[10]={0,0,0,0,0,0,0,0,0,0};
int contM=0;

//variáveis para armazenar os valores em Json
char Amps[100];
char Watts[100];
char alert[100];

TaskHandle_t Task1;

EspMQTTClient client (
  "", // nome da rede
  "", // senha do wifi
  "mqtt.tago.io", // MQTT Broker server ip - padrão da tago.io
  "Default", // username
  "", // token
  "TestClient", // Client name 
  1883 // The MQTT port, default - padrão
);

void setup() {
  pinMode(sensorIn, INPUT);
  Serial.begin(9600);
  delay(5000);
}

// linha de configuração obrigatória  da biblioteca EspMQTTC
void onConnectionEstablished()
{}

float media (float m){
  float med=0;
  vetM[contM]=m;
  contM++;
  if(contM==10)
    contM=0;
  for(int i=0;i<10;i++)
    med=med+vetM[i];

  return med/10;
}
void loop() {

  Serial.println ("");
  Voltage = getVPP(); // Função que lê o valor analógico e transforma em valor de tensão
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = ((VRMS * 1000) / mVperAmp) - 0.35; // Calcula a corrente percorrida, dado o datasheet do sensor
  Watt = (AmpsRMS * 240 / 1.2);


  Serial.print(AmpsRMS);
  Serial.print(" Amps RMS  ---  ");
  Serial.print(Watt);
  Serial.print(" Watts ");

  Serial.print(" | ");
  Serial.print(media(Watt));
  int Mwatt = media(Watt);

  if ( Mwatt < 20 ){
    Serial.println(" OFF");
    client.publish("info/Alert", Alert);
  }else{
    Serial.println(" On");
  }

  // arquivo Json 

  StaticJsonDocument<300> amps;
  amps["variable"] = "Amps";
  amps["value"] = AmpsRMS;
  serializeJson(amps, Amps);

  client.publish("info/Amps", Amps); 
  
  StaticJsonDocument<300> watts;
  watts["variable"] = "Watts";
  watts["value"] = Mwatt;
  // watts["value"] = Watt;
  serializeJson(watts, Watts);

  client.publish("info/Watts", Watts); 

  StaticJsonDocument<300> alert;
  alert["variable"] = "Alert";
  alert["value"] = Alert;
  serializeJson(alert, Alert);

  delay(1000);
  client.loop();
}ESTEESTE

void atualizarSerialAT() {
  while (SerialMon.available())
  {
    SerialAT.write(SerialMon.read());
  }
  while (SerialAT.available())
  {
    SerialMon.write(SerialAT.read());
  }
}

float getVPP() {
  float result;
  int readValue;
  int maxValue = 0;
  int minValue = 4096;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000)
  {
    readValue = analogRead(sensorIn);
    if (readValue > maxValue)
    {
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      minValue = readValue;
    }
  }
  result = ((maxValue - minValue) * 5) / 4096.0;
  return result;
}