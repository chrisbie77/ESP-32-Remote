#include <WiFi.h>
#include <PubSubClient.h>

char ssid[16];

const char *password = "csd1977jld1939ejd1938";

String strCurrentSSID;
String strThisIP;

IPAddress ip;
IPAddress local_IP(192, 168, 0, 40);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 0, 253); //optional
IPAddress secondaryDNS(192, 168, 0, 254); //optional

// mqtt setup
const char *clientId = "esp32remote1";
const char *mqttServer = "marvin.home";

const int msgSize = 64;
const int topicSize = 128;
const int mqttBufferSize = 4096;

char mqttInMessage[msgSize];
char mqttInTopic[topicSize];

bool newMqttInMsg = false;
bool newMqttOutMsg = false;

String strMqttTopic;
String strMqttMessage;
String strMqttOutTopic;
String strMqttOutMsg;

// Topics
const char *statusTopic = "/robots/remote1/status";
const char *controlTopic = "/robots/remote1/control";
const char *availabilityTopic = "/robots/remote1/availability";
const char *stick1Xtopic = "/robots/remote1/status/stick1x";
const char *stick1Ytopic = "/robots/remote1/status/stick1x";
const char *stick2Xtopic = "/robots/remote1/status/stick2x";
const char *stick2Ytopic = "/robots/remote1/status/stick2y";
const char *sticksTopic = "/robots/remote1/output/sticks";
const char *buttonsTopic = "/robots/remote1/output/buttons";
const char *motorTopic = "/robots/tank1/control/motors";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void mqttConnect();
void mqttCallback(char *topic, byte *payload, int length);
void myPublish(const char topic[topicSize], float message);
void myPublish(const char topic[topicSize], long int message);
void myPublish(const char topic[topicSize], int message);
void myPublish(const char topic[topicSize], const char message[msgSize]);
void myPublish(const char topic[topicSize], String message);
void myPublish(String topic, String message);
void myPublish(const char topic[topicSize], float message, bool retain);
void myPublish(const char topic[topicSize], long int message, bool retain);
void myPublish(const char topic[topicSize], int message, bool retain);
void myPublish(const char topic[topicSize], const char message[msgSize], bool retain);
void myPublish(const char topic[topicSize], String message, bool retain);
void myPublish(String topic, String message, bool retain);
void myPublish(const char message[msgSize]);
void myPublish(long int message);
void myPublish(int message);
void myPublish(String message);


void scan_wifi()
{
    strcpy(ssid, "hal35c");
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    int hal35_rssi = -99;
    int hal35c_rssi = -99;
    int hal35g_rssi = -99;
    Serial.println("scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            strCurrentSSID = WiFi.SSID(i);
            if (strCurrentSSID == "hal35")
            {
                hal35_rssi = WiFi.RSSI(i);
            }
            else if (strCurrentSSID == "hal35c")
            {
                hal35c_rssi = WiFi.RSSI(i);
            }
            else if (strCurrentSSID == "hal35g")
            {
                hal35g_rssi = WiFi.RSSI(i);
            }
        }
    }
    Serial.println("SSIDS:");
    Serial.print("hal35 -> ");
    Serial.println(hal35_rssi);
    Serial.print("hal35c -> ");
    Serial.println(hal35c_rssi);
    Serial.print("hal35g -> ");
    Serial.println(hal35g_rssi);
    Serial.println("Chose:");
    Serial.println(ssid);
    delay(10);
    if ((hal35_rssi > hal35c_rssi) && (hal35_rssi > hal35g_rssi))
    {
        strcpy(ssid, "hal35");
    }
    else if ((hal35c_rssi > hal35_rssi) && (hal35c_rssi > hal35g_rssi))
    {
        strcpy(ssid, "hal35c");
    }
    else if ((hal35g_rssi > hal35_rssi) && (hal35g_rssi > hal35_rssi))
    {
        strcpy(ssid, "hal35g");
    }
}

void setup_wifi()
{
    delay(10);
    WiFi.mode(WIFI_STA);
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA Failed to configure");
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(2000);
        Serial.println("Connecting...");
    }
    ip = WiFi.localIP();
    strThisIP = WiFi.localIP().toString();
    Serial.println(ip);
}

void mqttCallback(char *topic, byte *payload, int length)
{
    int topicLength = 0;
    topicLength = strlen(topic);
    for (int i = 0; i < topicLength; i++)
    {
        mqttInTopic[i] = topic[i];
    }
    mqttInTopic[topicLength] = '\0';
    strMqttTopic = mqttInTopic;
    for (int i = 0; i < length; i++)
    {
        mqttInMessage[i] = (char)payload[i];
    }
    mqttInMessage[length] = '\0';
    strMqttMessage = mqttInMessage;
    newMqttInMsg = true;
}

void mqttConnect()
{
    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(mqttBufferSize);
    if (mqttClient.connect(clientId, statusTopic, 1, true, "offline"))
    {
        myPublish("Remote 1 esp online from Reconnect");
        myPublish(strThisIP);
    }
    else
    {
        Serial.print("mqtt failed, rc=");
        Serial.println(mqttClient.state());
        delay(2000);
    }
}

void myPublish(const char topic[topicSize], float message)
{
    char msgChar[128];
    // dtostrf(message, 4, 2, msgChar);
    sprintf(msgChar, "%f", message);
    mqttClient.publish(topic, msgChar);
}

void myPublish(const char topic[topicSize], float message, bool retain)
{
    char msgChar[128];
    // dtostrf(message, 4, 2, msgChar);
    sprintf(msgChar, "%f", message);
    if (retain == true)
    {
        mqttClient.publish(topic, msgChar, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topic, "");
        mqttClient.publish(topic, msgChar);
    }
}

void myPublish(const char topic[topicSize], long int message)
{
    char msgChar[128];
    sprintf(msgChar, "%ld", message);
    mqttClient.publish(topic, msgChar);
}

void myPublish(const char topic[topicSize], long int message, bool retain)
{
    char msgChar[128];
    sprintf(msgChar, "%ld", message);
    if (retain == true)
    {
        mqttClient.publish(topic, msgChar, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topic, "");
        mqttClient.publish(topic, msgChar);
    }
}

void myPublish(const char topic[topicSize], int message)
{
    char msgChar[msgSize];
    // itoa(message, msgChar, 10);
    sprintf(msgChar, "%d", message);
    mqttClient.publish(topic, msgChar);
}

void myPublish(const char topic[topicSize], int message, bool retain)
{
    char msgChar[msgSize];
    sprintf(msgChar, "%d", message);
    if (retain == true)
    {
        mqttClient.publish(topic, msgChar, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topic, "");
        mqttClient.publish(topic, msgChar);
    }
}

void myPublish(const char topic[topicSize], const char message[msgSize])
{
    mqttClient.publish(topic, message);
}

void myPublish(const char topic[topicSize], const char message[msgSize], bool retain)
{
    if (retain == true)
    {
        mqttClient.publish(topic, message, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topic, "");
        mqttClient.publish(topic, message);
    }
}

void myPublish(const char topic[topicSize], String message)
{
    char msgChar[msgSize];
    message.toCharArray(msgChar, msgSize);
    mqttClient.publish(topic, msgChar);
}

void myPublish(const char topic[topicSize], String message, bool retain)
{
    char msgChar[msgSize];
    message.toCharArray(msgChar, msgSize);
    if (retain == true)
    {
        mqttClient.publish(topic, msgChar, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topic, "");
        mqttClient.publish(topic, msgChar);
    }
}

void myPublish(String topic, String message)
{
    char topicChar[topicSize];
    char msgChar[msgSize];
    message.toCharArray(msgChar, msgSize);
    topic.toCharArray(topicChar, topicSize);
    mqttClient.publish(topicChar, msgChar);
}

void myPublish(String topic, String message, bool retain)
{
    char topicChar[topicSize];
    char msgChar[msgSize];
    message.toCharArray(msgChar, msgSize);
    topic.toCharArray(topicChar, topicSize);
    if (retain == true)
    {
        mqttClient.publish(topicChar, msgChar, true);
    }
    else if (retain == false)
    {
        mqttClient.publish(topicChar, "");
        mqttClient.publish(topicChar, msgChar);
    }
}

void myPublish(const char message[msgSize])
{
    mqttClient.publish(statusTopic, message);
}

void myPublish(long int message)
{
    char msgChar[msgSize];
    sprintf(msgChar, "%ld", message);
    // itoa(message, msgChar, 10);
    mqttClient.publish(statusTopic, msgChar);
}

void myPublish(int message)
{
    char msgChar[msgSize];
    sprintf(msgChar, "%d", message);
    // itoa(message, msgChar, 10);
    mqttClient.publish(statusTopic, msgChar);
}

void myPublish(String message)
{
    char msgChar[msgSize];
    message.toCharArray(msgChar, msgSize);
    mqttClient.publish(statusTopic, msgChar);
}
