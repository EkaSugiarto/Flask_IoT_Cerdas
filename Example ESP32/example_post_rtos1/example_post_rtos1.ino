const String device_name = "IoT_Cerdas";

const String ssid = "Nahideung";
const String pass = "EKABL123";

const String api_post = "https://kelasiotcerdas.pythonanywhere.com/api/post/data";
const String api_time = "https://kelasiotcerdas.pythonanywhere.com/api/time";

#include <esp_task_wdt.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

WiFiClientSecure* client_wifi;

#include "Sensor_DHT.h"
#include "Sensor_CO2.h"

#include "Date_Time.h"
#include "Send.h"

#include "Core_0.h"
#include "Core_1.h"

void setup() {
  rtc.setTime(0, 0, 22, 23, 1, 2002);
  Serial.begin(115200);

  esp_task_wdt_init(60, true);

  Core_0_setup();
  Core_1_setup();

  esp_task_wdt_delete(NULL);
  vTaskDelete(NULL);
}

void loop() {
}
