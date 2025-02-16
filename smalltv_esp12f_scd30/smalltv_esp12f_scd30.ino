#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include "TimesNRCyr10.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <OpenWeather.h>
#include <Wire.h>
#include <SensirionI2cScd30.h>

// https://github.com/immortalserg/AdafruitGFXRusFonts?ysclid=lt1kmcrl40546383100

#define TFT_BL 16//5 // LED back-light control pin
#define TFT_DC    0   
#define TFT_RST   2
#define TFT_CS    -1
#define DISABLE_BACKLIGHT 1
#define ENABLE_BACKLIGHT 0
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//define TFT_BL 16//5 // LED back-light control pin
//define TFT_BACKLIGHT_ON LOW // Level to turn ON back-light (HIGH or LOW)
//define TFT_CS -1 // Chip select control pin D8
//define TFT_DC 0 // Data Command control pin
//define TFT_RST 2 // Reset pin (could connect to NodeMCU RST, see next line)
//SCK 14
//MOSI 13

SensirionI2cScd30 sensor;
#define SDA0_Pin 4
#define SCL0_Pin 5
static int16_t error;
static char errorMessage[128];


// Replace these with your WiFi credentials
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// NTP Servers
static const char* ntpServerName = "pool.ntp.org";
const int timeZone = 3;  // Change this to your time zone offset in hours
// Define NTP related variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServerName, timeZone * 3600);

// OpenWeather.com
String api_key = "OPENWEATHER_API_KEY"; // Obtain this from your OpenWeather account
String latitude =  "55.785377"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "37.475994"; // 180.000 to -180.000 negative for West
String units = "metric";  // or "imperial"
String language = "ru";   // See notes tab
OW_Weather ow; // Weather forecast library instance

// Data API host
String serverName = "https://nonexistentdomain.ru/yandex_smarthome/sensors_data";

// Coins API host
const char *bitcoinApiUrl = "https://api.coinbase.com/v2/prices/BTC-RUB/sell";

// Sensirion SCD30 variables
float scd30_co2Concentration = 0.0;
float scd30_temperature = 0.0;
float scd30_humidity = 0.0;

// function prototypes
void displayDateTime();
void displayWeather();
void displayBitcoin();
void displaySensirionSCD30();
void sendDataToServer();

void setup() {
  Serial.begin(9600);

  // Initialize LCD
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, DISABLE_BACKLIGHT);
  tft.init(240, 240, SPI_MODE3); 
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
//  testdrawtext("12:34", ST77XX_WHITE);
  digitalWrite(TFT_BL, ENABLE_BACKLIGHT);

  tft.setCursor(10, 90);
//  tft.setFont(&FreeSans12pt7b);// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  tft.setFont(&TimesNRCyr10pt8b);// https://github.com/immortalserg/AdafruitGFXRusFonts
  tft.print("Подключение к Wi-Fi...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize and synchronize time
  configTime(0, 0, ntpServerName);
  Serial.println("Waiting for time synchronization...");
  while (!time(nullptr)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Time synchronized");

  // Initialize Sensirion SCD30
  Wire.begin(SDA0_Pin, SCL0_Pin);
  sensor.begin(Wire, SCD30_I2C_ADDR_61);
  sensor.stopPeriodicMeasurement();
  sensor.softReset();
  delay(2000);
  error = sensor.startPeriodicMeasurement(0);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }

}

void testdrawtext(String text, uint16_t color, int x, int y) {
//  tft.setTextSize(6);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.setFont(&FreeSans24pt7b);// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  tft.print(text);
}


void loop() {
  tft.fillScreen(ST77XX_BLACK);
  displayDateTime();
  displayWeather();
  displayBitcoin();
  displaySensirionSCD30();
  sendDataToServer();
  delay(60000);  // Update every minutes
}

void displayDateTime() {
  String days[] = {"воскресенье", "понедельник", "вторник", "среда", "четверг", "пятница", "суббота"};

  // Update time
  timeClient.update();

  // Get current time
  time_t now = timeClient.getEpochTime();
  struct tm *timeInfo;
  timeInfo = localtime(&now);
   
  uint16_t year = timeInfo->tm_year + 1900;
  String yearStr = String(year);

  uint8_t month = timeInfo->tm_mon + 1;
  String monthStr = month < 10 ? "0" + String(month) : String(month);

  uint8_t day = timeInfo->tm_mday;
  String dayStr = day < 10 ? "0" + String(day) : String(day);

  uint8_t hours = timeInfo->tm_hour;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  uint8_t minutes = timeInfo->tm_min;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  uint8_t seconds = timeInfo->tm_sec;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  int dow = timeInfo->tm_wday;       // Sat = 7, Sun = 0;

  String timeStr = hoursStr + ":" + minuteStr;
  String dateStr = dayStr + "." + monthStr + "." + yearStr + " (" + days[dow] + ")";

  tft.setCursor(50, 50);
  tft.setFont(&FreeSans24pt7b);// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  tft.print(timeStr);

  tft.setCursor(10, 90);
//  tft.setFont(&FreeSans12pt7b);// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  tft.setFont(&TimesNRCyr10pt8b);// https://github.com/immortalserg/AdafruitGFXRusFonts
  tft.print(dateStr);
}

void displayWeather() {
  OW_forecast *forecast = new OW_forecast;
  ow.getForecast(forecast, api_key, latitude, longitude, units, language);
  String weatherTemp = "На улице " + String(int(forecast->temp[0])) + "C\n  "
     + forecast->description[0];
//     + "\n  Давление " + String(int(forecast->pressure[0]*0.75006))
//     + " мм\n  Влажность " + forecast->humidity[0] + "%";

  tft.setCursor(10, 120);
  tft.setFont(&TimesNRCyr10pt8b);// https://github.com/immortalserg/AdafruitGFXRusFonts
  tft.print(weatherTemp);
}

void displayBitcoin() {
  JsonDocument json;
//  StaticJsonDocument<200> json;
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  //client->setFingerprint(fingerprint_sni_cloudflaressl_com);
  client->setInsecure();
  HTTPClient https;
  if (!https.begin(*client, bitcoinApiUrl))
    return;
  int httpsCode = https.GET();
  if (httpsCode != 200)
     return;
  String jsonStr = https.getString();
  https.end();
  DeserializationError err = deserializeJson(json, jsonStr);
  if (err)
    return;
  float amount = json["data"]["amount"];
  String CoinStr = "Биткоин: " + String(int(amount)) + " руб";
  tft.setCursor(10, 230);
//  tft.setFont(&TimesNRCyr10pt8b);// https://github.com/immortalserg/AdafruitGFXRusFonts
  tft.print(CoinStr);  
}

void displaySensirionSCD30(){
    error = sensor.blockingReadMeasurementData(scd30_co2Concentration, scd30_temperature,
                                               scd30_humidity);
scd30_temperature = 27.4;
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute blockingReadMeasurementData(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    String co2Info = "Уровень CO2: "+ String(int(scd30_co2Concentration)) + " ppm\n"
        + "  Температура: " + String(scd30_temperature, 1) + "C\n"
        + "  Влажность: " + String(int(scd30_humidity)) + "%";
    tft.setCursor(10, 165);
    tft.setFont(&TimesNRCyr10pt8b);// https://github.com/immortalserg/AdafruitGFXRusFonts
    tft.print(co2Info);
}

void sendDataToServer(){
    Serial.println("Send data to server...");

    String serverPath = serverName + "?scd30_temp=" + String(scd30_temperature, 1) + "&scd30_co2=" + String(int(scd30_co2Concentration))
        + "&scd30_h=" + String(int(scd30_humidity));

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint_sni_cloudflaressl_com);
    client->setInsecure();
    HTTPClient https;
    if (!https.begin(*client, serverPath.c_str()))
        return;
//    HTTPClient https;
//    https.begin(serverPath.c_str());
    int httpResponseCode = https.GET();
    Serial.println("Code: "+String(httpResponseCode));
}

