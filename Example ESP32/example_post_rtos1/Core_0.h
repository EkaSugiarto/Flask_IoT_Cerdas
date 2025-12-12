#include "esp32-hal-gpio.h"
TaskHandle_t WiFi_Connect_handler, POSTTimer_handler, Data_Send_handler;

bool WiFi_status = false;
bool send_2m = false;

void POSTTimer(void *pvParameters) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED && (rtc.getYear() < 2025)) {
      Serial.println("Syncning RTC ...");
      Sync_RTC();
      esp_task_wdt_reset();
    }

    if (rtc.getSecond() == 15 && rtc.getMinute() == 0 && rtc.getHour(true) == 0 && !send_2m) {
      Serial.println("Restarting AQMS ...");
      ESP.restart();
      esp_task_wdt_reset();
    }

    if (WiFi.status() == WL_CONNECTED && rtc.getSecond() == 0 && rtc.getMinute() % 2 == 0 && rtc.getYear() >= 2025) {
      send_2m = true;

      client_wifi = new WiFiClientSecure;
      client_wifi->setInsecure();

      Send();

      delete client_wifi;

      send_2m = false;
      delay(1000);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

void WiFi_Connect(void *pvParameters) {
  while (1) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi_status = false;
      Serial.println("Connecting to WiFi");
      char char_ssid[sizeof(ssid)];
      char char_pass[sizeof(pass)];
      ssid.toCharArray(char_ssid, sizeof(ssid));
      pass.toCharArray(char_pass, sizeof(pass));
      WiFi.hostname(device_name);
      WiFi.mode(WIFI_STA);
      WiFi.begin(char_ssid, char_pass);
    } else {
      WiFi_status = true;
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}


void Core_0_setup() {
  xTaskCreatePinnedToCore(
    WiFi_Connect,
    "WiFi_Connect",
    4096,
    NULL,
    1,
    &WiFi_Connect_handler,
    0);

  xTaskCreatePinnedToCore(
    POSTTimer,
    "POSTTimer",
    8192,
    NULL,
    1,
    &POSTTimer_handler,
    0);

  esp_task_wdt_add(WiFi_Connect_handler);
  esp_task_wdt_add(POSTTimer_handler);
}