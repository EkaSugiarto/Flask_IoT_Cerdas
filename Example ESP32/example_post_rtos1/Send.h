String jsonString;

void addJson(String key, String value) {
  if (jsonString.length() > 1) jsonString += ",";
  jsonString += "\"" + key + "\":\"" + String(value) + "\"";
}

String getJson() {
  return "{" + jsonString + "}";
}

int httpCode;

void Send() {
  addJson("created_at", get_precission_second(0));
  addJson("device", device_name);
  addJson("co2", String(co2));
  addJson("suhu", String(suhu));
  addJson("kelembapan", String(kelembapan));

  Serial.println("Starting POST ...");

  HTTPClient https;

  if (https.begin(*client_wifi, api_post)) {
    https.setUserAgent(String(device_name));
    https.addHeader("Content-Type", "application/json");

    int attempt = 0;
    bool success = false;

    while (attempt < 5 && !success) {
      Serial.println("Attempt " + String(attempt + 1));
      Serial.println(getJson());
      Serial.println("API link = " + api_post);

      httpCode = https.POST(getJson());

      if (httpCode == 200 || httpCode == 201) {
        Serial.println("POST sent! Code: " + String(httpCode) + "\n");
        success = true;
      } else {
        Serial.println("POST failed! Code: " + String(httpCode) + "\n");
        attempt++;
        if (attempt < 5) delay(1000);
      }
    }
    https.end();
  }

  jsonString = "";
}