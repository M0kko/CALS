#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- НАСТРОЙКИ СЕТИ И MQTT ---
const char* ssid = "DIR-300NRU1";         // Ваша сеть WiFi
const char* password = "DIR-300NRU1"; // Ваш пароль WiFi
const char* mqtt_server = "m3.wqtt.ru";  // MQTT брокер
const int mqtt_port = 14317;
const char* mqtt_user = "u_DT7B0K";    // Логин от WQTT
const char* mqtt_pass = "r9fypHVG";   // Пароль от WQTT

// RELAY TOPICS
const String relay_topic   = "/topic/relay1";     // лампа 1
const String relay2_topic  = "/topic/relay2";     // лампа 2
const String relay3_topic  = "/topic/relay3";     // лампа 3
const String bright1_topic  = "/topic/brightness1"; // ЯРКОСТЬ 0..100
const String bright2_topic  = "/topic/brightness2"; // ЯРКОСТЬ 0..100
const String bright3_topic  = "/topic/brightness3"; // ЯРКОСТЬ 0..100

// Пины (GPIO)
const int RELAY  = 14;
const int RELAY2 = 15;
const int RELAY3 = 16;

/////////////////////////////////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);

bool relay_on  = false;
bool relay2_on = false;
bool relay3_on = false;

// Яркость в процентах и в значении PWM
int brightnessPercent = 100;   // 0..100
int brightnessPwm     = 1023;  // 0..1023 для ESP8266

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void updateStatePins(void) {
  // Если это реально светодиоды — используем PWM:
  analogWrite(RELAY,  relay_on  ? brightnessPwm : 0);
  analogWrite(RELAY2, relay2_on ? brightnessPwm : 0);
  analogWrite(RELAY3, relay3_on ? brightnessPwm : 0);
  
  // Если у тебя механические реле – тут нужно оставить digitalWrite,
  // а яркость реализовывать по-другому (через отдельный транзистор/ШИМ-канал).
}

void callback(char* topic, byte* payload, unsigned int length) {
  String data_pay;
  for (unsigned int i = 0; i < length; i++) {
    data_pay += (char)payload[i];
  }

  String topicStr = String(topic);

  Serial.print("Topic: ");
  Serial.print(topicStr);
  Serial.print(" | Payload: ");
  Serial.println(data_pay);

  // ----- Управление реле 1 -----
  if (topicStr == relay_topic) {
    if (data_pay == "ON"  || data_pay == "1") relay_on  = true;
    if (data_pay == "OFF" || data_pay == "0") relay_on  = false;
  }

  // ----- Управление реле 2 -----
  if (topicStr == relay2_topic) {
    if (data_pay == "ON"  || data_pay == "1") relay2_on = true;
    if (data_pay == "OFF" || data_pay == "0") relay2_on = false;
  }

  // ----- Управление реле 3 -----
  if (topicStr == relay3_topic) {
    if (data_pay == "ON"  || data_pay == "1") relay3_on = true;
    if (data_pay == "OFF" || data_pay == "0") relay3_on = false;
  }

  // ----- Управление яркостью -----
  if (topicStr == bright1_topic) {
    int value = data_pay.toInt();  // ожидаем 0..100
    if (value < 0)   value = 0;
    if (value > 100) value = 100;

    brightnessPercent = value;
    brightnessPwm = map(brightnessPercent, 0, 100, 0, 1023);

    Serial.print("New brightness 1: ");
    Serial.print(brightnessPercent);
    Serial.print("% (PWM=");
    Serial.print(brightnessPwm);
    Serial.println(")");
  }

  if (topicStr == bright2_topiс) {
    int value = data_pay.toInt();  // ожидаем 0..100
    if (value < 0)   value = 0;
    if (value > 100) value = 100;

    brightnessPercent = value;
    brightnessPwm = map(brightnessPercent, 0, 100, 0, 1023);

    Serial.print("New brightness 2: ");
    Serial.print(brightnessPercent);
    Serial.print("% (PWM=");
    Serial.print(brightnessPwm);
    Serial.println(")");
  }

  if (topicStr == bright3_topic) {
    int value = data_pay.toInt();  // ожидаем 0..100
    if (value < 0)   value = 0;
    if (value > 100) value = 100;

    brightnessPercent = value;
    brightnessPwm = map(brightnessPercent, 0, 100, 0, 1023);

    Serial.print("New brightness 3: ");
    Serial.print(brightnessPercent);
    Serial.print("% (PWM=");
    Serial.print(brightnessPwm);
    Serial.println(")");
  }

  updateStatePins();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      
      // Подписка на 3 реле
      client.subscribe(relay_topic.c_str());    // /topic/relay1
      client.subscribe(relay2_topic.c_str());   // /topic/relay2
      client.subscribe(relay3_topic.c_str());   // /topic/relay3

      // Подписка на яркость
      client.subscribe(bright1_topic.c_str());   // /topic/brightness1
      client.subscribe(bright2_topic.c_str());   // /topic/brightness2
      client.subscribe(bright3_topic.c_str());   // /topic/brightness3

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(RELAY,  OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  digitalWrite(RELAY,  LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);

  Serial.begin(115200);
  setup_wifi();

  // Настраиваем диапазон PWM 0..1023
  analogWriteRange(1023);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}