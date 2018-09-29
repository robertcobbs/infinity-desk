#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#define DATA_PIN 4
//#define NUM_LEDS 177
#define NUM_LEDS 30
#define MAX_BRIGHTNESS 100

ESP8266WebServer server(80);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

int display_mode = 1;
int led_hex_color = 0x00FF00;
int led_red = 255;
int led_green = 255;
int led_blue = 255;
int led_num = 0; // iterator to pass around to know what LED we are on

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

//WiFi WAN connection info
char wifissid[] = "";
char wifipassword[] = "";

void setup() {
    Serial.begin(9600);
    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifissid, wifipassword);
    // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //    Serial.println("Connection Failed! Rebooting...");
    //    delay(5000);
    //    ESP.restart();
    //}

    server.on("/api/test", HTTP_GET, [&](){
        server.send(200, "text/plain", "OKAY");
    });
    
    server.on("/api/mode", HTTP_POST, [&](){
        if (server.args() != 1) return server.send(500, "text/plain", "Requires one argument.");
        display_mode = server.arg(0).toInt();
        server.send(200, "text/plain", "OK");
    });

    server.on("/api/solid_rgb", HTTP_POST, [&](){
        if (server.args() != 3) return server.send(500, "text/plain", "Requires three arguments.");
        led_red = server.arg(0).toInt();
        led_green = server.arg(1).toInt();
        led_blue = server.arg(2).toInt();
        display_mode = 1;
    });

    server.begin(); // Web server start

    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    
    ArduinoOTA.begin();
    
    delay(5000); // Delay so the serial debug has time to connect
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    strip.begin();
    strip.setBrightness(MAX_BRIGHTNESS);
    strip.show();
}

// Fill the dots one after the other with a color
void solid_rgb(uint32_t c) {
    for(uint16_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
}

// Fill the dots one after the other with a color
void color_wipe(uint32_t c, uint16_t wait) {
    if(currentMillis - previousMillis > wait){
        previousMillis = currentMillis;
        if (led_num > NUM_LEDS){
            led_num = 0;
        }
        strip.setPixelColor(led_num, c);
        strip.show();
        led_num++;
    }
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();

    currentMillis = millis();
    
    switch (display_mode) {
        case 1:
            solid_rgb(strip.Color(led_red, led_green, led_blue));
            break;
        case 2:
            color_wipe(strip.Color(led_red, led_green, led_blue), currentMillis);
            break;
}
