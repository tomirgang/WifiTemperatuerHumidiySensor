#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "DHT20.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
DHT20 DHT(&Wire);

#define DATA_LEN 100

double humidity[DATA_LEN];
double temperature[DATA_LEN];

int ptr = 0;
int ctr = -1;

uint32_t delay_time = 30000;
int save_divisor = 30;

int max_temp = 30;
int min_temp = 10;

int max_hum = 90;
int min_hum = 30;

double hum = 0.0;
double temp = 0.0;

int get_values(void) {
  int ok = 1;

  //  READ DATA
  Serial.print("DHT20, \t");
  
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_RED);  
  tft.setTextSize(2);

  int status = DHT.read();
  switch (status)
  {
  case DHT20_OK:
    ok = 0;
    Serial.print("OK,\t");
    break;
  case DHT20_ERROR_CHECKSUM:
    tft.print("Checksum error");
    Serial.print("Checksum error,\t");
    break;
  case DHT20_ERROR_CONNECT:
    tft.print("Connect error");
    Serial.print("Connect error,\t");
    break;
  case DHT20_MISSING_BYTES:
    tft.print("Missing bytes");
    Serial.print("Missing bytes,\t");
    break;
  case DHT20_ERROR_BYTES_ALL_ZERO:
    tft.print("All bytes read zero");
    Serial.print("All bytes read zero");
    break;
  case DHT20_ERROR_READ_TIMEOUT:
    tft.print("Read time out");
    Serial.print("Read time out");
    break;
  case DHT20_ERROR_LASTREAD:
    tft.print("Read too fast");
    Serial.print("Error read too fast");
    break;
  default:
    tft.print("Unknown error");
    Serial.print("Unknown error,\t");
    break;
  }

  hum = DHT.getHumidity();
  temp = DHT.getTemperature();

  Serial.print(hum, 1);
  Serial.print(",\t");
  Serial.println(temp, 1);

  if(ok == 0) {
    ctr++;
    if(ctr % save_divisor == 0) {
      ctr = 0;
      
      temperature[ptr] = temp;
      humidity[ptr] = hum;

      ptr++;
      ptr = ptr % 100;
    }
  }

  return ok;
}

void draw_values(void) {
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_RED,TFT_BLACK);  
  tft.setTextSize(2);
  tft.printf("%0.1lf", temp);

  tft.setCursor(0, 35, 2);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);  
  tft.setTextSize(2);
  tft.printf("%0.1lf", hum);
} 

void draw_history(void) {
  double t_f = 135.0 / (max_temp - min_temp);
  double h_f = 135.0 / (max_hum - min_hum);

  int x = 109;
  for(int i=0; i<DATA_LEN; i++) {
    int z = (i + ptr + 1) % DATA_LEN;
    x++;

    if(temperature[z] == -1) {
      continue;
    }

    int t_y = 135 - (int)((temperature[z] - ((double)min_temp)) * t_f);
    int h_y = 135 - (int)((humidity[z] - ((double)min_hum)) * h_f);

    if(t_y >= h_y) {
      tft.drawLine(x, h_y, x, 135, TFT_BLUE);
      tft.drawLine(x, t_y, x, 135, TFT_RED);
    } else {
      tft.drawLine(x, t_y, x, 135, TFT_RED);
      tft.drawLine(x, h_y, x, 135, TFT_BLUE);
    }
  }
}

void draw_grid(void) {
  tft.setTextColor(TFT_RED, TFT_BLACK);  
  tft.setTextSize(1);
  tft.setCursor(85, 0, 2);
  tft.printf("%d", max_temp);

  tft.setCursor(85, 60, 2);
  tft.printf("%d", (max_temp + min_temp)/2);

  tft.setCursor(85, 120, 2);
  tft.printf("%d", min_temp);

  tft.setTextColor(TFT_BLUE, TFT_BLACK);  
  tft.setTextSize(1);
  tft.setCursor(220, 0, 2);
  tft.printf("%d", max_hum);

  tft.setCursor(220, 60, 2);
  tft.printf("%d", (max_hum + min_hum)/2);

  tft.setCursor(220, 120, 2);
  tft.printf("%d", min_hum);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.setTextSize(1);
  tft.setCursor(115, 0, 2);
  tft.printf("24h");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.setTextSize(1);
  tft.setCursor(145, 0, 2);
  tft.printf("12h");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.setTextSize(1);
  tft.setCursor(185, 0, 2);
  tft.printf("now");

  tft.drawLine(211, 0, 211, 135, TFT_LIGHTGREY);
  tft.drawLine(109, 0, 109, 135, TFT_LIGHTGREY);
  tft.drawLine(110, 67, 211, 67, TFT_LIGHTGREY);
}



void setup(void) {
  for(int i=0; i<DATA_LEN; i++) {
    humidity[i]  = -1;
    temperature[i]  = -1;
  }

  tft.init();
  tft.setRotation(1);

  DHT.begin(21, 22);  //  select your pin numbers here

  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("DHT20 LIBRARY VERSION: ");
  Serial.println(DHT20_LIB_VERSION);
  Serial.println();
  delay(2000);

  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");
}

void loop() {
  int ok = get_values();
  if(ok == 0) {
    tft.fillScreen(TFT_BLACK);
    draw_values();
    draw_history();
    draw_grid();
  }

  delay(delay_time);
}
