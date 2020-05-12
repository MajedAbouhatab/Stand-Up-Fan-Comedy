#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

HTTPClient http;
char ssid[] = "YourSSID";
char pass[] = "YourPassword";
String Joke;
const long TimeBetweenLaughs = 12 * 60 * 60 * 1000;
const long AlarmTimeOut = 10 * 1000;
long TimeStamp = -1 * (TimeBetweenLaughs + AlarmTimeOut);
const int AlertPin = 12;
const int OnDemandPin = 26;
const int FanPin = 32;

void PrintAndWait(const uint8_t* Font, String Message, unsigned long HoldTime, int LineOffset) {
  u8g2.setFont(Font);
  u8g2.firstPage();
  do {
    for (int i = 0; i < 5; i++) u8g2.drawStr(0, LineOffset + (((i + 1) * 10) + (i * 2)), Message.substring(20 * i, (20 * i) + 20).c_str());
  } while ( u8g2.nextPage() );
  while (millis() < HoldTime) yield();
}

void setup()
{
  u8g2.begin();
  pinMode(AlertPin, OUTPUT);
  pinMode(OnDemandPin, INPUT_PULLUP);
  pinMode(FanPin, OUTPUT);
  digitalWrite(FanPin, 1);
}

void loop() {
  // Alarm has been going for too long
  if (millis() > TimeStamp + TimeBetweenLaughs + AlarmTimeOut) {
    // Turn off the alarm
    digitalWrite(AlertPin, 0);
    // Note the time
    TimeStamp = millis();
    // Time for another session , no alarm when we first enter main loop
  } else if (TimeStamp > 0 && millis() > TimeStamp + TimeBetweenLaughs) {
    PrintAndWait(u8g2_font_lubI12_tf , "Controlled                                     Breathing", millis(), 20);
    // Turn on the alarm
    digitalWrite(AlertPin, 1);
  }
  else {
    // Ice breaker
    PrintAndWait(u8g2_font_lubI12_tf , "Hello,                                      I'm Fanny", millis() + (1 * 1000), 20);
    // Not a wind breaker
    PrintAndWait(u8g2_font_lubI12_tf , "Pull                                          my                                           finger", millis() + (1 * 1000), 3);
  }
  // Yes, I'm ready for my session
  if (!digitalRead(OnDemandPin)) {
    // Note the time
    TimeStamp = millis();
    // This is gonna be good
    PrintAndWait(u8g2_font_lubI12_tf , "Let me tell you                               a joke", millis(), 20);
    // Turn off the alarm
    digitalWrite(AlertPin, 0);
    // Turn on the fan
    digitalWrite(FanPin, 0);
    do {
      // Start Wi-Fi connection
      if (WiFi.status() != WL_CONNECTED) WiFi.begin(ssid, pass);
      // Wait until connected
      while (WiFi.status() != WL_CONNECTED) yield;
      // Make API call
      http.begin("https://sv443.net/jokeapi/v2/joke/Any?blacklistFlags=nsfw,religious,political,racist,sexist&type=single");
      // Wait until OK
      while (http.GET() != 200) yield;
      // Get JSON
      StaticJsonDocument<200> doc;
      deserializeJson(doc, http.getString().c_str());
      http.end();
      // Get Joke out of JSON
      Joke = (doc["joke"]) ? String((const char*) doc["joke"]) : "";
      // No new lines
      Joke.replace("\n", " ");
      // Text is not empty but not more than what can be displayed on the screen
    } while (Joke.length() == 0 || Joke.length() > 100);
    // Tell the joke
    PrintAndWait(u8g2_font_t0_11_mr, Joke, millis() + (5 * 1000), 0);
    // Controlled Breathing
    for (int i = 0; i < 90; i++) {
      PrintAndWait(u8g2_font_lubI24_tf, " Inhale", millis() + (5 * 1000), 30);
      PrintAndWait(u8g2_font_lubI24_tf, " Exhale", millis() + (5 * 1000), 30);
    }
    // So long
    PrintAndWait(u8g2_font_lubI12_tf , "Have a                                       Great Day!", millis() + (5 * 1000), 20);
    // Turn off the fan
    digitalWrite(FanPin, 1);
  }
  // Restart every 30 days to avoid millis() overflow
  if (millis() > (30 * 24 * 60 * 60 * 1000)) ESP.restart();
}
