#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <credentials.h>
#include <Pinger.h>
#include <ESP8266WiFi.h>

extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}
int soilSensorInput, soilSensorPercentage;
int baselineAir = 680;
int baselineWater = 280;
int minimumPercentage = 35;

int webNow;

Pinger pinger;

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
//  wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet); // optional DNS 4th argument
  Serial.println(WiFi.localIP());

  /*if(res){
    //dnsServer.start(DNS_PORT,"*", local_IP);
    webNow = ESPUI.label("Umiditate Sol acum:", ControlColor::Emerald, "");
        ESPUI.number("Umiditate Minima Dorita", &setMinimum, ControlColor::Alizarin, minimumPercentage, 0, 10);

  }*/

  if(!res){
    Serial.print("DID NOT CONNECT");
    ESP.restart();}
//   else {
//    Serial.print("Ss");
//    server.on("/", handleClient);
//    server.begin();
//  }
  //ESPUI.begin("UmiSol");
    
  pinger.OnReceive([](const PingerResponse& response)
  {
    if (response.ReceivedResponse)
    {
      Serial.printf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
    }
    else
    {
      Serial.printf("Request timed out.\n");
    }

    // Return true to continue the ping sequence.
    // If current event returns false, the ping sequence is interrupted.
    return true;
  });
  
  pinger.OnEnd([](const PingerResponse& response)
  {
    // Evaluate lost packet percentage
    float loss = 100;
    if(response.TotalReceivedResponses > 0)
    {
      loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
    }
    
    // Print packet trip data
    Serial.printf(
      "Ping statistics for %s:\n",
      response.DestIPAddress.toString().c_str());
    Serial.printf(
      "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\n",
      response.TotalSentRequests,
      response.TotalReceivedResponses,
      response.TotalSentRequests - response.TotalReceivedResponses,
      loss);

    // Print time information
    if(response.TotalReceivedResponses > 0)
    {
      Serial.printf("Approximate round trip times in milli-seconds:\n");
      Serial.printf(
        "    Minimum = %lums, Maximum = %lums, Average = %.2fms\n",
        response.MinResponseTime,
        response.MaxResponseTime,
        response.AvgResponseTime);
    }
    
    // Print host data
    Serial.printf("Destination host data:\n");
    Serial.printf(
      "    IP address: %s\n",
      response.DestIPAddress.toString().c_str());
    if(response.DestMacAddress != nullptr)
    {
      Serial.printf(
        "    MAC address: " MACSTR "\n",
        MAC2STR(response.DestMacAddress->addr));
    }
    if(response.DestHostname != "")
    {
      Serial.printf(
        "    DNS name: %s\n",
        response.DestHostname.c_str());
    }

    return true;
  });
  
  // Ping default gateway
  Serial.printf(
    "\n\nPinging default gateway with IP %s\n",
    WiFi.gatewayIP().toString().c_str());
  if(pinger.Ping(WiFi.gatewayIP()) == false)
  {
    Serial.println("Error during last ping command.");
  }
  
  delay(10000);
}

void loop() {
  // put your main code here, to run repeatedly:
  soilSensorInput = analogRead(A0);
  soilSensorPercentage = map(soilSensorInput,baselineAir,baselineWater, 0, 100);
  percentageLimiter(soilSensorPercentage);
  // Serial.println(soilSensorInput);
  // Serial.print(soilSensorPercentage);
    Serial.printf("\n\nPinging google.com\n");
  if(pinger.Ping("google.com") == false)
  {
    Serial.println("Error during ping command.");
  }

    delay(3000);

  //ESPUI.print(webNow, String(soilSensorPercentage)+"%");

 
}
