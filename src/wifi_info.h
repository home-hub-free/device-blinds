#ifndef WIFI_INFO_H_
#define WIFI_INFO_H_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#elif defined(ESP32)
#endif

HTTPClient http;
WiFiClient wifiClient;

const char *ssid = "";
const char *password = "";
String home_server = "";

const uint32 id = system_get_chip_id();

void wifi_connect() {
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin(ssid, password);
	Serial.println("WiFi connecting...");
	while (!WiFi.isConnected()) {
		delay(100);
		Serial.print(".");
	}
	Serial.print("\n");
	Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

void ping() {
  http.begin(wifiClient, home_server + "/device-declare");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST("{ \"id\": \"" + String(id) + "\", \"name\": \"blinds\" }");
  if (httpCode > 0) {
    String result = http.getString();
    Serial.print("Ping sent to server: ");
    Serial.println(result);
  }
  else {
    Serial.print("Oops: ");
    Serial.println(httpCode);
  }
  http.end();
}

#endif /* WIFI_INFO_H_ */