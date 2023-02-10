//
// My Analog Read esp
//
#include <RTOS.h>
#include <Arduino.h>
#include <ezButton.h>
#include <U8g2lib.h>
#include <my_esp32_ota.h>
#include <Adafruit_MCP3008.h>
#include <TaskScheduler.h>
#include <NeoPixelBrightnessBus.h> 
#include <SPI.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#include <my_wifi.h>

#define colorSaturation 128
#define APP_CPU 1
#define PRO_CPU 0

const uint16_t button1Pin = 12;
const uint16_t button2Pin = 14;
const uint16_t button3Pin = 27;
const uint16_t button4Pin = 26;
const uint16_t button5Pin = 25;
const uint16_t button6Pin = 32;
const uint16_t button7Pin = 33;
const uint16_t button8Pin = 4;
const uint16_t button9Pin = 0;
const uint16_t button10Pin = 2;
const uint16_t button11Pin = 15;
const uint16_t buttonDebounceTime = 25;
const uint16_t analogResolution = 9; // 9 = 512 bit, 12 = 4095 bit
const uint8_t analog1Pin = 35;
const uint8_t analog2Pin = 34;
const uint8_t analog3Pin = 39;
const uint8_t batteryPin = 36;
const uint8_t neoPixelPin = 16;
const uint16_t PixelCount = 4;        // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = neoPixelPin; // make sure to set this to the correct pin, ignored for Esp8266

// Ez Buttons
uint8_t button1State;
uint8_t button2State;
uint8_t button3State;
uint8_t button4State;
uint8_t button5State;
uint8_t button6State;
uint8_t button7State;
uint8_t button8State;
uint8_t button9State;
uint8_t button10State;
uint8_t button11State;

// analog read task vars

uint16_t knob1 = 0;
uint16_t knob2 = 0;
uint16_t knob3 = 0;
uint16_t knob4 = 0;
uint16_t lastKnob1 = 0;
uint16_t lastKnob2 = 0;
uint16_t lastKnob3 = 0;
uint16_t lastKnob4 = 0;
uint16_t stick1x = 0;
uint16_t stick1y = 0;
uint16_t stick2x = 0;
uint16_t stick2y = 0;
uint16_t stick3x = 0;
uint16_t stick3y = 0;
uint16_t stick4x = 0;
uint16_t stick4y = 0;
uint16_t lastStick1x = 0;
uint16_t lastStick1y = 0;
uint16_t lastStick2x = 0;
uint16_t lastStick2y = 0;
uint16_t lastStick3x = 0;
uint16_t lastStick3y = 0;
uint16_t lastStick4x = 0;
uint16_t lastStick4y = 0;
uint16_t mappedStick1x = 0;
uint16_t mappedStick1y = 0;
uint16_t mappedStick2x = 0;
uint16_t mappedStick2y = 0;
uint16_t mappedStick3x = 0;
uint16_t mappedStick3y = 0;
uint16_t mappedStick4x = 0;
uint16_t mappedStick4y = 0;
uint16_t lastMappedStick1x = 0;
uint16_t lastMappedStick1y = 0;
uint16_t lastMappedStick2x = 0;
uint16_t lastMappedStick2y = 0;
uint16_t lastMappedStick3x = 0;
uint16_t lastMappedStick3y = 0;
uint16_t lastMappedStick4x = 0;
uint16_t lastMappedStick4y = 0;
uint16_t batteryReading = 1;
uint16_t lastBatteryReading = 0;
uint16_t mappedKnob1 = 0;
uint16_t mappedKnob2 = 0;
uint16_t lastMappedKnob1 = 0;
uint16_t lastMappedKnob2 = 0;

String strStickReading;
String strStickMqttOutMsg;
String strStickReading1x;
String strStickReading1y;
String strStickReading2x;
String strStickReading2y;
String strStickReading3x;
String strStickReading3y;
String strStickReading4x;
String strStickReading4y;
String strKnobReading1;
String strKnobReading2;
String strKnobsOutMsg;

// Serial receive vars
const byte numChars = 100;
char receivedChars[numChars]; // array to store received SERIAL chars

volatile bool newData = false;
volatile bool newMqttMsg = false;
bool mqttConnected = false;
bool sendMqtt = true;
bool readKnobs = true;
bool stickPad1Active = true;
bool stickPad2Active = false;
bool pubStick1x = false;
bool pubStick1y = false;
bool pubStick2x = false;
bool pubStick2y = false;
bool pubStick3x = false;
bool pubStick3y = false;
bool pubStick4x = false;
bool pubStick4y = false;
bool pubKnob1 = false;
bool pubKnob2 = false;

NeoPixelBrightnessBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

// taskrunner tasks
void trBatteryTaskCb();
void trBeaconTaskCb();

Scheduler trRunner;

Task trBatteryTask(10000, TASK_FOREVER, &trBatteryTaskCb);
Task trBeaconTask(3000, TASK_FOREVER, &trBeaconTaskCb);
// Task t3(5000, TASK_FOREVER, &t3Callback);

// rtos tasks
void otaTask(void *pvParameters);
void mqttTask(void *pvParameters);
void analogReadTask(void *pvParameters);
void buttonTask(void *pvParameters);
void taskRunnerTask(void *pvParameters);

TaskHandle_t tOtaTask;
TaskHandle_t tMqttTask;
TaskHandle_t tAnalogReadTask;
TaskHandle_t tButtonTask;
TaskHandle_t tTaskRunnerTask;

void handleMqttMsg();

ezButton button1(button1Pin);
ezButton button2(button2Pin);
ezButton button3(button3Pin);
ezButton button4(button4Pin);
ezButton button5(button5Pin);
ezButton button6(button6Pin);
ezButton button7(button7Pin);
ezButton button8(button8Pin);
ezButton button9(button9Pin);
ezButton button10(button10Pin);
ezButton button11(button11Pin);

Adafruit_MCP3008 mcpAdc;

SemaphoreHandle_t mqttSemaphore;
SemaphoreHandle_t screenSemaphore;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
//  U8G2_SSD1306_128X64_ALT0_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem

// const uint8_t SCL = 22;
// const uint8_t SDA = 21;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);
  delay(1000);
  Serial.println("Begin--");
  analogReadResolution(9);
  mqttSemaphore = xSemaphoreCreateMutex();
  screenSemaphore = xSemaphoreCreateMutex();
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);
  pinMode(button5Pin, INPUT_PULLUP);
  pinMode(button6Pin, INPUT_PULLUP);
  pinMode(button7Pin, INPUT_PULLUP);
  pinMode(button8Pin, INPUT_PULLUP);
  pinMode(button9Pin, INPUT_PULLUP);
  pinMode(button10Pin, INPUT_PULLUP);
  pinMode(button11Pin, INPUT_PULLUP);
  pinMode(batteryPin, INPUT);
  pinMode(analog1Pin, INPUT);
  pinMode(analog2Pin, INPUT);
  pinMode(analog3Pin, INPUT);
  button1.setDebounceTime(buttonDebounceTime);
  button2.setDebounceTime(buttonDebounceTime);
  button3.setDebounceTime(buttonDebounceTime);
  button4.setDebounceTime(buttonDebounceTime);
  button5.setDebounceTime(buttonDebounceTime);
  button6.setDebounceTime(buttonDebounceTime);
  button7.setDebounceTime(buttonDebounceTime);
  button8.setDebounceTime(buttonDebounceTime);
  button9.setDebounceTime(buttonDebounceTime);
  button10.setDebounceTime(buttonDebounceTime);
  button11.setDebounceTime(buttonDebounceTime);
  strip.Begin();
    // // turn off the pixels
  strip.SetPixelColor(0, black);
  strip.SetPixelColor(1, black);
  strip.SetPixelColor(2, black);
  strip.SetPixelColor(3, black);
  strip.SetBrightness(4);
  strip.Show();
  mcpAdc.begin();
  u8g2.begin();
  scan_wifi();
  WiFi.setSleep(false);
  setup_wifi();
  WiFi.setSleep(false);
  strip.SetPixelColor(0, red);
  strip.Show();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(15, 25);
    // u8g2.print(F("Hello World!"));

    u8g2.println(strCurrentSSID);
    u8g2.println(strCurrentRSSI);
  } while (u8g2.nextPage());
  delay(1000);

  // Tasks
  // Start OTA task
  xTaskCreatePinnedToCore(
      otaTask,
      "ota_Task",
      4096,
      NULL,
      1,
      &tOtaTask,
      APP_CPU);
  // Start mqtt task
  xTaskCreatePinnedToCore(
      mqttTask,
      "mqtt_task",
      4096,
      NULL,
      1,
      &tMqttTask,
      APP_CPU);
  // Start button task
  xTaskCreatePinnedToCore(
      buttonTask,
      "button_task",
      4096,
      NULL,
      1,
      &tButtonTask,
      APP_CPU);
  // Start analog read task
  xTaskCreatePinnedToCore(
      analogReadTask,
      "analog_read_task",
      4096,
      NULL,
      1,
      &tAnalogReadTask,
      APP_CPU);
  xTaskCreatePinnedToCore(
      taskRunnerTask,
      "task_runner_task",
      4096,
      NULL,
      1,
      &tTaskRunnerTask,
      APP_CPU);
}

void loop()
{
  taskYIELD();
  delay(10000);
} // end loop

void taskRunnerTask(void *pvParameters)
{
  trRunner.init();
  trRunner.addTask(trBeaconTask);
  trRunner.addTask(trBatteryTask);
  trRunner.enableAll();
  for (;;)
  {
    trRunner.execute();
    taskYIELD();
  }
}

void analogReadTask(void *pvParameters)
{
  for (;;)
  {
    knob1 = analogRead(analog3Pin);
    knob2 = analogRead(analog2Pin);
    if ((knob1 - lastKnob1 > 5) || (lastKnob1 - knob1 > 5))
    {
      mappedKnob1 = map(knob1, 0, 512, 0, 100);
      if (mappedKnob1 != lastMappedKnob1)
      {
        pubKnob1 = true;
        lastMappedKnob1 = mappedKnob1;
        strKnobReading1 = mappedKnob1;
      }
      lastKnob1 = knob1;
    }
    if ((knob2 - lastKnob2 > 5) || (lastKnob2 - knob2 > 5))
    {
      mappedKnob2 = map(knob2, 0, 512, 0, 100);
      if (mappedKnob2 != lastMappedKnob2)
      {
        pubKnob2 = true;
        lastMappedKnob2 = mappedKnob2;
        strKnobReading2 = mappedKnob2;
      }
      lastKnob2 = knob2;
    }
    stick1x = mcpAdc.readADC(2);
    delay(2);
    stick1y = mcpAdc.readADC(3);
    delay(2);
    stick2x = mcpAdc.readADC(1);
    delay(2);
    stick2y = mcpAdc.readADC(0);
    delay(2);
    stick3x = mcpAdc.readADC(4);
    delay(2);
    stick3y = mcpAdc.readADC(5);
    delay(2);
    stick4x = mcpAdc.readADC(7);
    delay(2);
    stick4y = mcpAdc.readADC(6);
    delay(2);

    if ((stick1x - lastStick1x > 5) || (lastStick1x - stick1x > 5))
    {
      mappedStick1x = map(stick1x, 0, 1023, 0, 99);
      lastStick1x = stick1x;
      if (mappedStick1x != lastMappedStick1x)
      {
        strStickReading1x = mappedStick1x;
        lastMappedStick1x = mappedStick1x;
        pubStick1x = true;
      }
    }
    if ((stick1y - lastStick1y > 5) || (lastStick1y - stick1y > 5))
    {
      mappedStick1y = map(stick1y, 0, 1023, 0, 99);
      lastStick1y = stick1y;
      if (mappedStick1y != lastMappedStick1y)
      {
        strStickReading1y = mappedStick1y;
        lastMappedStick1y = mappedStick1y;
        pubStick1y = true;
      }
    }
    if ((stick2x - lastStick2x > 5) || (lastStick2x - stick2x > 5))
    {
      mappedStick2x = map(stick2x, 0, 1023, 0, 99);
      lastStick2x = stick2x;
      if (mappedStick2x != lastMappedStick2x)
      {
        strStickReading2x = mappedStick2x;
        lastMappedStick2x = mappedStick2x;
        pubStick2x = true;
      }
    }
    if ((stick2y - lastStick2y > 5) || (lastStick1x - stick2y > 5))
    {
      mappedStick2y = map(stick2y, 0, 1023, 0, 99);
      lastStick2y = stick2y;
      if (mappedStick2y != lastMappedStick2y)
      {
        strStickReading2y = mappedStick2y;
        lastMappedStick2y = mappedStick2y;
        pubStick2y = true;
      }
    }
    if ((stick3x - lastStick3x > 5) || (lastStick3x - stick3x > 5))
    {
      mappedStick3x = map(stick3x, 0, 1023, 0, 99);
      lastStick3x = stick3x;
      if (mappedStick3x != lastMappedStick3x)
      {
        strStickReading3x = mappedStick3x;
        lastMappedStick3x = mappedStick3x;
        pubStick3x = true;
      }
    }
    if ((stick3y - lastStick3y > 5) || (lastStick3y - stick3y > 5))
    {
      mappedStick3y = map(stick3y, 0, 1023, 0, 99);
      lastStick3y = stick3y;

      if (mappedStick3y != lastMappedStick3y)
      {
        strStickReading3y = mappedStick3y;
        lastMappedStick3y = mappedStick3y;
        pubStick3y = true;
      }
    }
    if ((stick4x - lastStick4x > 5) || (lastStick4x - stick4x > 5))
    {
      mappedStick4x = map(stick4x, 0, 1023, 0, 99);
      lastStick4x = stick4x;
      if (mappedStick4x != lastMappedStick4x)
      {
        strStickReading4x = mappedStick4x;
        lastMappedStick4x = mappedStick4x;
        pubStick4x = true;
      }
    }
    if ((stick4y - lastStick4y > 5) || (lastStick4y - stick4y > 5))
    {
      mappedStick4y = map(stick4y, 0, 1023, 0, 99);
      lastStick4y = stick4y;
      if (mappedStick4y != lastMappedStick4y)
      {
        strStickReading4y = mappedStick4y;
        lastMappedStick4y = mappedStick4y;
        pubStick4y = true;
      }
    }
    xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
    if (pubKnob1)
    {
      strKnobsOutMsg = "kn1:" + strKnobReading1;
      myPublish(knobsTopic, strKnobsOutMsg);
      pubKnob1 = false;
    }
    if (pubKnob2)
    {
      strKnobsOutMsg = "kn2:" + strKnobReading2;
      myPublish(knobsTopic, strKnobsOutMsg);
      pubKnob2 = false;
    }
    if (pubStick1x)
    {
      strStickMqttOutMsg = "s1x:" + strStickReading1x;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick1x = false;
    }
    if (pubStick1y)
    {
      strStickMqttOutMsg = "s1y:" + strStickReading1y;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick1y = false;
    }
    if (pubStick2x)
    {
      strStickMqttOutMsg = "s2x:" + strStickReading2x;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick2x = false;
    }
    if (pubStick2y)
    {
      strStickMqttOutMsg = "s2y:" + strStickReading2y;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick2y = false;
    }
    if (pubStick3x)
    {
      strStickMqttOutMsg = "s3x:" + strStickReading3x;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick3x = false;
    }
    if (pubStick3y)
    {
      strStickMqttOutMsg = "s3y:" + strStickReading3y;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick3y = false;
    }
    if (pubStick4x)
    {
      strStickMqttOutMsg = "s4x:" + strStickReading4x;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick4x = false;
    }
    if (pubStick4y)
    {
      strStickMqttOutMsg = "s4y:" + strStickReading4y;
      myPublish(sticksTopic, strStickMqttOutMsg);
      pubStick4y = false;
    }
    xSemaphoreGive(mqttSemaphore);
    taskYIELD();
    vTaskDelay(pdTICKS_TO_MS(25));
  }
}

void otaTask(void *pvParameters)
{
  startOtaServer();
  for (;;)
  {
    otaServer.handleClient();
    taskYIELD();
  }
}

void buttonTask(void *pvParameters)
{
  String strButtMqttOutMsg;
  for (;;)
  {
    /*
     * Created by ArduinoGetStarted.com
     *
     * This example code is in the public domain
     *
     * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-button-library
     *
     * This example:
     *   + uses debounce for a button.
     *   + reads state of a button
     *   + detects the pressed and released events of a button
     */
    strButtMqttOutMsg = "x";
    button1.loop();
    button2.loop();
    button3.loop();
    button4.loop();
    button5.loop();
    button6.loop();
    button7.loop();
    button8.loop();
    button9.loop();
    button10.loop();
    button11.loop();
    button1State = button1.getState();
    button2State = button2.getState();
    button3State = button3.getState();
    button4State = button4.getState();
    button5State = button5.getState();
    button6State = button6.getState();
    button7State = button7.getState();
    button8State = button8.getState();
    button9State = button9.getState();
    button10State = button10.getState();
    button11State = button11.getState();
    if (button1.isPressed())
    {
      strButtMqttOutMsg = "b1:d";

    }
    // if (button1.isReleased())
    // {
    // myPublish("b1:released");
    // }
    else if (button2.isPressed())
    {
      strButtMqttOutMsg = "b2:d";
    }
    else if (button3.isPressed())
    {
      strButtMqttOutMsg = "b3:d";
    }
    else if (button4.isPressed())
    {
      strButtMqttOutMsg = "b4:d";
    }
    else if (button5.isPressed())
    {
      strButtMqttOutMsg = "b5:d";
    }
    else if (button6.isPressed())
    {
      strButtMqttOutMsg = "b6:d";
    }
    else if (button7.isPressed())
    {
      strButtMqttOutMsg = "b7:d";
    }
    else if (button8.isPressed())
    {
      strButtMqttOutMsg = "b8:d";
    }
    else if (button9.isPressed())
    {
      strButtMqttOutMsg = "b9:d";
    }
    else if (button10.isPressed())
    {
      strButtMqttOutMsg = "b10:d";
    }
    else if (button11.isPressed())
    {
      strButtMqttOutMsg = "b11:d";
    }
    if ((sendMqtt) && (strButtMqttOutMsg != "x"))
    {
      xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
      myPublish(buttonsTopic, strButtMqttOutMsg);
      xSemaphoreGive(mqttSemaphore);
      strButtMqttOutMsg = "x";
      // Serial1.println(strMqttMessage);
    }
    taskYIELD();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void mqttTask(void *pvParameters)
{
  for (;;)
  {
    mqttClient.loop();
    if (newMqttInMsg == true)
    {
      // myPublish("Got the mqtt message in mqtt task");
      handleMqttMsg();
      newMqttInMsg = false;
    }
    taskYIELD();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void trBatteryTaskCb()
{
  batteryReading = analogRead(batteryPin);
  if (batteryReading != lastBatteryReading)
  {
    // xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
    // myPublish(batteryTopic, batteryReading);
    // xSemaphoreGive(mqttSemaphore);
    xSemaphoreTake(screenSemaphore, portMAX_DELAY);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.firstPage();
    do
    {
      u8g2.setCursor(15, 50);
      // u8g2.print(F("Hello World!"));

      u8g2.println("Batt: ");
      u8g2.println(batteryReading);
    } while (u8g2.nextPage());
    xSemaphoreGive(screenSemaphore);
  }
  lastBatteryReading = batteryReading;
  if (batteryReading < 230)
  {
    strip.SetPixelColor(0, red);
    strip.Show();
  }
}

void trBeaconTaskCb()
{
  xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
  myPublish(beaconTopic, "online");
  xSemaphoreGive(mqttSemaphore);
  // xSemaphoreTake(screenSemaphore, portMAX_DELAY);
  // u8g2.setFont(u8g2_font_ncenB14_tr);
  // u8g2.firstPage();
  // do
  // {
  //   u8g2.setCursor(15, 25);
  //   // u8g2.print(F("Hello World!"));
  //   strCurrentSSID = WiFi.SSID();
  //   strCurrentRSSI = WiFi.RSSI();
  //   u8g2.println(strCurrentSSID);
  //   u8g2.println(strCurrentRSSI);
  // } while (u8g2.nextPage());
  // xSemaphoreGive(screenSemaphore);
}

void handleMqttMsg()
{
  // myPublish(statusTopic, mqttInTopic);
  // myPublish(statusTopic, mqttInMessage);
  if (mqttInMessage == "RESET")
  {
    myPublish("Resetting now");
    delay(1000);
    ESP.restart();
  }
}

// Serial.print("MOSI Pin: ");
// Serial.println(MOSI);
// Serial.print("MISO Pin: ");
// Serial.println(MISO);
// Serial.print("SCK Pin: ");
// Serial.println(SCK);
// Serial.print("SS Pin: ");
// Serial.println(SS);

// Averaging analog read vars
// int XreadIndex1 = 0; // the index of the current reading
// int YreadIndex1 = 0;
// int totalX1 = 0; // the running total
// int totalY1 = 0;
// int averageX1 = 0; // the average
// int averageY1 = 0;
// int readingsX1[numreadingsX1]; // the readingsX1 from the analog input
// int readingsY1[numreadingsY1];
// int XreadIndex2 = 0; // the index of the current reading
// int YreadIndex2 = 0;
// int totalX2 = 0; // the running total
// int totalY2 = 0;
// int averageX2 = 0; // the average
// int averageY2 = 0;
// int readingsX2[numreadingsX2]; // the readingsX2 from the analog input
// int readingsY2[numreadingsY2];
// const int numreadingsX1 = 5;
// const int numreadingsY1 = 5;
// const int numreadingsX2 = 5;
// const int numreadingsY2 = 5;

 // strip.SetPixelColor(0, red);
  // strip.SetPixelColor(1, green);
  // strip.SetPixelColor(2, blue);
  // //strip.SetPixelColor(3, white);
  // strip.SetBrightness(8);
  // strip.Show();


  // Serial.println("Off ...");

  // // turn off the pixels
  // strip.SetPixelColor(0, black);
  // strip.SetPixelColor(1, black);
  // strip.SetPixelColor(2, black);
  // strip.SetPixelColor(3, black);
  // strip.Show();

  // delay(5000);

  // Serial.println("HSL Colors R, G, B, W...");

  // // set the colors,
  // // if they don't match in order, you may need to use NeoGrbFeature feature
  // strip.SetPixelColor(0, hslRed);
  // strip.SetPixelColor(1, hslGreen);
  // strip.SetPixelColor(2, hslBlue);
  // strip.SetPixelColor(3, hslWhite);
  // // strip.Show();

  // delay(5000);

  // Serial.println("Off again...");

  // // turn off the pixels
  // strip.SetPixelColor(0, hslBlack);
  // strip.SetPixelColor(1, hslBlack);
  // strip.SetPixelColor(2, hslBlack);
  // strip.SetPixelColor(3, hslBlack);
  // strip.Show();