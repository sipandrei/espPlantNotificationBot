#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(PLANT_NOTE_BOT, secured_client);
const unsigned long SCAN_DELAY = 1000;
unsigned long lastScan;

extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}
int soilSensorInput, soilSensorPercentage;
int baselineAir = 680;
int baselineWater = 280;
int minimumPercentage = 35;
bool watered = true;
bool waitMin = false;

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

void messagesHandler(int numNewMessages){
 Serial.printf("\n Received %d new messages \n", numNewMessages);
  String response;
  for(int i = 0; i < numNewMessages; i++){
    telegramMessage &msg = bot.messages[i];
    Serial.print("Message " + msg.text);
    Serial.print(numNewMessages);
    Serial.print(i);
    if(msg.text == "/setmin" || msg.text == "/setmin@plantnote_bot"){
        waitMin = true;
        response = "Set the minimum humidity  by replying with a number between 5 and 90";
    } else if(msg.text.toInt() >= 5 && msg.text.toInt() <= 90 && waitMin == true){
        waitMin = false;
        minimumPercentage = msg.text.toInt();
        response = "Minimum percentage set to " + msg.text;
      } else if(msg.text == "/humidity" || msg.text == "/humidity@plantnote_bot"){
          response = "Current humidity is " + String(soilSensorPercentage) + "%"; 
        } else {
        response = "That was not a valid input";
      }
    bot.sendMessage(msg.chat_id, response, "Markdown");
  }
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
  Serial.println(WiFi.localIP());

  if(!res){
    Serial.print("DID NOT CONNECT");
    ESP.restart();}
   else {
    secured_client.setTrustAnchors(&cert); 
    Serial.print("Retrieving time: ");
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    while (now < 24 * 3600)
    {
      Serial.print(".");
      delay(100);
      now = time(nullptr);
    }
    Serial.println(now);
    Serial.print("Sending message to ");
    Serial.println(CHAT_ID);
    bot.sendMessage(CHAT_ID, "Plant bot started, minimum moisture - 35%", "");
  }
}

void loop() {
  soilSensorInput = analogRead(A0);
  soilSensorPercentage = map(soilSensorInput,baselineAir,baselineWater, 0, 100);
  percentageLimiter(soilSensorPercentage);
    
  if(millis() - lastScan > SCAN_DELAY){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    Serial.print(numNewMessages);
    while (numNewMessages)
    {
      Serial.println("got response");
      messagesHandler(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Serial.print(numNewMessages);
    lastScan = millis();
  }
  
  if(waitMin == false){
    if(soilSensorPercentage < minimumPercentage && watered == true){
     watered = false;
      Serial.print("Plant needs water!! Current humidity: ");
      Serial.println(soilSensorPercentage);
      bot.sendMessage(CHAT_ID, "*ALERT!! PLANT NEEDS WATER!!* Current percentage: ", "Markdown");
      bot.sendMessage(CHAT_ID, String(soilSensorPercentage),"");
   } else if(soilSensorPercentage >= minimumPercentage && watered == false){
      watered = true;
      Serial.print("Plant plant *watered*! Current humidity: ");
      Serial.println(soilSensorPercentage);
      bot.sendMessage(CHAT_ID, "Plant *watered*!", "");
    }
  } 
}
