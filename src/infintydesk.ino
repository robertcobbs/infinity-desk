#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#define DATA_PIN 4
#define NUM_LEDS 190

ESP8266WebServer server(80);

CRGB leds[NUM_LEDS];

int display_mode = 1;
int led_hex_color = 0xFF0000;

void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Serial.begin(115200);
    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //    Serial.println("Connection Failed! Rebooting...");
    //    delay(5000);
    //    ESP.restart();
    //}

    server.on("/api/display/mode", HTTP_POST, [&](){
    if (server.args() != 1) return server.send(500, "text/plain", "Requires one argument.");
    display_mode = server.arg(0).toInt();
    server.send(200, "text/plain", "OK");
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
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void solid_color() {
    FastLED.clear();
    for ( int led=0; led<NUM_LEDS; led++ ) {
		leds[led] = led_hex_color;
	}
    FastLED.show();
}

void run_leds() {
    switch (display_mode) {
        case 1:
            solid_color();
            break;
  }
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();

    run_leds();
}