TaskHandle_t Sensor_Read_handler;

void Sensor_Read(void *pvParameters) {
  while (1) {
    if (!send_2m && rtc.getMillis() < 500 && rtc.getSecond() % 5 == 0 && rtc.getSecond() != 0) {
      co2 = 450;
      suhu = 24.5;
      kelembapan = 50;

      Serial.println("CO2 = " + String(co2));
      Serial.println("Suhu = " + String(suhu));
      Serial.println("Kelembapan = " + String(kelembapan));

      delay(1000);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

void LED(void *pvParameters) {
  while (1) {
    ledcWrite(4, 200);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ledcWrite(4, 256);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ledcWrite(4, 200);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ledcWrite(4, 256);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    esp_task_wdt_reset();
  }
}


void Core_1_setup() {
  xTaskCreatePinnedToCore(
    Sensor_Read,
    "Sensor_Read",
    4096,
    NULL,
    5,
    &Sensor_Read_handler,
    1);

  esp_task_wdt_add(Sensor_Read_handler);
}