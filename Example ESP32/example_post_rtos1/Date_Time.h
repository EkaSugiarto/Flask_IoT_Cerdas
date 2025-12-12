#include "sys/_types.h"
#include "esp32-hal.h"

#include <ESP32Time.h>

ESP32Time rtc(0);

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