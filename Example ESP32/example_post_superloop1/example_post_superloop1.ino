#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <HTTPClient.h>

#include <ESP32Time.h>

#include <DHT.h>

const String ssid = "Nahideung";
const String pass = "EKABL123";

const String device_name = "IoT_Cerdas";

const String api_post = "https://kelasiotcerdas.pythonanywhere.com/api/post/data";
const String api_time = "https://kelasiotcerdas.pythonanywhere.com/api/time";

int co2, kelembapan;
float suhu;

bool send_2m;

WiFiClientSecure* client_wifi;

ESP32Time rtc(0);

DHT dht(32, DHT11);


// YYYY-MM-DD hh:mm:ss
String get_date_time(int choose) {
  String string_month, string_day, shour, sminute;

  rtc.getMonth() + 1 < 10 ? string_month = "0" + String(rtc.getMonth() + 1) : string_month = String(rtc.getMonth() + 1);
  rtc.getDay() < 10 ? string_day = "0" + String(rtc.getDay()) : string_day = String(rtc.getDay());

  switch (choose) {
    case 0:
      return String(rtc.getYear()) + "-" + string_month + "-" + string_day + " " + String(rtc.getTime());
      break;
    case 1:
      return String(rtc.getYear()) + "-" + string_month + "-" + string_day;
      break;
    case 2:
      return String(rtc.getTime());
      break;
  }
}


String get_precission_second(int seconds) {
  String shour, sminute, ssecond;

  rtc.getHour(true) < 10 ? shour = "0" + String(rtc.getHour(true)) : shour = String(rtc.getHour(true));
  rtc.getMinute() < 10 ? sminute = "0" + String(rtc.getMinute()) : sminute = String(rtc.getMinute());
  seconds < 10 ? ssecond = "0" + String(seconds) : ssecond = String(seconds);

  return get_date_time(1) + " " + shour + ":" + sminute + ":" + ssecond;
}


void Sync_RTC() {
  Serial.println("RTC initial time: " + get_date_time(0));

  WiFiClientSecure* date_time_client = new WiFiClientSecure;
  date_time_client->setInsecure();

  HTTPClient https;
  int get_httpcode;


  if (https.begin(*date_time_client, api_time)) {
    https.setUserAgent(String(device_name));

    get_httpcode = https.GET();


    if (get_httpcode >= 200 && get_httpcode <= 299) {
      String payload = https.getString();
      Serial.println(payload);

      int datetimeIndex = payload.indexOf("\"datetime\":");
      if (datetimeIndex != -1) {
        datetimeIndex = payload.indexOf("\"", datetimeIndex + 10) + 1;
        int datetimeEndIndex = payload.indexOf("\"", datetimeIndex);

        String datetime = payload.substring(datetimeIndex, datetimeEndIndex);

        if (datetime.length() >= 19) {
          int year = datetime.substring(0, 4).toInt();
          int month = datetime.substring(5, 7).toInt();
          int day = datetime.substring(8, 10).toInt();
          int hour = datetime.substring(11, 13).toInt();
          int minute = datetime.substring(14, 16).toInt();
          int second = datetime.substring(17, 19).toInt();

          rtc.setTime(second - 2, minute, hour, day, month, year);
        } else {
          Serial.println("Error: Datetime string format is invalid!");
        }
      } else {
        Serial.println("Error: \"datetime\" key not found in JSON response.");
      }
      Serial.println("RTC synchronized: " + get_date_time(0));
    } else {
      Serial.println("Failed to get the date time!");
      WiFi.disconnect();
    }
    https.end();
  }

  delete date_time_client;
}


String get_uptime() {
  unsigned long uptimeMillis = millis();
  unsigned long totalSeconds = uptimeMillis / 1000;

  unsigned int hours = totalSeconds / 3600;
  unsigned int minutes = (totalSeconds % 3600) / 60;
  unsigned int seconds = totalSeconds % 60;

  char buffer[9];
  sprintf(buffer, "%02u:%02u:%02u", hours, minutes, seconds);

  return String(buffer);
}


// Pengiriman data
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


void setup() {
  Serial.begin(115200);
  rtc.setTime(0, 0, 22, 23, 1, 2002);

  dht.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    long int start = millis();
    Serial.println("Connecting to WiFi");

    char char_ssid[sizeof(ssid)];
    char char_pass[sizeof(pass)];

    ssid.toCharArray(char_ssid, sizeof(ssid));
    pass.toCharArray(char_pass, sizeof(pass));

    WiFi.hostname(device_name);

    WiFi.mode(WIFI_STA);
    WiFi.begin(char_ssid, char_pass);

    delay(5000);

    Serial.println("Waktu eksekusi koneksi wifi (ms) = " +  String(millis() - start));
  }

  if (WiFi.status() == WL_CONNECTED && (rtc.getYear() < 2025)) {
    long int start = millis();
    Serial.println("Syncning RTC ...");
    Sync_RTC();
    delay(500);
    Serial.println("Waktu eksekusi sinkronisasi RTC (ms) = " +  String(millis() - start));
  }

  if (WiFi.status() == WL_CONNECTED && rtc.getMillis() < 500 && rtc.getSecond() == 0 && rtc.getMinute() % 2 == 0 && rtc.getYear() >= 2025) {
    long int start = millis();
    send_2m = true;

    client_wifi = new WiFiClientSecure;
    client_wifi->setInsecure();

    Send();

    delete client_wifi;

    send_2m = false;
    delay(500);
    Serial.println("Waktu eksekusi post data (ms) = " +  String(millis() - start));
  }

  if (!send_2m && rtc.getMillis() < 500 && rtc.getSecond() % 5 == 0 && rtc.getSecond() != 0) {
    long int start = millis();
    co2 = 450;
    suhu = dht.readTemperature();
    kelembapan = dht.readHumidity();

    Serial.println(get_date_time(0));
    Serial.println("CO2 = " + String(co2));
    Serial.println("Suhu = " + String(suhu));
    Serial.println("Kelembapan = " + String(kelembapan) + "\n");

    delay(500);
    Serial.println("Waktu eksekusi pembacaan sensor (ms) = " +  String(millis() - start));
  }
}