#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ==========================================
// 1. DATELE TALE: WIFI ȘI GROQ
// ==========================================
const char* ssid = "-";
const char* password = "-";
const char* API_KEY = "-";

// ==========================================
// 2. PINI OFICIALI TOUCH (ESP32-2432S028)
// ==========================================
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// ==========================================
// 3. VARIABILE INTERFAȚĂ
// ==========================================
String inputText = "";
String chatHistory = "Sebi AI PRO conectat! 🔥";
unsigned long lastTouchTime = 0;
const int touchDelay = 250; 

// Layout Tastatură QWERTY
const char* row0[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
const char* row1[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
const char* row2[] = {"Z", "X", "C", "V", "B", "N", "M", "<"};

// ==========================================
// 4. FUNCȚII GRAFICE (DESENARE)
// ==========================================
void drawButton(int x, int y, int w, int h, uint16_t color, String label) {
  tft.fillRoundRect(x, y, w, h, 4, color);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString(label, x + w / 2, y + h / 2 - 8, 2);
}

void drawKeyboard() {
  tft.fillRect(0, 120, 320, 120, TFT_BLACK); 
  
  for (int i = 0; i < 10; i++) drawButton(i * 32, 120, 30, 28, 0x2104, row0[i]); // Gri inchis
  for (int i = 0; i < 9; i++) drawButton(16 + i * 32, 150, 30, 28, 0x2104, row1[i]);
  for (int i = 0; i < 7; i++) drawButton(32 + i * 32, 180, 30, 28, 0x2104, row2[i]);
  
  drawButton(32 + 7 * 32, 180, 62, 28, TFT_RED, "<"); 
  drawButton(0, 210, 200, 28, 0x2B6C, "SPATIU"); // Albastru Pro
  drawButton(210, 210, 110, 28, 0x07E0, "TRIMITE"); // Verde Pro
}

void updateChatScreen() {
  tft.fillRect(0, 0, 320, 120, 0x0B0B); // Fundal #0b0b0f
  tft.setTextColor(0x07E0); // Culoare text #00ff99
  tft.setCursor(5, 5, 2);
  
  int cursorY = 5;
  String temp = chatHistory;
  while(temp.length() > 0 && cursorY < 90) {
    String line = temp.substring(0, 40); 
    tft.drawString(line, 5, cursorY, 2);
    temp.remove(0, 40);
    cursorY += 15;
  }

  // Caseta de text
  tft.fillRect(0, 100, 320, 20, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.drawString(inputText + "_", 5, 102, 2);
}

// ==========================================
// 5. APELARE API GROQ (llama-3.1-8b-instant)
// ==========================================
String callGroqAI(String msg) {
  WiFiClientSecure client;
  client.setInsecure(); // Ignoră certificatele pentru a funcționa direct
  HTTPClient http;
  
  http.begin(client, "https://api.groq.com/openai/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(API_KEY));

  // Creare payload JSON compatibil direct cu formatul Groq
  StaticJsonDocument<1024> doc;
  doc["model"] = "llama-3.1-8b-instant";
  doc["max_tokens"] = 150; // Menținem răspunsurile scurte pentru ecranul plăcii
  
  JsonArray messages = doc.createNestedArray("messages");
  
  JsonObject sysMsg = messages.createNestedObject();
  sysMsg["role"] = "system";
  sysMsg["content"] = "Ești Sebi AI PRO. Răspunzi natural, prietenos și scurt în română.";

  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";
  userMsg["content"] = msg;

  String body;
  serializeJson(doc, body);

  int code = http.POST(body);
  if (code > 0) {
    String res = http.getString();
    http.end();
    client.stop(); // Curățăm conexiunea pentru stabilitate

    DynamicJsonDocument respDoc(4096);
    DeserializationError error = deserializeJson(respDoc, res);
    if (!error) {
       return respDoc["choices"][0]["message"]["content"].as<String>();
    } else {
       return "Eroare parsare AI!";
    }
  }
  
  http.end();
  client.stop();
  return "Eroare Server: " + String(code);
}

// ==========================================
// 6. PROCESARE LOGICĂ TASTE
// ==========================================
void processTouch(int x, int y) {
  if (millis() - lastTouchTime < touchDelay) return;
  lastTouchTime = millis();

  if (y >= 120 && y < 148) { 
    int col = x / 32;
    if (col < 10) inputText += row0[col];
  } 
  else if (y >= 150 && y < 178) { 
    int col = (x - 16) / 32;
    if (col >= 0 && col < 9) inputText += row1[col];
  } 
  else if (y >= 180 && y < 208) { 
    if (x >= 32 + 7 * 32) { 
      if (inputText.length() > 0) inputText.remove(inputText.length() - 1);
    } else {
      int col = (x - 32) / 32;
      if (col >= 0 && col < 7) inputText += row2[col];
    }
  } 
  else if (y >= 210 && y < 238) { 
    if (x < 200) {
      inputText += " "; 
    } else {
      if (inputText.length() > 0) {
        chatHistory = "Tu: " + inputText;
        updateChatScreen();
        
        chatHistory = "🤖 Sebi AI thinking...";
        updateChatScreen();
        
        String raspuns = callGroqAI(inputText);
        chatHistory = "AI: " + raspuns;
        inputText = ""; 
      }
    }
  }
  updateChatScreen();
}

// ==========================================
// 7. SETUP ȘI CALIBRARE OFICIALĂ
// ==========================================
void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1); // Orientare Landscape
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1); // Sincronizare cu ecranul

  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Conectare la WiFi...", 160, 110, 2);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  drawKeyboard();
  updateChatScreen();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    
    // ========================================================
    // 🔥 FIX OFICIAL CALIBRARE TOUCH (Mapare Extremități)
    // Majoritatea display-urilor CYD au limite între ~200 și ~3700.
    // ========================================================
    
    // Configurația STANDARD:
    int touchX = map(p.x, 200, 3700, 0, 320); 
    int touchY = map(p.y, 240, 3800, 0, 240); 

    // Dacă atingi dreapta și scrie în stânga, inversează X-ul comentând liniile de sus și folosindu-le pe acestea:
    // int touchX = map(p.x, 3700, 200, 0, 320); 
    // int touchY = map(p.y, 3800, 240, 0, 240);
    
    // Asigurare împotriva ieșirii din cadru
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    processTouch(touchX, touchY);
  }
}