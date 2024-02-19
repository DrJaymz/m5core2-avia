#include <Arduino.h>

#include "global.h"
#include "Free_Fonts.h"

TelnetSpy debug;
ESPNowReceiver espnow;
char string[16];
TFT_eSprite gSprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite pSprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite warnSprite = TFT_eSprite(&M5.Lcd);
char timeStr[20];

#define GAUGE_WIDTH 200
#define GAUGE_HEIGHT 34
#define PANEL_WIDTH 320 - GAUGE_WIDTH
#define PANEL_HEIGHT GAUGE_HEIGHT

void soundTask(void *parameter)
{
  for (;;)
  {
    if (alarmSound > 0)
    {
      soundsBeep(2200, 200, 100);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup()
{
  Serial.begin(115200);
  debug.setStoreOffline(true);
  debug.begin(115200);
  debug.print("MAC:");
  debug.println(WiFi.macAddress());

  M5.begin(); // Init M5Core2.  初始化 M5Core2
  M5.Lcd.setBrightness(255);

  gSprite.createSprite(GAUGE_WIDTH, GAUGE_HEIGHT);
  pSprite.createSprite(PANEL_WIDTH, PANEL_HEIGHT);
  warnSprite.createSprite(PANEL_WIDTH, 30);

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

  //  M5.Spk.DingDong();
  M5.Axp.SetLcdVoltage(2900);

  beginSD();
  soundsBeep(2700, 500, 100);

  M5.IMU.Init(); // Init IMU sensor.

  xTaskCreatePinnedToCore(soundTask, "soundTask", 4096, NULL, 1, NULL, 0);
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

void drawGaugeSprite(M5Display &tft, int x, int y, int minValue, int maxValue, float value, bool isError, int labels[], int numLabels, ColoredRange ranges[], int numRanges)
{
  int border = 3, pointerSize = 10;
  float pointer = (float)GAUGE_WIDTH / ((float)maxValue - (float)minValue);
  int colourTop = GAUGE_HEIGHT * 0.65;
  int colourHeight = GAUGE_HEIGHT * 0.25;
  int needle = (pointer * (value - minValue));

  gSprite.fillSprite(BLACK);

  // draw colour bar
  gSprite.fillRect(border, colourTop, GAUGE_WIDTH - border - border, colourHeight, GREEN);

  // draw labels
  gSprite.textdatum = TC_DATUM;
  // draw color bars
  for (int i = 0; i < numRanges; i++)
  {
    int start = (int)(pointer * (ranges[i].start - minValue));
    int end = (int)(pointer * (ranges[i].end - minValue));
    int width = end - start;
    gSprite.fillRect(start, colourTop, width, colourHeight, ranges[i].color);
  }

  for (int i = 0; i < numLabels; i++)
  {
    gSprite.setFreeFont(&FreeMonoBold9pt7b);
    int pos = (int)(pointer * (labels[i] - minValue));
    gSprite.setCursor(pos, 0);
    gSprite.fillRect(pos, colourTop, 3, colourHeight, BLACK);
    gSprite.drawCentreString((String)labels[i], pos, 5, GFXFF);
    gSprite.textcolor = LIGHTGREY;
  }
  gSprite.textdatum = TL_DATUM;

  // draw needle
  gSprite.fillTriangle(needle - pointerSize, 0 - border, needle + pointerSize, 0 - border, needle, colourTop, WHITE);

  // if data not valid then draw red cross over the gauge
  if (isError)
  {
    drawFatLineSprite(gSprite, 0, 0, GAUGE_WIDTH, GAUGE_HEIGHT, 5, RED);
    drawFatLineSprite(gSprite, 0, GAUGE_HEIGHT, GAUGE_WIDTH, 0, 5, RED);
  }

  // Copy the sprite to the screen
  gSprite.pushSprite(x, y);
}

void drawPanelSprite(M5Display &tft, int x, int y, String name, String units, float value, bool isFloat = false, int warning = 0)
{

  switch (warning)
  {
  case 1:
    pSprite.fillSprite(BLACK);
    pSprite.textcolor = (YELLOW);
    pSprite.textbgcolor = (BLACK);
    break;
  case 2:
    pSprite.fillSprite(RED);
    pSprite.textcolor = (WHITE);
    pSprite.textbgcolor = (RED);
    break;

  default:
    pSprite.fillSprite(BLACK);
    pSprite.textcolor = (WHITE);
    pSprite.textbgcolor = (BLACK);
    break;
  }

  pSprite.setFreeFont(&FreeSansBold9pt7b);
  pSprite.textsize = 1;
  pSprite.drawString(name, 0, 0);

  pSprite.setFreeFont(&FreeSansBold9pt7b);
  pSprite.drawString(units, 0, 18);

  pSprite.setFreeFont(&FreeSans18pt7b);

  if (isFloat)
  {
    sprintf(string, "%0.1f", value);
  }
  else
  {
    sprintf(string, "%i", (int)value);
  }

  pSprite.textdatum = TC_DATUM;
  pSprite.drawString(string, 85, 5, GFXFF);

  pSprite.pushSprite(x, y);
}

void drawGauges()
{
  char string[15];

  int y = 30;
  int x = 320 - GAUGE_WIDTH;
  int gauageIncrement = 48;

  int *labels;

  labels = new int[6]{10, 30, 50, 70, 90, 110};

  drawGaugeSprite(M5.lcd, x, y, 0, 120, (int)sensorData.fuelLitres, sensorData.fuelQtyError, labels, 6, fuelQTYRange, fuelQTYRangeNum);
  drawPanelSprite(M5.Lcd, 0, y, "FUEL", "  Ltr", (int)sensorData.fuelLitres, false, fuelQTYWarning);
  delete[] labels;

  y += gauageIncrement;

  labels = new int[5]{0, 50, 150, 250, 350};
  drawGaugeSprite(M5.lcd, x, y, 0, 350, (int)sensorData.fuelPress, sensorData.fuelPressError, labels, 5, fuelPressRange, fuelPressRangeNum);
  drawPanelSprite(M5.Lcd, 0, y, "FUEL", "  mb", (int)sensorData.fuelPress, false, fuelPressWarning);
  delete[] labels;

  y += gauageIncrement;

  labels = new int[5]{40, 60, 80, 110, 130};
  drawGaugeSprite(M5.lcd, x, y, 30, 140, (int)sensorData.oilTemp, sensorData.oilTempError, labels, 5, oilTempRange, oilTempRangeNum);
  drawPanelSprite(M5.Lcd, 0, y, "O I L", "   C", (int)sensorData.oilTemp, false, oilTempWarning);
  delete[] labels;

  y += gauageIncrement;

  labels = new int[4]{0, 2, 4, 6};
  drawGaugeSprite(M5.lcd, x, y, 0, 7, sensorData.oilPress, sensorData.oilPressError, labels, 4, oilPressRange, oilPressRangeNum);
  drawPanelSprite(M5.Lcd, 0, y, "O I L", "  bar", sensorData.oilPress, true, oilPressWarning);
  delete[] labels;
}

uint32_t getColorForValue(const ColoredRange *ranges, size_t numRanges, float value)
{
  for (size_t i = 0; i < numRanges; i++)
  {
    const ColoredRange &range = ranges[i];
    if (value >= range.start && value <= range.end)
    {
      return range.color;
    }
  }
  // Return a default color (e.g., white) if the value is not within any of the ranges
  return GREEN;
}

int checkRanges()
{

  oilTempWarning = oilPressWarning = fuelQTYWarning = fuelPressWarning = 0;

  uint32_t oilTempColor = getColorForValue(oilTempRange, oilTempRangeNum, sensorData.oilTemp);
  uint32_t oilPressColor = getColorForValue(oilPressRange, oilPressRangeNum, sensorData.oilPress);
  uint32_t fuelQtyColor = getColorForValue(fuelQTYRange, fuelQTYRangeNum, sensorData.fuelLitres);
  uint32_t fuelPressColor = getColorForValue(fuelPressRange, fuelPressRangeNum, sensorData.fuelPress);

  if (oilPressColor == YELLOW)
    oilPressWarning = 1;
  if (oilPressColor == RED)
    oilPressWarning = 2;

  if (oilTempColor == YELLOW)
    oilTempWarning = 1;
  if (oilTempColor == RED)
    oilTempWarning = 2;

  if (fuelPressColor == YELLOW)
    fuelPressWarning = 1;
  if (fuelPressColor == RED)
    fuelPressWarning = 2;

  if (fuelQtyColor == YELLOW)
  {
    fuelQTYWarning = 1;
  }

  if (fuelQtyColor == RED)
  {
    fuelQTYWarning = 2;
  }

  return oilTempWarning + oilPressWarning + fuelPressWarning + fuelQTYWarning;
}

void testDisplay()
{
  static int direction = 1;
  static int counter;
  static float batteryVoltage = 13.8;
  static float fuelLitres = 60;
  static float fuelPress = 250;
  static float oilPress = 4.5;
  static float oilTemp = 82;

  batteryVoltage += direction;
  fuelLitres += direction;
  fuelPress += direction;
  oilPress += (direction / 10.0);
  oilTemp += direction;

  sensorData.frame = counter;
  sensorData.batteryVoltage = batteryVoltage;
  sensorData.fuelLitres = fuelLitres;
  sensorData.fuelPress = fuelPress;
  sensorData.oilPress = oilPress;
  sensorData.oilTemp = oilTemp;

  sensorData.fuelQtyError = false;
  sensorData.fuelPressError = false;
  sensorData.oilPressError = false;
  sensorData.oilTempError = false;

  if (counter % 40 == 0)
    direction *= -1;

  counter++;
}

void drawTopBar()
{
  M5.Lcd.textsize = 2;
  m5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("BUS %4.1fV %2iA        %c %3.0f\n",
                sensorData.batteryVoltage,
                (int)sensorData.amp,
                sdPresent ? 'S' : '-',
                M5.Axp.GetBatteryLevel());

  //(int)M5.Axp.GetBatCurrent(),
}

void drawBottomBar()
{
  m5.Lcd.setCursor(0, 225);
  M5.Lcd.printf("G:%4.1f ", accY);

  m5.Lcd.setCursor(100, 225);
  M5.Lcd.printf("CHT:%3.0f ", sensorData.cht1);

  m5.Lcd.setCursor(230, 225);
  M5.Lcd.printf("%6i", sensorData.frame);
}

void loop()
{
  static unsigned long lastUpdated;

  if (sensorDataUpdated)
  {
    if (millis() - lastUpdated > 5000)
      M5.lcd.clearDisplay();

    if (SIMULATE)
      testDisplay();

    // check for anything in the red:
    int warning = checkRanges();
    if (warning > 1 && !MUTE)
    {
      alarmSound = 1;
    }
    else
    {
      alarmSound = 0;
    }

    // warnSprite.fillSprite(BLACK);
    // warnSprite.pushSprite(0, 220);

    M5.IMU.getAccelData(&accX, &accY, &accZ);

    // draw the actual gauges
    drawGauges();
    drawTopBar();
    drawBottomBar();

    sensorDataUpdated = false;
    lastUpdated = millis();
  }

  if (millis() - lastUpdated > 5000)
  {
    // M5.Lcd.fillScreen(BLACK);

    drawFatLine(0, 0, 320, 240, 10, RED);
    drawFatLine(0, 240, 320, 0, 10, RED);
    delay(100);
  }

  static int nextSecond = millis() + 1000;

  if (millis() > nextSecond)
  {

    getRtcTime(timeStr, sizeof(timeStr));
    debug.printf("i: Time %s\n", timeStr);
    nextSecond = millis() + 1000;
    writeSD();
  }
}