#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

// Constants
#define LED_PIN D4
#define BUTTON_PIN 0
#define EEPROM_SIZE 512

// Wi-Fi AP credentials for the configuration mode
const char *ap_ssid = "ESP-391F35";
const char *ap_password = "1234";

// Web server
ESP8266WebServer server(80);

// Variables for storing configuration
char ssid[32];
char password[32];
char deviceID[32];
bool ledStatus;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Read configuration from EEPROM
  if (!readConfig()) {
    // Start in AP mode if no valid configuration found
    startAPMode();
  } else {
    // Connect to Wi-Fi and restore LED status
    connectToWiFi();
    digitalWrite(LED_PIN, ledStatus ? HIGH : LOW);
  }
}

void loop() {
  // Handle web server
  server.handleClient();

  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Debounce delay
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("------------------------------------------------------------");
      Serial.println("Button pressed. Restarting in AP mode...");
      startAPMode();
      while (digitalRead(BUTTON_PIN) == LOW);  // Wait for button release
      Serial.println("------------------------------------------------------------");
    }
  }
}

// Function to read configuration from EEPROM
bool readConfig() {
  EEPROM.get(0, ssid);
  EEPROM.get(32, password);
  EEPROM.get(64, deviceID);
  EEPROM.get(96, ledStatus);

  Serial.println("------------------------------------------------------------");
  // Print read values for debugging
  Serial.println("Reading configuration from EEPROM...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("Device ID: ");
  Serial.println(deviceID);
  Serial.print("LED Status: ");
  Serial.println(ledStatus ? "ON" : "OFF");
  Serial.println("------------------------------------------------------------");

  // Validate SSID
  if (strlen(ssid) == 0) {
    return false;
  }
  return true;
}

// Function to save configuration to EEPROM
void saveConfig() {
  Serial.println("------------------------------------------------------------");
  Serial.println("Saving configuration to EEPROM...");
  EEPROM.put(0, ssid);
  EEPROM.put(32, password);
  EEPROM.put(64, deviceID);
  EEPROM.put(96, ledStatus);
  EEPROM.commit();
  
  // Print saved values for debugging
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("Device ID: ");
  Serial.println(deviceID);
  Serial.print("LED Status: ");
  Serial.println(ledStatus ? "ON" : "OFF");
  Serial.println("------------------------------------------------------------");
}

// Function to start the ESP8266 in Access Point mode
void startAPMode() {
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("------------------------------------------------------------");
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("------------------------------------------------------------");

  // Set up web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  Serial.println("HTTP server started");
}

// Function to handle the root URL ("/") for the web server
void handleRoot() {
  String html = "<div style=\"display: flex; justify-content: center; align-items: center; height: 100vh; background-color: #3498db;\">";
  html += "<div style=\"background-color: #fff; padding: 20px; border-radius: 10px;\">";
  html += "<h1 style=\"text-align: center; color: #3498db;\">ESP8266 with EEPROM</h1>"; // Title added
  html += "<form action=\"/save\" method=\"POST\">";
  html += "<label for=\"ssid\">SSID:</label><br><input type=\"text\" id=\"ssid\" name=\"ssid\" style=\"width: calc(100% - 20px); margin-bottom: 10px; padding: 5px;\"><br>"; // Proper alignment and styling for input boxes
  html += "<label for=\"password\">Password:</label><br><input type=\"password\" id=\"password\" name=\"password\" style=\"width: calc(100% - 20px); margin-bottom: 10px; padding: 5px;\"><br>"; // Password type for password input
  html += "<label for=\"deviceID\">Device ID:</label><br><input type=\"text\" id=\"deviceID\" name=\"deviceID\" style=\"width: calc(100% - 20px); margin-bottom: 10px; padding: 5px;\"><br>"; // Proper alignment and styling for input boxes
  html += "<label for=\"ledStatus\">LED Status:</label><br>"; // Label added for LED Status
  html += "<div style=\"display: flex; align-items: center;\">"; // Flexbox for button and circle
  html += "<button type=\"button\" id=\"toggleButton\" onclick=\"toggleLED()\" style=\"background-color: #FF5733; color: #fff; padding: 10px; border: 2px solid #ccc; border-radius: 20px; width: 50px; margin-right: 10px; position: relative; overflow: hidden;\">"; // Toggle button with oval design
  html += "<span id=\"toggleCircle\" style=\"background-color: #fff; border: 2px solid #ccc; border-radius: 50%; width: 20px; height: 20px; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);\"></span>"; // Circle inside the button
  html += "</button>"; 
  html += "</div><br>"; 
  html += "<input type=\"hidden\" id=\"ledStatus\" name=\"ledStatus\" value=\"off\">"; // Hidden input field to store LED status
  html += "<input type=\"submit\" value=\"submit\" style=\"width: 100%; background-color: #3BDC6B; color: #fff; padding: 10px; border: none; border-radius: 5px; margin-top: 10px;\">"; // Proper alignment and styling for submit button
  html += "</form>";
  html += "</div></div>";

  // JavaScript for toggle button functionality
  html += "<script>";
  html += "function toggleLED() {";
  html += "var button = document.getElementById('toggleButton');";
  html += "var circle = document.getElementById('toggleCircle');";
  html += "var hiddenInput = document.getElementById('ledStatus');";
  html += "if (hiddenInput.value === 'off') {";
  html += "hiddenInput.value = 'on';";
  html += "button.style.backgroundColor = '#3BDC6B';";
  html += "circle.style.width = '25px';"; 
  html += "circle.style.height = '25px';"; 
  html += "} else {";
  html += "hiddenInput.value = 'off';";
  html += "button.style.backgroundColor = '#FF5733';";
  html += "circle.style.width = '20px';"; 
  html += "circle.style.height = '20px';"; 
  html += "}";
  html += "}";
  html += "</script>";

  server.send(200, "text/html", html);
}

// Function to handle the save URL ("/save") for the web server
void handleSave() {
  String ssidInput = server.arg("ssid");
  String passwordInput = server.arg("password");
  String deviceIDInput = server.arg("deviceID");
  String ledStatusStr = server.arg("ledStatus");

  if (ssidInput.length() > 0) {
    ssidInput.toCharArray(ssid, sizeof(ssid));
    passwordInput.toCharArray(password, sizeof(password));
    deviceIDInput.toCharArray(deviceID, sizeof(deviceID));
    ledStatus = (ledStatusStr == "on");

    saveConfig();
    server.send(200, "text/html", "Configuration saved. Rebooting...");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Invalid input");
  }
}

// Function to connect to Wi-Fi using the stored credentials
void connectToWiFi() {
  Serial.println("------------------------------------------------------------");
  if (strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssid);
  }

//COnnecting to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

//Wifi connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //Print the Wi-Fi IP
  } else {
    Serial.println("Failed to connect. Restarting in AP mode."); //Fail to connect to Wi-Fi
    startAPMode();//Back to Access Point mode
  }
  Serial.println("------------------------------------------------------------");
}
