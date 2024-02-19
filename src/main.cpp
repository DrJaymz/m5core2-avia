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
int muteUntil = 60;
int lcdVoltage = 3100;

#define GAUGE_WIDTH 195
#define GAUGE_HEIGHT 34
#define PANEL_WIDTH 320 - GAUGE_WIDTH - 5
#define PANEL_HEIGHT GAUGE_HEIGHT
#define RADIO_SLEEP_MS 500
#define ENABLE_IMU 0

int secondsSinceBoot()
{
  return (int)millis() / 1000;
}

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

void set40Mhz()
{
  uint32_t f = getCpuFrequencyMhz();
  sprintf(string, "CPU Freq: %i", f);
  M5.Lcd.println(string);

  f = 40;

  sprintf(string, "Trying Freq:%i", f);
  M5.Lcd.println(string);

  bool retVal = setCpuFrequencyMhz(f);
  f = getCpuFrequencyMhz();

  sprintf(string, "Actual Freq:%i retVal:%d", f, retVal);
  M5.Lcd.println(string);
}

void SleepProcessor(uint64_t time_in_us)
{
  if (time_in_us > 0)
  {
    esp_sleep_enable_timer_wakeup(time_in_us);
  }
  else
  {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  }
  esp_light_sleep_start();
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

  // set40Mhz();
  // M5.Lcd.println("CPU complete");

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
  M5.Axp.SetLcdVoltage(lcdVoltage);

  beginSD();

  soundsBeep(2700, 500, 100);

  if (ENABLE_IMU)
    M5.IMU.Init(); // Init IMU sensor.

  xTaskCreatePinnedToCore(soundTask, "soundTask", 4096, NULL, 1, NULL, 0);

  espnow.pauseWiFi();
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
  // pSprite.textsize = 1;
  pSprite.textdatum = TC_DATUM;
  pSprite.drawString(name, 24, 2);

  pSprite.setFreeFont(&FreeSansBold9pt7b);
  pSprite.drawString(units, 24, 18);

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
  pSprite.drawString(string, 85, 3, GFXFF);

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
  drawPanelSprite(M5.Lcd, 0, y, "FUEL", "Ltr", (int)sensorData.fuelLitres, false, fuelQTYWarning);
  drawGaugeSprite(M5.lcd, x, y, 0, 120, (int)sensorData.fuelLitres, sensorData.fuelQtyError, labels, 6, fuelQTYRange, fuelQTYRangeNum);

  delete[] labels;

  y += gauageIncrement;

  labels = new int[5]{0, 50, 150, 250, 350};
  drawPanelSprite(M5.Lcd, 0, y, "FUEL", "mb", (int)sensorData.fuelPress, false, fuelPressWarning);
  drawGaugeSprite(M5.lcd, x, y, 0, 350, (int)sensorData.fuelPress, sensorData.fuelPressError, labels, 5, fuelPressRange, fuelPressRangeNum);
  delete[] labels;

  y += gauageIncrement;

  labels = new int[5]{40, 60, 80, 110, 130};
  drawPanelSprite(M5.Lcd, 0, y, "O I L", "C", (int)sensorData.oilTemp, false, oilTempWarning);
  drawGaugeSprite(M5.lcd, x, y, 30, 140, (int)sensorData.oilTemp, sensorData.oilTempError, labels, 5, oilTempRange, oilTempRangeNum);
  delete[] labels;

  y += gauageIncrement;

  labels = new int[4]{0, 2, 4, 6};
  drawPanelSprite(M5.Lcd, 0, y, "O I L", "bar", sensorData.oilPress, true, oilPressWarning);
  drawGaugeSprite(M5.lcd, x, y, 0, 7, sensorData.oilPress, sensorData.oilPressError, labels, 4, oilPressRange, oilPressRangeNum);

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
  M5.Lcd.textcolor = GREEN;
  M5.Lcd.textbgcolor = BLACK;
  M5.Lcd.textsize = 2;

  if (sensorData.batteryVoltage < 12.5 || sensorData.amp < 0)
  {
    M5.Lcd.textcolor = YELLOW;
  }

  if (sensorData.batteryVoltage < 12.0)
  {
    M5.Lcd.textcolor = WHITE;
    M5.Lcd.textbgcolor = RED;
  }

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("BUS %4.1fV %2iA ",
                sensorData.batteryVoltage,
                (int)sensorData.amp);

  M5.Lcd.textcolor = WHITE;
  M5.Lcd.textbgcolor = BLACK;

  M5.Lcd.setCursor(200, 0);
  // M5.Lcd.printf("%c %3.0f %d ",
  //               sdPresent ? 'S' : '-',
  //               M5.Axp.GetBatteryLevel(),
  //               M5.Axp.isCharging());

  M5.Lcd.printf("%c %3.0f %3.0f ",
                sdPresent ? 'S' : '-',
                M5.Axp.GetBatteryLevel(),
                M5.Axp.GetBatCurrent());

  for (size_t i = 0; i < 3; i++)
  {
    M5.Lcd.drawFastHLine(0, 20 + i, M5.Lcd.width(), WHITE);
  }

  //(int)M5.Axp.GetBatCurrent(),
}

void drawBottomBar()
{
  M5.Lcd.textcolor = WHITE;
  M5.Lcd.textbgcolor = BLACK;

  if (ENABLE_IMU)
  {
    m5.Lcd.setCursor(0, 225);
    M5.Lcd.printf("G:%4.1f ", accY);
  }

  m5.Lcd.setCursor(100, 225);
  M5.Lcd.printf("CHT:%3.0f ", sensorData.cht1);

  m5.Lcd.setCursor(230, 225);
  M5.Lcd.printf("%6i ", sensorData.frame);
}

void loop()
{
  static unsigned long lastUpdated;
  static int nextSecond = millis() + 1000;
  static int nextUpdate = millis() + RADIO_SLEEP_MS;
  static bool newData = false;

  // very rough touch detection - i.e. doesn't work when device sleeping. 
  // touch on the left will decrease brightness touch on the right will increase brightness.
  if (M5.Touch.ispressed())
  {

    muteUntil = secondsSinceBoot() + 60;
    Point p = M5.Touch.getPressPoint();

    // Get touch position
    int16_t x = M5.Touch.point->x;
    int16_t y = M5.Touch.point->y;

    // right touch - brighter
    if (x > 160 && x < 320 && lcdVoltage < 3300)
    {
      lcdVoltage += 100;
      soundsBeep(2700, 25, 100);
    }

    // left touch - dimmer
    if (x > 10 && x < 160 && lcdVoltage > 2500)
    {
      lcdVoltage -= 100;
      soundsBeep(2500, 25, 100);
    }

    Serial.printf("Touch position: x=%d, y=%d LCD=%d\n", x, y, lcdVoltage);

    M5.Axp.SetLcdVoltage(lcdVoltage);
  }

  if (sensorDataUpdated)
  {

    if (POWER_SAVE)
      espnow.pauseWiFi();

    if (millis() - lastUpdated > 5000)
      M5.lcd.clearDisplay();

    if (SIMULATE)
      testDisplay();

    // check for anything in the red:
    int warning = checkRanges();
    if (warning > 1 && !MUTE && secondsSinceBoot() > muteUntil)
    {
      alarmSound = 1;
    }
    else
    {
      alarmSound = 0;
    }

    if (ENABLE_IMU)  M5.IMU.getAccelData(&accX, &accY, &accZ);

    // draw the actual gauges
    drawGauges();
    drawTopBar();
    drawBottomBar();

    sensorDataUpdated = false;
    lastUpdated = millis();
    newData = true;
    SleepProcessor(1000 * RADIO_SLEEP_MS);
  }

  //check for lack of valid data for 5 seconds - if so then the display must change to indicate unreliable data.
  if (millis() - lastUpdated > 5000)
  {
    drawGauges();
    drawFatLine(0, 0, 320, 240, 10, RED);
    drawFatLine(0, 240, 320, 0, 10, RED);

    drawTopBar();
    drawBottomBar();

    if (POWER_SAVE)
      espnow.pauseWiFi();
    SleepProcessor(1000000); // low power sleep for 1 sec
    
  }

  // task for every second
  if (millis() > nextSecond)
  {

    getRtcTime(timeStr, sizeof(timeStr));
    debug.printf("i: Time %s\n", timeStr);
    nextSecond = millis() + 1000;
    //only if there is actually received valid data, then write to the SD card.
    if(newData) writeSD();
  }

  //allow new data
  if (millis() > nextUpdate)
  {
    nextUpdate = millis() + RADIO_SLEEP_MS;
    if (POWER_SAVE)
      espnow.resumeWiFi();
  }
}