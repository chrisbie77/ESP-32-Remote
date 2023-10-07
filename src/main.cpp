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
#include <main.h>
#include <my_wifi.h>

#define APP_CPU 1
#define PRO_CPU 0
#define LOCAL_BATTERY_MODE 0
#define REMOTE_BATTERY_MODE 1

void setup()
{
  Serial.begin(9600);
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
  setup_wifi();
  strip.SetPixelColor(0, red);
  strip.Show();
  u8g2.setFont(u8g2_font_8bitclassic_tr);
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(15, 25);
    // u8g2.print(F("Hello World!"));

    u8g2.println(strCurrentSSID);
    u8g2.println(strCurrentRSSI);
  } while (u8g2.nextPage());

  trRunner.addTask(trBatteryTask);
  trRunner.addTask(trScreenUpdateTask);
  trRunner.enableAll();
  delay(100);

  // Tasks
  // Start OTA task
  xTaskCreate(
      otaTask,
      "ota_Task",
      4096,
      NULL,
      1,
      &tOtaTask);
  // APP_CPU);
  // Start mqtt task
  xTaskCreate(
      mqttTask,
      "mqtt_task",
      8128,
      NULL,
      1,
      &tMqttTask);
  //     APP_CPU);
  // Start analog read task
  xTaskCreate(
      analogReadTask,
      "analog_read_task",
      8128,
      NULL,
      1,
      &tAnalogReadTask);
  //     APP_CPU);
  xTaskCreate(
      taskRunnerTask,
      "task_runner_task",
      8128,
      NULL,
      1,
      &tTaskRunnerTask);
  //     APP_CPU);
}

void loop()
{
  // taskYIELD();
  // vTaskDelay(pdMS_TO_TICKS(60000));
  vTaskDelay(1000);
  delay(10000);
} // end loop

void taskRunnerTask(void *pvParameters)
{
  for (;;)
  {
    trRunner.execute();
    // vTaskDelay(pdTICKS_TO_MS(100));
    // taskYIELD();
    vTaskDelay(10);
  }
}

void analogReadTask(void *pvParameters)
{
  for (;;)
  {
    xSemaphoreTake(mqttSemaphore, portMAX_DELAY);
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
    // delay(2);
    stick1y = mcpAdc.readADC(3);
    // delay(2);
    stick2x = mcpAdc.readADC(1);
    // delay(2);
    stick2y = mcpAdc.readADC(0);
    // delay(2);
    stick3x = mcpAdc.readADC(4);
    // delay(2);
    stick3y = mcpAdc.readADC(5);
    // delay(2);
    stick4x = mcpAdc.readADC(7);
    // delay(2);
    stick4y = mcpAdc.readADC(6);
    // delay(2);

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
    if ((stick2y - lastStick2y > 5) || (lastStick2y - stick2y > 5))
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
      myPublish(buttonsTopic, strButtMqttOutMsg);
      strButtMqttOutMsg = "x";
    }
    xSemaphoreGive(mqttSemaphore);
    // taskYIELD();
    // vTaskDelay(pdTICKS_TO_MS(25));
    vTaskDelay(10);
  }
}

void otaTask(void *pvParameters)
{
  startOtaServer();
  for (;;)
  {
    otaServer.handleClient();
    vTaskDelay(10);
    // taskYIELD();
  }
}

void mqttTask(void *pvParameters)
{
  for (;;)
  {
    mqttClient.loop();
    if (newMqttInMsg == true)
    {
      if (strMqttTopic == truckTeleTopic)
      {
        strScreenLine2 = strMqttMessage;
      }
      newMqttInMsg = false;
      newScreenMessage = true;
    }
    // taskYIELD();
    // vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelay(10);
  }
}

void trBatteryTaskCb()
{
  strCurrentRSSI = WiFi.RSSI();
  if (strCurrentRSSI != strLastRSSI)
  {
    newScreenMessage = true;
    strLastRSSI = strCurrentRSSI;
  }
  batteryReading = analogRead(batteryPin);
  if (batteryReading != lastBatteryReading)
  {
    mappedBatteryReading = map(batteryReading, 220, 290, 0, 100);
    // xSemaphoreTake(screenSemaphore, portMAX_DELAY);
    // u8g2.setFont(u8g2_font_ncenB14_tr);
    // u8g2.firstPage();
    // do
    // {
    //   //u8g2.setCursor(15, 30);
    //   //u8g2.println(strCurrentSSID);
    //   //u8g2.println("  ");
    //   u8g2.println(strCurrentRSSI);
    //   u8g2.setCursor(15, 30);
    //   u8g2.println("B1: ");
    //   u8g2.println(mappedBatteryReading);
    //   u8g2.println("%");
    // } while (u8g2.nextPage());
    // xSemaphoreGive(screenSemaphore);
    lastBatteryReading = batteryReading;
    strBatteryReading = mappedBatteryReading;
    newScreenMessage = true;
  }
}

void trScreenUpdateTaskCb()
{
  if (newScreenMessage)
  {
    strScreenLine1 = strCurrentRSSI;
    strScreenLine1 += "db   ";
    strScreenLine1 += strBatteryReading;
    strScreenLine1 += "%";
    xSemaphoreTake(screenSemaphore, portMAX_DELAY);
    u8g2.setFont(u8g2_font_8bitclassic_tr);
    u8g2.firstPage();
    do
    {
      u8g2.setCursor(5, 20);
      u8g2.println(strScreenLine1);
      u8g2.setCursor(5, 50);
      u8g2.println(strScreenLine2);

    } while (u8g2.nextPage());
    xSemaphoreGive(screenSemaphore);
    newScreenMessage = false;
  }
}
