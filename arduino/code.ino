#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;        
char pass[] = SECRET_PASS;       

char server[] = "us-central1-loople2.cloudfunctions.net"; 

WiFiClientSecure client;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> 
#endif

#define PIN 13 
#define NUMPIXELS 13


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500

int tempo = 100;
int delayTime = 0;
void setup() {
  Serial.begin(115200);
  pixels.setBrightness(50);
  delay(1000); 
  Serial.println("Starting setup...");


  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    retries++;
    if (retries > 40) { 
      ESP.restart();
    }
  }
  Serial.println("\nConnected to WiFi!");
  printWifiStatus();

  client.setInsecure();

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); 
  fetchData();
  pixels.clear(); 

}

void fetchData(){
  client.stop();
  Serial.println("Fetching Data!");
  if (client.connect(server, 443)) {
    Serial.println("Connected to server!");
    client.println("GET /getData HTTP/1.1");
    client.println("Host: us-central1-loople2.cloudfunctions.net");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Connection failed.");
  }

  while (client.connected()) {
    if (client.available()) {
      tempo = read_response();
      delayTime = 60000/tempo;
    }
  }
  if (!client.connected()) {
    Serial.println("Disconnecting from server.");
    client.stop();
  }
}

int read_response() {
  uint32_t received_data_num = 0;
  String response = "";
  bool json_started = false;
  Serial.println("Reading response...");
  while (client.available()) {
    char c = client.read();
    if (c == '{') {
      json_started = true;
    }
    if (json_started) {
      response += c;
    }
    Serial.print(c);
    received_data_num++;
    if (received_data_num % 100 == 0) {
      Serial.println();
    }
  }
  Serial.println("\nResponse reading finished.");

  int data_start = response.indexOf("\"data\":") + 7;
  int data_end = response.indexOf("}", data_start);
  int data_value = 0;
  if (data_start > 6 && data_end > data_start) {
    String data_str = response.substring(data_start, data_end);
    data_value = data_str.toInt();
    Serial.print("Parsed data value: ");
    Serial.println(data_value);
  } else {
    Serial.println("Failed to parse the data value.");
  }
  return data_value;
}

void loop() {
  for(int i=0; i<NUMPIXELS; i+=2) { 
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.setPixelColor(i+1, pixels.Color(0, 150, 0));
    pixels.show();  
    delay(delayTime);
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.setPixelColor(i+1, pixels.Color(0, 0, 0));
    pixels.show(); 
    delay(delayTime);
  }
  RunningLights(0xff,0xff,0x00, delayTime);
  fetchData();
  RGBLoop();
  fetchData();
  fetchData();
  byte colors[3][3] = { {0xff, 0,0},
                    {0xff, 0xff, 0xff},
                    {0, 0, 0xff} };
  BouncingColoredBalls(3, colors);
}

void BouncingColoredBalls(int BallCount, byte colors[][3]) {
  float Gravity = -9.81;
  int StartHeight = 1;
 
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
 
  for (int i = 0 ; i < BallCount ; i++) {  
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
  }

  while (true) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
 
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
 
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUMPIXELS - 1) / StartHeight);
    }
 
    for (int i = 0 ; i < BallCount ; i++) {
      pixels.setPixelColor(Position[i],colors[i][0],colors[i][1],colors[i][2]);
    }
   
    pixels.show();
    setAll(0,0,0);
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setAll(int red, int green, int blue){
  for(int i=0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }
}

void RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
   
    for(int k = 0; k < 256; k+=50) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      pixels.show();
      delay(delayTime);
    }

    for(int k = 255; k >= 0; k-=50) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      pixels.show();
      delay(delayTime);
    }
  }
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
 
  for(int j=0; j<NUMPIXELS*2; j++)
  {
      Position++;
      for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
     
      pixels.show();
      delay(WaveDelay);
  }
}
