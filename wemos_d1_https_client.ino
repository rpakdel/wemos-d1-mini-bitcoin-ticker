
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "TickerValues.h"
#include "myssid.h"

#define OLED_I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
const char* host = "api.coinmarketcap.com";
const int httpsPort = 443;

long prevGet = 0;
TickerValues values[NUM_TICKERS];
bool lastGetResult = false;
#define GET_TICKER_VALUES_DELAY 60000 // 1 minute



void initDisplay()
{
    Wire.begin();
    oled.begin(&Adafruit128x32, OLED_I2C_ADDRESS);
    oled.set400kHz();
    oled.setFont(Adafruit5x7);
    oled.clear();
}




bool fillTickerValues(TickerValues values[NUM_TICKERS])
{
    Serial.println("Getting ticker values");
    WiFiClientSecure client;
    if (!client.connect(host, httpsPort))
    {
        Serial.println(F("connection failed"));
        return false;
    }

    String url = String("/v1/ticker/?convert=CAD&limit=") + String(NUM_TICKERS);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");

    // read headers
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }
    // read body
    String body = client.readString();
    DynamicJsonBuffer buffer;
    JsonArray& arr = buffer.parseArray(body);
    byte index = 0;    
    for (JsonArray::iterator it = arr.begin(); it != arr.end(); ++it)
    {
        JsonObject& o = it->asObject();
        values[index].symbol = o["symbol"].asString();
        values[index].price_cad = o["price_cad"].as<double>();
        values[index].percent_change_24h = o["percent_change_24h"].as<double>();
        values[index].percent_change_7d = o["percent_change_7d"].as<double>();
        index++;
    }

    return true;
}



void setup() 
{
    Serial.begin(115200);
    initDisplay();
    Serial.println();
    Serial.print(F("SSID "));
    Serial.println(MYSSID);
    oled.print(F("SSID "));
    oled.println(MYSSID);
    Serial.println();
    WiFi.mode(WIFI_STA);
    WiFi.begin(MYSSID, MYPASSWORD);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println();
    Serial.print(F("WiFi connected, IP: "));
    Serial.println(WiFi.localIP());
    oled.print(WiFi.localIP());
    prevGet = millis();
    lastGetResult = fillTickerValues(values);

}

void displayTickerValues(TickerValues values[NUM_TICKERS])
{
    oled.clear();
    for (byte index = 0; index < NUM_TICKERS; ++index)
    {
        oled.clear();
        TickerValues v = values[index];
        oled.print(v.symbol);
        oled.print(" $");
        oled.print(v.price_cad);
        oled.println(" CAD");

        oled.print("24h ");
        oled.print(v.percent_change_24h);
        oled.println("%");
        oled.print("7d ");
        oled.print(v.percent_change_7d);
        oled.println("%");
        delay(3000);
    }
}


void loop() 
{
    long m = millis();
    long diff = m - prevGet;
    if (diff >= GET_TICKER_VALUES_DELAY)
    {
        prevGet = m;
        lastGetResult = fillTickerValues(values);
    }

    if (lastGetResult) 
    {
        displayTickerValues(values);
    }
    else
    {
        oled.clear();
    }
}