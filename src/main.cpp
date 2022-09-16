#include <Arduino.h>
#include <wifi_info.h>
#include <motor-handler.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void handleRoot() {}

void move() {
  String value = server.arg("value");
  if (value) {
    int position = value.toInt();
    move_to_position_async(position);
  }
  server.send(200, "text/plain", "true");
}

void setup()
{
	Serial.begin(9600);
	// Turn led on to indicate initialization process begins
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	delay(50);
	wifi_connect(); // in wifi_info.h

  server.on("/", handleRoot);
  server.on("/set", move);
  server.begin();

  declareDevice();

	// Initialize stepper motor pins and starting position/configuration
	init_motor();

	// Turn led off to indicate initialization process finish
	digitalWrite(LED_BUILTIN, HIGH);
}

unsigned long previousMillis = 0;
unsigned long interval = 30 * 1000;
void loop() {
  server.handleClient();

  unsigned long currentMills = millis();
  if (currentMills - previousMillis >= interval) {
    previousMillis = currentMills;
    declareDevice();
  }

  motor_handler_loop();
}