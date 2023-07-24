#include <WiFiManager.h>

int soilSensorInput, soilSensorPercentage;
int baselineAir = 670;
int baselineWater = 280;
int minimumPercentage;

WiFiManager wifiManager;

void percentageLimiter(int &percentageReading){
  if(percentageReading < 0)
    percentageReading = 0;
    else if(percentageReading > 100)
     percentageReading = 100;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("");

  wifiManager.autoConnect("RobotNotificarePlante");
}

void loop() {
  // put your main code here, to run repeatedly:
  soilSensorInput = analogRead(A0);
  soilSensorPercentage = map(soilSensorInput,baselineAir,baselineWater, 0, 100);
  percentageLimiter(soilSensorPercentage);
  Serial.println(soilSensorInput);
  Serial.println(soilSensorPercentage);
  delay(1000);
}
