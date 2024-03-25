#if !defined(ESP8266)
#error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);


#include <DHT.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Constants
#define DHTPIN 0          // what pin we're connected to
#define DHTTYPE DHT22      // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);  //// Initialize DHT sensor for normal 16mhz Arduino


//Variables
float hum;   //Stores humidity value
float temp;  //Stores temperature value

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  //display.setFont(&FreeMono9pt7b);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(WHITE);

  display.clearDisplay();
  display.setCursor(0, 16);
  display.println("Init Sensor");
  display.display();

  dht.begin();
  wifi_connect(); // in wifi_info.h
	my_homekit_setup();
}

void loop() {
	my_homekit_loop();
  delay(10);
}


//==============================
// Homekit setup and loop
//==============================

// access your homekit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_current_temperature;
extern "C" homekit_characteristic_t cha_humidity;

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_setup() {
	arduino_homekit_setup(&config);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_report_millis) {
		// report sensor values every 10 seconds
		next_report_millis = t + 10 * 1000;
		my_homekit_report();
	}
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}

void my_homekit_report() {
  char s[40];
  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  display.clearDisplay();
  display.setCursor(0, 12);
  sprintf(s,"Tmp: %5.1fC", temp);
  display.print(s);

  display.setCursor(0, 30);
  sprintf(s,"Hmd: %5.1f%c", hum, '%');
  display.print(s);
  display.display();      // Show initial text

	cha_current_temperature.value.float_value = temp;
	LOG_D("Current temperature: %.1f", temp);
	homekit_characteristic_notify(&cha_current_temperature, cha_current_temperature.value);
	cha_humidity.value.float_value = hum;
	LOG_D("Current humidity: %.1f", hum);
	homekit_characteristic_notify(&cha_humidity, cha_humidity.value);
}
