#include <Arduino.h>
#include <M5Core2.h>
#include "global.h"

TelnetSpy debug;
ESPNowReceiver espnow;
char string[16];
TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);
#define GAUGE_WIDTH 200
#define GAUGE_HEIGHT 35

void setup()
{
  Serial.begin(115200);
  debug.setStoreOffline(true);
  debug.begin(115200);
  debug.print("MAC:");
  debug.println(WiFi.macAddress());

  M5.begin(); // Init M5Core2.  初始化 M5Core2
  /* Power chip connected to gpio21, gpio22, I2C device
         Set battery charging voltage and current
         If used battery, please call this function in your project */
  M5.Lcd.textsize = 2;
  // M5.Lcd.print("Hello World"); // Print text on the screen
  M5.Lcd.setBrightness(255);
  sprite.createSprite(GAUGE_WIDTH, GAUGE_HEIGHT);

  uint8_t mac[6];
  WiFi.macAddress(mac);

  Serial.print("ESP32 WiFi MAC address: ");
  for (int i = 0; i < 6; i++)
  {
    Serial.print(mac[i], HEX);
    if (i < 5)
    {
      Serial.print(":");
    }
  }
  Serial.println();
  espnow.init();
}

void updateDisplay()
{
  M5Display tft = M5.Lcd;
  int xLocation = 0, yLocation = 0, yIncrement = 20;
  tft.setTextSize(2);

  sprintf(string, "Bus V:%0.1f (%i)  ", sensorData.batteryVoltage + 0.05, sensorData.frame);
  tft.setCursor(xLocation, yLocation);
  tft.print(string);
  // TFT_printLine(string, true);

  sprintf(string, "F Qty:%i %i   ", (int)sensorData.fuelLitres, sensorData.fuelQtyError);
  yLocation += yIncrement;
  tft.setCursor(xLocation, yLocation);
  tft.print(string);

  // TFT_printLine(string, false);

  sprintf(string, "F Prs:%i %i   ", (int)sensorData.fuelPress, sensorData.fuelPressError);
  yLocation += yIncrement;
  tft.setCursor(xLocation, yLocation);
  tft.print(string);
  // TFT_printLine(string, false);

  sprintf(string, "O Tmp:%i %i   ", (int)sensorData.oilTemp, sensorData.oilTempError);
  yLocation += yIncrement;
  tft.setCursor(xLocation, yLocation);
  tft.print(string);
  // TFT_printLine(string, false);

  sprintf(string, "O Prs:%0.1f %i   ", sensorData.oilPress, sensorData.oilPressError);
  yLocation += yIncrement;
  tft.setCursor(xLocation, yLocation);
  tft.print(string);
  // TFT_printLine(string, false);
}

void drawFatLine(int x, int y, int destx, int desty, int thickness, uint32_t color)
{
  for (size_t i = 0; i < thickness; i++)
  {
    M5.Lcd.drawLine(x + i, y, destx + i, desty, color);
  }
}

void drawFatLine(TFT_eSprite &sprite, int x, int y, int destx, int desty, int thickness, uint32_t color)
{
  for (size_t i = 1; i < thickness; i++)
  {
    sprite.drawLine(x, y + i, destx, desty + i, color);
  }
}

void drawFatLineSprite(TFT_eSprite &sprite, int x, int y, int destx, int desty, int thickness, uint32_t color)
{
  int midPoint = (int)(thickness / 2);
  for (size_t i = 1; i < thickness; i++)
  {
    sprite.drawLine(x, y - i + midPoint, destx, desty - i + midPoint, color);
  }
}

void drawGaugeSprite(M5Display &tft, int x, int y, int minValue, int maxValue, int value, bool isError, int labels[], int numLabels)
{
  int border = 3, pointerSize = 10;
  float pointer = (float)GAUGE_WIDTH / (float)maxValue;
  int top = GAUGE_HEIGHT / 2;
  int needle = (pointer * value);

  // draw outline
  sprite.fillSprite(BLACK);
  sprite.fillRect(0, 0, border, GAUGE_HEIGHT, WHITE);
  sprite.fillRect(GAUGE_WIDTH - border, 0, border, GAUGE_HEIGHT, WHITE);
  sprite.fillRect(0, GAUGE_HEIGHT - border, GAUGE_WIDTH, border, WHITE);

  // draw colour bar
  sprite.fillRect(border, top, GAUGE_WIDTH - border - border, (GAUGE_HEIGHT / 2) - border, GREEN);

  // draw labels
  sprite.textdatum = CC_DATUM;
  sprite.textsize = 1;
  for (int i = 0; i < numLabels; i++)
  {
    int pos = (int)(pointer * labels[i]);
    sprite.setCursor(pos, 5);
    sprite.drawFastVLine(pos, 0, GAUGE_HEIGHT, BLACK);
    // char cstr[10];
    // itoa(labels[i], cstr, 10);
    // debug.printf("%i:%s ", labels[i], cstr);
    sprite.print(labels[i]); //<-- crashes here with LoaderError
  }
  sprite.textdatum = TL_DATUM;

  // draw needle
  sprite.fillTriangle(needle - pointerSize, 0 - border, needle + pointerSize, 0 - border, needle, top, WHITE);

  // if data not valid then draw red cross over the gauge
  if (isError)
  {
    drawFatLineSprite(sprite, 0, 0, GAUGE_WIDTH, GAUGE_HEIGHT, 5, RED);
    drawFatLineSprite(sprite, 0, GAUGE_HEIGHT, GAUGE_WIDTH, 0, 5, RED);
  }

  // Copy the sprite to the screen
  sprite.pushSprite(x, y);
}

void drawGauges()
{
  int y = 20;
  int x = 120;
  int gauageIncrement = 55;

  int labels[] = {10, 30, 50, 70, 90, 100};
  drawGaugeSprite(M5.lcd, x, y, 0, 120, (int)sensorData.fuelLitres, sensorData.fuelQtyError, labels, 6);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.printf("Fuel Qty \n %i  ", (int)sensorData.fuelLitres);

  y += gauageIncrement;

  int labels2[] = {40, 60, 80, 110, 130};
  drawGaugeSprite(M5.lcd, x, y, 30, 140, (int)sensorData.fuelPress, sensorData.fuelPressError, labels2, 5);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.printf("Fuel P \n %i  ", (int)sensorData.fuelPress);

  y += gauageIncrement;
  // int labels2[] = {40, 60, 80, 110, 130};
  drawGaugeSprite(M5.lcd, x, y, 30, 140, (int)sensorData.oilTemp, sensorData.oilTempError, labels2, 5);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.printf("Oil Tmp \n %i  ", (int)sensorData.oilTemp);

  y += gauageIncrement;
  // int labels2[] = {40, 60, 80, 110, 130};
  drawGaugeSprite(M5.lcd, x, y, 30, 140, (int)sensorData.oilPress, sensorData.oilPressError, labels2, 5);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.printf("Oil P \n %i  ", (int)sensorData.oilPress);
}

void loop()
{
  static unsigned long lastUpdated;

  if (sensorDataUpdated)
  {
    if (millis() - lastUpdated > 5000)
      M5.lcd.clearDisplay();

    // updateDisplay();
    drawGauges();
    // espnow.debug();
    sensorDataUpdated = false;
    lastUpdated = millis();
  }

  if (millis() - lastUpdated > 5000)
  {
    // M5.Lcd.fillScreen(BLACK);

    drawFatLine(0, 0, 320, 240, 10, RED);
    drawFatLine(0, 240, 320, 0, 10, RED);
  }

  delay(100);
}