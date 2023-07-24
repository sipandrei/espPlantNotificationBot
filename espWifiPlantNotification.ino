#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <credentials.h>

int soilSensorInput, soilSensorPercentage;
int baselineAir = 680;
int baselineWater = 280;
int minimumPercentage = 35;

int webNow;

IPAddress local_IP(192, 168, 100, 36);
IPAddress gateway(192,168,100,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);
const int DNS_PORT = 53;

ESP8266WebServer server(80);

void percentageLimiter(int &percentageReading){
  if(percentageReading < 0)
    percentageReading = 0;
    else if(percentageReading > 100)
     percentageReading = 100;
}

void configModeCallback(WiFiManager *wifiManager){
  Serial.println("entered config mode");
  Serial.println(WiFi.softAPIP());
}

/*void setMinimum(Control* sender, int type)
{
    minimumPercentage = sender->value.toInt();
}*/


void handleClient() {
  server.send(200, "text/html", "<h1>Test</h1>"); 
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  bool res;
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("");

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  res = wifiManager.autoConnect("RobotNotificarePlante", "parolabuna");
  wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet); // optional DNS 4th argument
  Serial.println(WiFi.localIP());

  /*if(res){
    //dnsServer.start(DNS_PORT,"*", local_IP);
    webNow = ESPUI.label("Umiditate Sol acum:", ControlColor::Emerald, "");
        ESPUI.number("Umiditate Minima Dorita", &setMinimum, ControlColor::Alizarin, minimumPercentage, 0, 10);

  }*/

  if(!res){
    Serial.print("DID NOT CONNECT");
    ESP.restart();
  } else {
    Serial.print("Ss");
    server.on("/", handleClient);
    server.begin();
  }
  //ESPUI.begin("UmiSol");
}

void loop() {
  // put your main code here, to run repeatedly:
  soilSensorInput = analogRead(A0);
  soilSensorPercentage = map(soilSensorInput,baselineAir,baselineWater, 0, 100);
  percentageLimiter(soilSensorPercentage);
  // Serial.println(soilSensorInput);
  // Serial.print(soilSensorPercentage);

  //ESPUI.print(webNow, String(soilSensorPercentage)+"%");

 
}
