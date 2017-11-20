#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <OpenWeatherMap.h>

const char *ow_key      = "Your Open Weather Map key here";
const char *nodename    = "esp8266-weather";
const char *wifi_ssid   = "ssid";
const char *wifi_passwd = "password";


//==============================================================================================================

typedef enum wifi_s {
  W_AP = 0, W_READY, W_TRY
} WifiStat;

OWMconditions      owCC;
OWMfiveForecast    owF5;
OWMsixteenForecast owF16;
WifiStat           WF_status;

void connectWiFiInit(void) {
  WiFi.hostname(nodename);
  String ssid   = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());
}

String dateTime(String timestamp) {
  time_t ts = timestamp.toInt();
  char buff[30];
  sprintf(buff, "%2d:%02d %02d-%02d-%4d", hour(ts), minute(ts), day(ts), month(ts), year(ts));
  return String(buff);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n\n\n");

  connectWiFiInit();
  WF_status  = W_TRY;
}

void currentConditions(void) {
  OWM_conditions *ow_cond = new OWM_conditions;
  owCC.updateConditions(ow_cond, ow_key, "ru", "Moscow", "metric");
  Serial.print("Latitude & Longtitude: ");
  Serial.print("<" + ow_cond->longtitude + " " + ow_cond->latitude + "> @" + dateTime(ow_cond->dt) + ": ");
  Serial.println("icon: " + ow_cond->icon + ", " + " temp.: " + ow_cond->temp + ", press.: " + ow_cond->pressure);
  delete ow_cond;
}

void fiveDayFcast(void) {
  OWM_fiveForecast *ow_fcast5 = new OWM_fiveForecast[40];
  byte entries = owF5.updateForecast(ow_fcast5, 40, ow_key, "ru", "Moscow", "metric");
  Serial.print("Entries: "); Serial.println(entries+1);
  for (byte i = 0; i <= entries; ++i) { 
    Serial.print(dateTime(ow_fcast5[i].dt) + ": icon: ");
    Serial.print(ow_fcast5[i].icon + ", temp.: [" + ow_fcast5[i].t_min + ", " + ow_fcast5[i].t_max + "], press.: " + ow_fcast5[i].pressure);
    Serial.println(", descr.: " + ow_fcast5[i].description + ":: " + ow_fcast5[i].cond + " " + ow_fcast5[i].cond_value);
  }
  delete[] ow_fcast5;
}

void sixteedDayFcast(void) {
  OWM_sixteenLocation *location   = new OWM_sixteenLocation;
  OWM_sixteenForecast *ow_fcast16 = new OWM_sixteenForecast[7];
  byte entries = owF16.updateForecast(location, ow_fcast16, 7, ow_key, "ru", "Moscow", "metric");
  Serial.print("Entries: "); Serial.println(entries+1);
  Serial.print(location->city_name + ", " + location->country +"(" + location->city_id);
  Serial.println(") <" + location->longtitude + ", " + location->latitude + "> :");
  for (byte i = 0; i <= entries; ++i) { 
    Serial.print(dateTime(ow_fcast16[i].dt) + ": icon: ");
    Serial.print(ow_fcast16[i].icon + ", temp.: ");
    Serial.print("{" + ow_fcast16[i].t_min + ", " + ow_fcast16[i].t_max + "}, temp. change: ");
    Serial.print(ow_fcast16[i].t_night + " -> " + ow_fcast16[i].t_morning + " -> " + ow_fcast16[i].t_day + " -> " + ow_fcast16[i].t_evening);
    Serial.println("; pressure: " + ow_fcast16[i].pressure + ", descr.:" + ow_fcast16[i].description);
  }
  delete location;
  delete[] ow_fcast16;
}

void loop() {
  if (WF_status == W_TRY) {
    if (WiFi.status() == WL_CONNECTED) {
      MDNS.begin(nodename);
      WF_status = W_READY;
      Serial.println("Current Conditions: ");
      currentConditions();
      Serial.println("Five days forecast: ");
      fiveDayFcast();
      Serial.println("16 days forecast: ");
      sixteedDayFcast();
    }
  }
  delay(1000);
}

