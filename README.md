# DIVERA-27-7-Wifi-Pager

## Overview

This project allows you to build your own WiFi pager that integrates with the DIVERA API, displaying emergency alerts on an LCD screen. The pager is customizable, enabling you to change the screen color based on the emergency type.

### Components Used:
- **Waveshare 1602 LCD Display**: 16x2 RGB LCD screen for displaying emergency messages.
- **Wemos D1 Mini (ESP8266)**: A small microcontroller with WiFi capability for connecting to the DIVERA API.
- **EEMB Lithium Polymer Battery (3.7V)**: Rechargeable battery to power the device.
- **Aideepen 6-Pack Type-C USB-C TC4056 Li-Ion Battery Charging Board (5V, 1A)**: A charging board to charge the battery.
- **Push Button**: For user interaction, e.g., to acknowledge an alert.

## Features:
- **WiFi Connectivity**: Receives emergency notifications via the DIVERA API.
- **Customizable LCD Screen**: Change the screen color based on the type of emergency.
- **Rechargeable Battery**: Powered by a 3.7V Li-Po battery for portability.
- **Buzzer**: Alerts the user with sound notifications.

---

## Hardware Setup

### Wiring the Components:
1. **Wemos D1 Mini (ESP8266)**:
   - Connect the **GND** pin to the **GND** rail of your breadboard.
   - Connect the **3V3** pin to the **VCC** of the LCD and the battery.
   - Connect the **D1 (GPIO5)** pin to the **SDA** pin of the LCD.
   - Connect the **D2 (GPIO4)** pin to the **SCL** pin of the LCD.
   - Connect the **D6 (GPIO12)** pin to the **Button** pin (through a pull-down resistor).

2. **Waveshare 1602 RGB LCD**:
   - Connect **VCC** to **3.3V** on the Wemos D1 Mini.
   - Connect **GND** to **GND** on the Wemos D1 Mini.
   - Connect **SDA** to **D1 (GPIO5)**.
   - Connect **SCL** to **D2 (GPIO4)**.
   - Connect **RGB pins** to the Wemos D1 Mini's **D7, D8, and D5** pins (for Red, Green, and Blue control).

3. **Battery and Charging Module**:
   - Connect **B+** and **B-** from the charging board to the **3V3** and **GND** of the battery.
   - The **OUT+** and **OUT-** pins of the charging board should be connected to the **VCC** and **GND** of the LCD.

4. **Buzzer**:
   - Connect one leg to **D6 (GPIO12)** and the other leg to **GND**.

---

## Software Setup

### Prerequisites:
1. **Arduino IDE**: Install the [Arduino IDE](https://www.arduino.cc/en/software) if you haven't already.
2. **ESP8266 Board Package**: Add the ESP8266 board to your Arduino IDE.
   - Go to **File > Preferences**, and in the **Additional Board Manager URLs** field, add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Then go to **Tools > Board > Boards Manager**, search for **ESP8266**, and install it.
3. **Libraries**:
   - Install the following libraries via **Sketch > Include Library > Manage Libraries**:
     - **ESP8266WiFi**
     - **WiFiClientSecure**
     - **ArduinoJson**
     - **ESP8266HTTPClient**
     - **EEPROM**
     - **Waveshare_LCD1602_RGB**
     - **ESP8266WebServer**

### Setting Up the Code:
1. **WiFi Credentials**:
   In the code, replace the following placeholders with your WiFi credentials:
   ```cpp
   const char* ssid = "YOURSSID";
   const char* password = "YOURPASSWORD";
   ```

2. **API Key**:
   To fetch emergency data, replace `{your-api-key}` in the `fetchAlarmData()` function with your valid API key from the DIVERA service.

3. **Upload the Code**:
   - Select the **Wemos D1 Mini** board under **Tools > Board**.
   - Select the appropriate **COM port** under **Tools > Port**.
   - Upload the code to your Wemos D1 Mini.

---

## How It Works

1. **Start-Up**:
   - Upon powering up, the device connects to the specified WiFi network.
   - It displays the IP address of the device on the LCD screen.
   - The LCD screen will show a "Starting" message, followed by a short startup sound.

2. **Fetching Emergency Data**:
   - The device periodically queries the DIVERA API for the latest emergency alerts.
   - If a new emergency is detected, it updates the LCD screen with the emergency's title, address, and associated color.

3. **Emergency Display**:
   - The screen color changes based on the type of emergency (e.g., red for trauma, yellow for fire, etc.).
   - The buzzer sounds to alert the user of an emergency.

4. **Web Interface**:
   - A simple web server is built into the device. It serves a webpage showing recent emergencies.
   - Access it by navigating to the device’s IP address in a web browser.

5. **Button Interaction**:
   - The button can be used to acknowledge an alert, silencing the buzzer and stopping the sound.

---

## Functions Breakdown

- **`setup()`**: Initializes the LCD, connects to WiFi, starts the web server, and begins periodic checks for new emergency data.
- **`loop()`**: Handles incoming web requests, checks for new emergency data every 20 seconds, and processes the button press for acknowledging an emergency.
- **`fetchAlarmData()`**: Fetches emergency data from the DIVERA API and updates the display if there's a new emergency.
- **`displayEmergency()`**: Displays the emergency title and address on the LCD screen and changes the screen color accordingly.
- **`storeEmergency()`**: Stores the emergency details in the EEPROM for later use.
- **`connectionError()`**: Displays a message on the LCD when the WiFi connection fails.
- **`wifiDisconnectedAlert()`**: Alerts the user when the device disconnects from WiFi.
- **`startupSound()`**: Plays a melody when the device is powered on.
- **`emergencySound()`**: Plays an emergency sound when a new alert is detected and waits for the user to acknowledge.

---

## Troubleshooting

- **WiFi Connection Issues**:
  - Make sure the WiFi credentials are correct and the device is within range of the router.
  
- **LCD Display**:
  - Ensure the connections between the Wemos D1 Mini and the LCD are correct.
  - If the display shows garbage characters, check the wiring and ensure the `SDA` and `SCL` pins are correctly connected.

- **API Data**:
  - If no data is fetched, ensure your API key is correct and that the DIVERA service is available.
  - 
  Here's a credit section for your README, including references to relevant sources:

---

## Credits

This project uses several open-source libraries and resources to integrate with external services and hardware components:

- **API Documentation**:  
  The API for fetching emergency data is provided by [Divera247](https://api.divera247.com/#/Einsatzinformationen%20abrufen/get_api_last_alarm).

- **WiFi Setup**:  
  - The connection to the WiFi router is based on the tutorial from [Instructables: IoT ESP8266 Series - Connect to WiFi Router](https://www.instructables.com/IoT-ESP8266-Series-1-Connect-to-WIFI-Router/).
  - For ESP32 Web Server configuration, reference [ESP32IO Tutorial: ESP32 Web Server](https://esp32io.com/tutorials/esp32-web-server).

- **HTTP GET Requests**:  
  - The HTTP requests in the project are handled using the [ArduinoJson HTTPClient guide](https://arduinojson.org/v6/how-to/use-arduinojson-with-httpclient/).
  - For detailed instructions on how to make HTTP requests in Arduino, refer to [Arduino Get Started: HTTP Requests](https://arduinogetstarted.com/tutorials/arduino-http-request).

- **LCD Display**:  
  The display functionality is based on the [Waveshare LCD1602 RGB Module documentation](https://www.waveshare.com/wiki/LCD1602_RGB_Module).

- **Buzzer Integration**:  
  The use of the buzzer in this project is inspired by the guide on [How to Use a Buzzer](https://www.ardumotive.com/how-to-use-a-buzzer-en.html).

Thank you to the open-source community for providing these valuable resources!
