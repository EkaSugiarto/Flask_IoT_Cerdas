#include <DHT.h>

DHT dht(32, DHT11);

int kelembapan;
float suhu;

void DHT_setup(){
  dht.begin();
}

void DHT_loop(){
  suhu = dht.readTemperature();
  kelembapan = dht.readHumidity();
}