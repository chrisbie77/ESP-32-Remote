#define colorSaturation 128



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

// analo#define colorSaturation 128g read task vars

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
uint16_t mappedBatteryReading = 0;
uint16_t lastMappedBatteryReading = 0;
uint16_t mappedKnob1 = 0;
uint16_t mappedKnob2 = 0;
uint16_t lastMappedKnob1 = 0;
uint16_t lastMappedKnob2 = 0;
uint16_t s1yDeadzone = 4;
uint16_t s2yDeadzone = 4;

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
String strScreenLine1 = "Line1";
String strScreenLine2 = "Line2";
String strButtMqttOutMsg;
String strBatteryReading = "x";

const byte numChars = 100;

char receivedChars[numChars]; // array to store received SERIAL chars

bool newData = false;
bool newMqttMsg = false;
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
bool newScreenMessage = false;

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

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);

// taskrunner tasks
void trBatteryTaskCb();
void trScreenUpdateTaskCb();

Scheduler trRunner;

Task trBatteryTask(10000, TASK_FOREVER, &trBatteryTaskCb);
//Task trBeaconTask(3000, TASK_FOREVER, &trBeaconTaskCb);
Task trScreenUpdateTask(500, TASK_FOREVER, &trScreenUpdateTaskCb);

// rtos tasks
void otaTask(void *pvParameters);
void mqttTask(void *pvParameters);
void analogReadTask(void *pvParameters);
//void buttonTask(void *pvParameters);
void taskRunnerTask(void *pvParameters);

TaskHandle_t tOtaTask;
TaskHandle_t tMqttTask;
TaskHandle_t tAnalogReadTask;
//TaskHandle_t tButtonTask;
TaskHandle_t tTaskRunnerTask;
