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
bool first_run = true;
bool run_toggle = true;
int run_num = 0;

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
        server.send(200, "text/plain", "OK");
        first_run = true;
        display_mode = server.arg(0).toInt();
    });

    server.on("/api/set_color", HTTP_POST, [&](){
        if (server.args() != 3) return server.send(500, "text/plain", "Requires one argument.");
        server.send(200, "text/plain", "OK");
        led_red = server.arg(0).toInt();
        led_green = server.arg(1).toInt();
        led_blue = server.arg(2).toInt();
    });

    server.on("/api/solid_rgd", HTTP_POST, [&](){
        if (server.args() != 3) return server.send(500, "text/plain", "Requires one argument.");
        server.send(200, "text/plain", "OK");
        led_red = server.arg(0).toInt();
        led_green = server.arg(1).toInt();
        led_blue = server.arg(2).toInt();
        first_run = true;
        display_mode = 1;
    });

    server.on("/api/color_wipe", HTTP_POST, [&](){
        if (server.args() != 3) return server.send(500, "text/plain", "Requires three arguments.");
        server.send(200, "text/plain", "OK");
        led_red = server.arg(0).toInt();
        led_green = server.arg(1).toInt();
        led_blue = server.arg(2).toInt();
        first_run = true;
        display_mode = 2;
    });

    server.on("/api/rainbow_cycle", HTTP_POST, [&](){
        server.send(200, "text/plain", "OK");
        first_run = true;
        display_mode = 3;

    });

    server.on("/api/fade_in_and_out", HTTP_POST, [&](){
        if (server.args() != 3) return server.send(500, "text/plain", "Requires three arguments.");
        server.send(200, "text/plain", "OK");
        led_red = server.arg(0).toInt();
        led_green = server.arg(1).toInt();
        led_blue = server.arg(2).toInt();
        first_run = true;
        display_mode = 4;
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
void color_wipe(uint32_t c, uint32_t wait) {
    if (first_run == true) {
        led_num = 0;
        first_run = false;
    }

    if (led_num > NUM_LEDS) {
        led_num = 0;
        run_toggle = !run_toggle;
    }
            
    if(currentMillis - previousMillis > wait) {
        previousMillis = currentMillis;
        strip.setPixelColor(led_num, c);
        strip.show();
        led_num++;
    }
}

void rainbowCycle(uint32_t wait) {
    uint16_t i;

    if (first_run == true) {
        led_num = 0;
        run_num = 0;
        first_run = false;
    }

    if (run_num > 256*5) {
        run_num = 0;
    }

    if (currentMillis - previousMillis > wait) {
        previousMillis = currentMillis;
        for(i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / NUM_LEDS) + run_num) & 255));
        }
        strip.show();
        run_num++;
    }
}

void fade_in_and_out(uint8_t red, uint8_t green, uint8_t blue, uint32_t wait) {
    if (first_run == true) {
        led_num = 0;
        run_num = 0;
        first_run = false;
    }

    if (currentMillis - previousMillis > wait) {
        previousMillis = currentMillis;
        if (run_toggle == true) {
            if (run_num < 255) {
                for(uint8_t i=0; i < NUM_LEDS; i++) {
                    strip.setPixelColor(i, red*run_num/255, green*run_num/255, blue*run_num/255);
                }
                strip.show();
                run_num++;
            }
            else if (run_num == 255) {
                run_toggle = false;
            }
        }
        if (run_toggle == false) {
            if (run_num > 0) {
                for(uint8_t i=0; i < NUM_LEDS; i++) {
                    strip.setPixelColor(i, red*run_num/255, green*run_num/255, blue*run_num/255);
                }
                strip.show();
                run_num--;
            }
            else if (run_num == 0) {
                run_toggle = true;
            }
        }
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
            if (run_toggle == false) {
                color_wipe(strip.Color(led_red, led_green, led_blue), 50);
            }
            else if (run_toggle == true) {
                color_wipe(strip.Color(0, 0, 0), 50);
            }            
            break;
        case 3:
            rainbowCycle(20);
        case 4:
            fade_in_and_out(led_red, led_green, led_blue, 70);
    }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}