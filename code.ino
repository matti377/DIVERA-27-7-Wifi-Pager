#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <Waveshare_LCD1602_RGB.h>
#include <ESP8266WebServer.h>

struct Emergency {
  char id[32];
  char title[32];
  char address[32];
  String text;
  bool active;
};

//define Screen
Waveshare_LCD1602_RGB lcd(16, 2);
WiFiClientSecure client;

// WiFi credentials
//const char* ssid = "404 Wi-Fi Not Found";
//const char* password = "-Garona-2023";

const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

// Define BUZZER and BUTTON pins
const int BUZZER = 14;
const int BUTTON = 12;

unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 20000;

// EEPROM addresses and emergency struct
const int emergencyIndexAddr = 0;
const int emergencyDataAddr = sizeof(int);
String lastId = "0";

// Create a web server on port 80
ESP8266WebServer server(80);

// Function Prototypes
void startupSound();
void emergencySound();
void connectionError();
void wifiDisconnectedAlert();
void storeEmergency(const String& id, const String& title, const String& address, const String& text);
void displayEmergency(const String& title, const String& address);

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GIPS Arts et Métiers Web Interface</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f7f7f7;
            margin: 0;
            padding: 0;
        }
        header {
            background: #0056b3;
            color: white;
            padding: 20px;
            text-align: center;
        }
        header h1 {
            margin: 0;
            font-size: 2rem;
        }
        main {
            padding: 20px;
            max-width: 800px;
            margin: auto;
        }
        h2 {
            color: #333;
            font-size: 1.5rem;
        }
        .emergency {
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            margin-bottom: 20px;
            padding: 15px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
        }
        .emergency h3 {
            margin: 0 0 10px;
            font-size: 1.2rem;
            color: #0056b3;
        }
        .emergency p {
            margin: 0;
            color: #555;
        }
        .divider {
            border-top: 1px solid #ccc;
            margin: 10px 0;
        }
        footer {
            text-align: center;
            padding: 10px;
            background: #ddd;
            font-size: 0.9rem;
        }
    </style>
</head>
<body>
    <header>
        <h1>GIPS Arts et Métiers Web Interface</h1>
    </header>
    <main>
        <h2>Recent Emergencies</h2>
)rawliteral";

  int emergencyIndex = 0;
  EEPROM.get(emergencyIndexAddr, emergencyIndex);
  if (emergencyIndex < 0 || emergencyIndex > 10) {
    emergencyIndex = 0;  // If invalid, reset to 0 (no emergencies)
  }

  for (int i = 0; i < emergencyIndex; i++) {
    Emergency emergency;
    EEPROM.get(emergencyDataAddr + i * sizeof(Emergency), emergency);

    if (emergency.active) {
      html += "<div class=\"emergency\">";
      html += "<h3>" + String(emergency.title) + "</h3>";

      String formattedText = "text";
      formattedText.replace("\r\n", "<br>");
      formattedText.replace("–––––––––––––––––––––––––", "<div class=\"divider\"></div>");
      html += "<p>" + formattedText + "</p>";
      html += "</div>";
    }
  }

  html += R"rawliteral(
    </main>
    <footer>
        &copy; 2024 GIPS Arts et Métiers
    </footer>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  pinMode(BUTTON, INPUT);

  lcd.init();
  lcd.setCursor(0, 0);
  lcd.setRGB(255, 0, 0);
  lcd.send_string("Starting");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting...");
  }
  Serial.println("Connected");

  lcd.clear();
  lcd.setRGB(0, 255, 0);
  lcd.send_string(WiFi.localIP().toString().c_str());
  Serial.print(WiFi.localIP().toString());
  startupSound();
  delay(5000);

  EEPROM.begin(512);
  client.setInsecure();

  // Define the route for the root URL
  server.on("/", handleRoot);

  // Start the server
  server.begin();
  Serial.println("Web server started!");
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();

    if (millis() - lastCheckTime >= checkInterval) {
      lastCheckTime = millis();
      fetchAlarmData();
    }
  } else {
    wifiDisconnectedAlert();
  }
}
void fetchAlarmData() {
  HTTPClient http;
  String url = "https://app.divera247.com/api/last-alarm?accesskey={your-api-key}";
  http.begin(client, url);

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String response = http.getString();
    StaticJsonDocument<4096> doc;

    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print(F("Deserialization failed: "));
      Serial.println(error.f_str());
      return;
    }

    if (!doc["success"].isNull() && doc["success"]) {
      JsonVariant data = doc["data"];
      if (!data.isNull()) {
        // Access elements safely using | operator to avoid invalid type conversions
        String id = data["id"] | "";
        String title = data["title"] | "";
        String address = data["address"] | "";
        String text = data["text"] | "";

        if (!lastId.equals(id)) {
          displayEmergency(title, address);
          storeEmergency(id, title, address, text);
          emergencySound();
          lastId = id;
        } else {
          Serial.println("No new emergency to process.");
        }
      }
    } else {
      Serial.println("API returned no data or false success.");
      lcd.setRGB(0, 0, 0);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.send_string("YOURTEXT");
    }
  } else {
    connectionError();
  }
  http.end();
}
void displayEmergency(const String& title, const String& address) {
  lcd.clear();
  if (title.equals("Trauma / Sturz")) {
    lcd.setRGB(50, 117, 168);
  } else if (title.equals("Brandmeldeanlage")) {
    lcd.setRGB(168, 92, 50);
  } else if (title.equals("Atemstörung") || title.equals("Aspiration / Bolusgeschehen") || title.equals("Reanimation") || title.equals("Bewusstlosigkeit")) {
    lcd.setRGB(255, 0, 0);
  } else {
    lcd.setRGB(168, 50, 103);
  }
  lcd.setCursor(0, 0);
  lcd.send_string(title.substring(0, 16).c_str());
  lcd.setCursor(0, 1);
  lcd.send_string(("Addr: " + address).substring(0, 16).c_str());
}
void storeEmergency(const String& id, const String& title, const String& address, const String& text) {
  int currentEmergencyIndex;
  EEPROM.get(emergencyIndexAddr, currentEmergencyIndex);

  if (currentEmergencyIndex < 0 || currentEmergencyIndex >= 10) {
    currentEmergencyIndex = 0;
  }

  Emergency emergency;
  id.toCharArray(emergency.id, sizeof(emergency.id));
  title.toCharArray(emergency.title, sizeof(emergency.title));
  address.toCharArray(emergency.address, sizeof(emergency.address));
  emergency.text = "text";
  emergency.active = true;

  // Write emergency data to EEPROM
  EEPROM.put(emergencyDataAddr + currentEmergencyIndex * sizeof(Emergency), emergency);

  // Update and save index
  currentEmergencyIndex++;
  EEPROM.put(emergencyIndexAddr, currentEmergencyIndex);

  EEPROM.commit();
}
void connectionError() {
  lcd.clear();
  lcd.setRGB(255, 0, 0);
  lcd.setCursor(0, 0);
  lcd.send_string("Connection failed");
  tone(BUZZER, 1000);
  delay(500);
  noTone(BUZZER);
}
void wifiDisconnectedAlert() {
  lcd.clear();
  lcd.setRGB(255, 0, 0);
  lcd.setCursor(0, 0);
  lcd.send_string("Wifi Disconnected");
  tone(BUZZER, 1200);
  delay(150);
  noTone(BUZZER);
  delay(40000);
}
void startupSound() {
  int melody[] = { 523, 659, 784, 880, 784, 659, 523 };
  int noteDurations[] = { 200, 200, 200, 200, 200, 200, 400 };
  for (int i = 0; i < 7; i++) {
    tone(BUZZER, melody[i]);
    delay(noteDurations[i]);
    noTone(BUZZER);
    delay(50);
  }
}
void emergencySound() {
  int buttonPressed = 0;
  while (buttonPressed == 0) {
    buttonPressed = digitalRead(BUTTON);
    tone(BUZZER, 1200);
    delay(150);
    noTone(BUZZER);
    delay(100);
    buttonPressed = digitalRead(BUTTON);
  }
}