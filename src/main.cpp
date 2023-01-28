//
// My Analog Read esp
//
#include <RTOS.h>
#include <Arduino.h>
#include <my_wifi.h>
#include <ezButton.h>
#include <U8g2lib.h>
#include <my_esp32_ota.h>
#include <Adafruit_MCP3008.h>
#include <SPI.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define APP_CPU 1
#define PRO_CPU 0

Adafruit_MCP3008 mcpAdc;

const int batteryPin = 36;
const int button1Pin = 12;
const int button2Pin = 14;
const int button3Pin = 27;
const int button4Pin = 26;
const int button5Pin = 25;
const int button6Pin = 33;
const int button7Pin = 32;

const int analogResolution = 9; // 9 = 512 bit, 12 = 4095 bit

int stickValueX1 = 0;
int stickValueY1 = 0;
int stickValueX2 = 0;
int stickValueY2 = 0;
int lastStickValueX1 = 0;
int lastStickValueY1 = 0;
int lastStickValueX2 = 0;
int lastStickValueY2 = 0;
// Ez Buttons
int button1State;
int button2State;
int button3State;
int button4State;
int button5State;
int button6State;
int button7State;


// Serial receive vars
const byte numChars = 100;
char receivedChars[numChars]; // array to store received SERIAL chars

volatile bool newData = false;
volatile bool newMqttMsg = false;
bool mqttConnected = false;
bool sendMqtt = true;
bool doShutDown = false;
bool lowBattery = false;

const int buttonDebounceTime = 50;

ezButton button1(button1Pin);
ezButton button2(button2Pin);
ezButton button3(button3Pin);
ezButton button4(button4Pin);
ezButton button5(button5Pin);
ezButton button6(button6Pin);
ezButton button7(button7Pin);

SemaphoreHandle_t mqttSemaphore;

TaskHandle_t tOtaTask;
TaskHandle_t tMqttTask;
TaskHandle_t tAnalogReadTask;
TaskHandle_t tButtonTask;

void otaTask(void *pvParameters);
void mqttTask(void *pvParameters);
void analogReadTask(void *pvParameters);
void buttonTask(void *pvParameters);

void handleMqttMsg();

// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
//  U8G2_SSD1306_128X64_ALT0_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem

// const uint8_t SCL = 22;
// const uint8_t SDA = 21;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);
  delay(1000);
  Serial.println("Begin--");
  Serial.print("MOSI Pin: ");
  Serial.println(MOSI);
  Serial.print("MISO Pin: ");
  Serial.println(MISO);
  Serial.print("SCK Pin: ");
  Serial.println(SCK);
  Serial.print("SS Pin: ");
  Serial.println(SS);
  mqttSemaphore = xSemaphoreCreateMutex();
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);
  pinMode(button5Pin, INPUT_PULLUP);
  pinMode(button6Pin, INPUT_PULLUP);
  pinMode(button7Pin, INPUT_PULLUP);
  // pinMode(batteryPin, INPUT);
  button1.setDebounceTime(buttonDebounceTime);
  button2.setDebounceTime(buttonDebounceTime);
  button3.setDebounceTime(buttonDebounceTime);
  button4.setDebounceTime(buttonDebounceTime);
  button5.setDebounceTime(buttonDebounceTime);
  button6.setDebounceTime(buttonDebounceTime);
  button7.setDebounceTime(buttonDebounceTime);
  mcpAdc.begin();
  // u8g2.begin();
  scan_wifi();
  WiFi.setSleep(false);
  setup_wifi();
  WiFi.setSleep(false);
  delay(1000);
  mqttConnect();
  //  showDisplay();
  delay(1000);
  // Tasks
  // Start OTA task
  xTaskCreatePinnedToCore(
      otaTask,
      "ota_Task",
      4096,
      NULL,
      2,
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
}

void loop()
{
  // if (doShutDown == true)
  // {
  //   // do shutdown
  //   ;
  // }

  yield();
  delay(5000);

} // end loop

void analogReadTask(void *pvParameters)
{
  uint16_t stick1x;
  uint16_t stick1y;
  uint16_t stick2x;
  uint16_t stick2y;
  uint16_t stick3x;
  uint16_t stick3y;
  uint16_t stick4x;
  uint16_t stick4y;
  uint16_t lastStick1x = 0;
  uint16_t lastStick1y = 0;
  uint16_t lastStick2x = 0;
  uint16_t lastStick2y = 0;
  uint16_t lastStick3x = 0;
  uint16_t lastStick3y = 0;
  uint16_t lastStick4x = 0;
  uint16_t lastStick4y = 0;
  uint16_t mappedStick1x;
  uint16_t mappedStick1y;
  uint16_t mappedStick2x;
  uint16_t mappedStick2y;
  uint16_t mappedStick3x;
  uint16_t mappedStick3y;
  uint16_t mappedStick4x;
  uint16_t mappedStick4y;
  uint16_t lastMappedStick1x;
  uint16_t lastMappedStick1y;
  uint16_t lastMappedStick2x;
  uint16_t lastMappedStick2y;
  uint16_t lastMappedStick3x;
  uint16_t lastMappedStick3y;
  uint16_t lastMappedStick4x;
  uint16_t lastMappedStick4y;
  bool pubStick1x = false;
  bool pubStick1y = false;
  bool pubStick2x = false;
  bool pubStick2y = false;
  bool pubStick3x = false;
  bool pubStick3y = false;
  bool pubStick4x = false;
  bool pubStick4y = false;
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
  for (;;)
  {
    stick1x = mcpAdc.readADC(3);
    delay(5);
    stick1y = mcpAdc.readADC(2);
    delay(5);
    stick2x = mcpAdc.readADC(0);
    delay(5);
    stick2y = mcpAdc.readADC(1);
    delay(5);
    stick3x = mcpAdc.readADC(4);
    delay(5);
    stick3y = mcpAdc.readADC(5);
    delay(5);
    stick4x = mcpAdc.readADC(7);
    delay(5);
    stick4y = mcpAdc.readADC(6);
    delay(5);
    if (stick1x != lastStick1x)
    {
      mappedStick1x = map(stick1x, 0, 1024, 0, 100);
      lastStick1x = stick1x;
    }
    if (stick1y != lastStick1y)
    {
      mappedStick1y = map(stick1y, 0, 1024, 0, 100);
      lastStick1y = stick1y;
    }   
    if (stick2x != lastStick2x)
    {
      mappedStick2x = map(stick2x, 0, 1024, 0, 100);
      lastStick2x = stick2x;
    }
    if (stick2y != lastStick1y)
    {
      mappedStick2y = map(stick2y, 0, 1024, 0, 100);
      lastStick2y = stick2y;
    }   
    if (stick3x != lastStick3x)
    {
      mappedStick3x = map(stick3x, 0, 1024, 0, 100);
      lastStick3x = stick3x;
    }
    if (stick3y != lastStick3y)
    {
      mappedStick3y = map(stick3y, 0, 1024, 0, 100);
      lastStick3y = stick3y;
    }
    if (stick4x != lastStick4x)
    {
      mappedStick4x = map(stick4x, 0, 1024, 0, 100);
      lastStick4x = stick4x;
    }
    if (stick4y != lastStick4y)
    {
      mappedStick4y = map(stick4y, 0, 1024, 0, 100);
      lastStick4y = stick4y;
    }
    if (mappedStick1x != lastMappedStick1x)
    {
      strStickReading1x = mappedStick1x;
      lastMappedStick1x = mappedStick1x;
      pubStick1x = true;
    }
    if (mappedStick1y != lastMappedStick1y)
    {
      strStickReading1y = mappedStick1y;
      lastMappedStick1y = mappedStick1y;
      pubStick1y = true;
    }
    if (mappedStick2x != lastMappedStick2x)
    {
      strStickReading2x = mappedStick2x;
      lastMappedStick2x = mappedStick2x;
      pubStick2x = true;
    }
    if (mappedStick2y != lastMappedStick2y)
    {
      strStickReading2y = mappedStick2y;
      lastMappedStick2y = mappedStick2y;
      pubStick2y = true;
    }
    if (mappedStick3x != lastMappedStick3x)
    {
      strStickReading3x = mappedStick3x;
      lastMappedStick3x = mappedStick3x;
      pubStick3x = true;
    }
    if (mappedStick3y != lastMappedStick3y)
    {
      strStickReading3y = mappedStick3y;
      lastMappedStick3y = mappedStick3y;
      pubStick3y = true;
    }
    if (mappedStick4x != lastMappedStick4x)
    {
      strStickReading4x = mappedStick4x;
      lastMappedStick4x = mappedStick4x;
      pubStick4x = true;
    }
    if (mappedStick4y != lastMappedStick4y)
    {
      strStickReading4y = mappedStick4y;
      lastMappedStick4y = mappedStick4y;
      pubStick4y = true;
    }
    xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
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
    delay(25);
  }
}

void otaTask(void *pvParameters)
{
  startOtaServer();
  for (;;)
  {
    otaServer.handleClient();
    yield();
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
    button1State = button1.getState();
    button2State = button2.getState();
    button3State = button3.getState();
    button4State = button4.getState();
    button5State = button5.getState();
    button6State = button6.getState();
    button7State = button7.getState();
    if (button1.isPressed())
    {
      // myPublish("b1:pressed");
      strButtMqttOutMsg = "b1:d";
    }
    // if (button1.isReleased())
    // {
    //   // myPublish("b1:released");
    // }
    else if (button2.isPressed())
    {
      strButtMqttOutMsg = "b2:d";
    }
    // if (button2.isReleased())
    // {
    //   // myPublish(buttonsTopic, "b2:released");
    // }
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
    if ((sendMqtt) && (strButtMqttOutMsg != "x"))
    {
      xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
      myPublish(buttonsTopic, strButtMqttOutMsg);
      xSemaphoreGive(mqttSemaphore);
      // Serial1.println(strMqttMessage);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
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
    yield();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void handleMqttMsg()
{
  myPublish(statusTopic, mqttInTopic);
  myPublish(statusTopic, mqttInMessage);
}



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