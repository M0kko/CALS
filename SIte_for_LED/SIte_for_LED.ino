#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ==========================================
// 1. Настройки Wi-Fi
// ==========================================
const char* ssid = "Huawei_ROSTELECOM_9A39";       // Введите имя вашей сети
const char* password = "9A9F569E"; // Введите пароль вашей сети

// ==========================================
// 2. Определение пинов
// ==========================================
// У ESP8266 только один аналоговый вход - A0
const int LDR_PIN = A0;   

// Пины для светодиодов (используем маркировку NodeMCU/Wemos)
// D1 (GPIO 5), D2 (GPIO 4), D3 (GPIO 0)
// Если IDE ругается на D1, замените на цифры: 5, 4, 0
#ifndef D1
  #define D1 5
  #define D2 4
  #define D3 0
#endif

const int LED1_PIN = D1;  
const int LED2_PIN = D2;  
const int LED3_PIN = D3;  

// ==========================================
// 3. Глобальные переменные и объекты
// ==========================================
ESP8266WebServer server(80); // Используем класс ESP8266WebServer

// Хранение состояния светодиодов
bool ledStates[3] = {LOW, LOW, LOW};

// ==========================================
// 4. Вспомогательные функции
// ==========================================

int readLuxValue() {
  // У ESP8266 АЦП 10-битный: значения от 0 до 1023
  return analogRead(LDR_PIN);
}

void updateLedOutputs() {
  digitalWrite(LED1_PIN, ledStates[0]);
  digitalWrite(LED2_PIN, ledStates[1]);
  digitalWrite(LED3_PIN, ledStates[2]);
}

String createHtmlPage() {
  int lux = readLuxValue();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP8266 Web Control</title>";
  
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; background-color: #f4f4f4; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += ".led-status { margin: 10px; padding: 10px; border: 2px solid #ccc; display: inline-block; border-radius: 8px; width: 150px;}";
  html += "button { padding: 10px 20px; font-size: 16px; margin: 5px; cursor: pointer; border: none; border-radius: 5px; color: white;}";
  html += ".btn-on { background-color: #28a745; }";
  html += ".btn-off { background-color: #dc3545; }";
  html += "</style>";
  
  html += "</head><body><div class='container'>";
  
  html += "<h1>Лабораторная работа (ESP8266)</h1>";
  html += "<h2>Данные с датчика (LDR)</h2>";
  // Обновленный текст про диапазон значений
  html += "<p style='font-size: 1.5em;'>Значение АЦП (0-1023): <b>" + String(lux) + "</b></p>";
  html += "<hr>";

  html += "<h2>Управление светодиодами</h2>";
  
  for (int i = 0; i < 3; i++) {
    String status = (ledStates[i] == HIGH) ? "ВКЛ" : "ВЫКЛ";
    String color = (ledStates[i] == HIGH) ? "#28a745" : "#dc3545"; 
    
    html += "<div class='led-status' style='border-color:" + color + ";'>";
    html += "<h3>LED " + String(i + 1) + "</h3>";
    html += "<p>Статус: <span style='color:" + color + "; font-weight:bold;'>" + status + "</span></p>";
    
    html += "<a href='/L" + String(i+1) + "_ON'><button class='btn-on'>ВКЛ</button></a><br>";
    html += "<a href='/L" + String(i+1) + "_OFF'><button class='btn-off'>ВЫКЛ</button></a>";
    html += "</div>";
  }
  
  html += "</div></body></html>";
  return html;
}

// ==========================================
// 5. Обработчики маршрутов
// ==========================================

void handleRoot() {
  server.send(200, "text/html", createHtmlPage());
}

void setLed(int index, bool state) {
  ledStates[index] = state;
  updateLedOutputs();
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLed1On()  { setLed(0, HIGH); }
void handleLed1Off() { setLed(0, LOW);  }
void handleLed2On()  { setLed(1, HIGH); }
void handleLed2Off() { setLed(1, LOW);  }
void handleLed3On()  { setLed(2, HIGH); }
void handleLed3Off() { setLed(2, LOW);  }

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// ==========================================
// 6. Setup
// ==========================================
void setup() {
  // Настройка пинов
  // Для ESP8266 pinMode для A0 обычно не требуется указывать явно, но не повредит
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  
  updateLedOutputs();

  Serial.begin(115200);
  delay(10);
  Serial.println("\nStarting ESP8266 Web Server...");

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Привязка маршрутов
  server.on("/", handleRoot);
  
  server.on("/L1_ON", handleLed1On);
  server.on("/L1_OFF", handleLed1Off);
  server.on("/L2_ON", handleLed2On);
  server.on("/L2_OFF", handleLed2Off);
  server.on("/L3_ON", handleLed3On);
  server.on("/L3_OFF", handleLed3Off);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

// ==========================================
// 7. Loop
// ==========================================
void loop() {
  server.handleClient();
  delay(2);
}