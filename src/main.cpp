// Minimal Arduino sketch for mdash.net
//
// - Install mDash library:
//   - Select "Sketch" &rarr; "Include Library" &rarr; "Manage Libraries"
//   - In the search field, type "mDash" and press Enter
//   - Click on "Install" to install the library
// - Select "Tools" → "Board" → "ESP32 Dev Module"
// - Select "Tools" → "Partitioning Scheme" → "Minimal SPIFFS"
// - Select "Tools" → "Port" → your serial port
// - Click on "Upload" button to build and flash the firmware
//
// See https://mdash.net/docs/ for the full IoT product reference impementation

#define MDASH_APP_NAME "MinimalApp"
//#include <mDash.h>

#include <Arduino.h>
#include <WiFi.h>
#define WIFI_NETWORK "bogonhat"
#define WIFI_PASSWORD ""
#define DEVICE_PASSWORD "KEp99DGyRpC90tAnOhBQ7kmg"

//for I2C with OLED
#include <Wire.h>

#include <DHT.h>
#include <VL53L0X.h>

#define DHTPIN 2 // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

VL53L0X tof_sensor;
// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE

// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

//#define HIGH_SPEED
//#define HIGH_ACCURACY


//for display
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)

/*
typedef enum
{
  MENU,      //Menu screen.
  DOWNLOADS, //Downloaded files screen
  ASK_HELP,  //Ask help screen
} Screen;

Screen currentScreen = MENU;
*/

bool temp_disp = true;

//init the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void text(String s, int size, int cX, int cY, bool clear, bool inverse)
{
  if (clear)
  {
    display.clearDisplay();
  }
  if (inverse)
  {
    display.setTextColor(BLACK, WHITE);
  }
  else
  {
    display.setTextColor(WHITE);
  }
  display.setTextSize(size);
  display.setCursor(cX, cY);
  display.println(s);
  display.display();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Started!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address ***** for 128x32 3D volt itt
    /*for debugging*/
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  text("Wifi connection...", 2, 0, 0, true, false);

  //WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  //mDashBegin(DEVICE_PASSWORD);

  text("Wifi connected!", 2, 0, 0, true, false);

  dht.begin();

  tof_sensor.setTimeout(500);
  if (!tof_sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1)
    {
    }
  }

  #if defined LONG_RANGE
    // lower the return signal rate limit (default is 0.25 MCPS)
    tof_sensor.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    tof_sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    tof_sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  #endif

  #if defined HIGH_SPEED
    // reduce timing budget to 20 ms (default is about 33 ms)
    tof_sensor.setMeasurementTimingBudget(20000);
  #elif defined HIGH_ACCURACY
    // increase timing budget to 200 ms
    tof_sensor.setMeasurementTimingBudget(200000);
  #endif
}

void loop()
{

  //read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  float tof = tof_sensor.readRangeSingleMillimeters();
  if (tof_sensor.timeoutOccurred())
  {
    Serial.println(" VL53L0X TIMEOUT");
  }

  // clear display
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(tof);
  display.print(" ");
  display.setTextSize(1);
  display.print("mm");

  if (temp_disp) {
    // display temp
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Temperature: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(t);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
  }
  else {
    // display humidity
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Humidity: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(h);
    display.print(" %");
  }
  temp_disp = not temp_disp;

  display.display();
  delay(2000);
}

