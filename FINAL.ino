#include <Wire.h>
#include <MAX3010x.h>
#include <WiFi.h>
#include <WebServer.h>
#include "filters.h"
MAX30102 sensor;
const char* ssid = "ABCD";
const char* password = "password";
const int buzzerPin = 15;
#define THERMISTOR_PIN 35 // Pin 35 for thermistor
double adcMax = 4095.0;  // Maximum ADC value for ESP32
double Vs = 3.3;         // 3.3V for ESP32
double R1 = 10000.0;     // Voltage divider resistor value
double Beta = 3997.0;    // Beta value from the thermistor's datasheet
double To = 298.15;      // Temperature in Kelvin for 25°C
double Ro = 10000.0;     // Resistance of thermistor at 25°C
float temperature = 0.0; 
// WebServer on port 80
WebServer server(80);
// Constants for SpO2 calculation
float kSpO2_A = 1.5958422;
float kSpO2_B = -34.6596622;
float kSpO2_C = 112.6898759;
// Filter and statistical instances for heart rate and SpO2
LowPassFilter low_pass_filter_red(5.0, 400.0);
LowPassFilter low_pass_filter_ir(5.0, 400.0);
HighPassFilter high_pass_filter(0.5, 400.0);
Differentiator differentiator(400.0);
MinMaxAvgStatistic stat_red;
MinMaxAvgStatistic stat_ir;
// Variables for SpO2 and BPM calculations
float bpm = 0;
float spo2 = 0;
long last_heartbeat = 0;
long crossed_time = 0;
float last_diff = NAN;
bool crossed = false;
long finger_timestamp = 0;
bool finger_detected = false;
void setup() 
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  // Initialising the MAX30102 sensor
  while(1)
  {
    if (sensor.begin()) 
    {
      Serial.println("MAX30102 sensor started.");
      break;
    } 
    else 
    {
      Serial.println("Sensor initialization failed!");
    }
  }
  // Initialize the buzzer pin as output
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, 1000, 1000);
  server.on("/", HTTP_GET, handleRoot);          // Home page
  server.on("/get_data", HTTP_GET, handleGetData); // Endpoint for AJAX requests
  server.begin();
  Serial.println("Web server started");
}
void loop() 
{
  server.handleClient();
  updateSensorData();
  updateThermistorData();
}
void updateSensorData() 
{
  auto sample = sensor.readSample(2000);  // Increased timeout to 2000 ms
  if (sample.red == 0 || sample.ir == 0) 
  {
    Serial.println("Error: Invalid sensor data or no signal detected.");
    return;
  }
  if (sample.red > 5000) 
  {
    finger_detected = true;
  } 
  else 
  {
    finger_detected = false;
    differentiator.reset();
    low_pass_filter_red.reset();
    low_pass_filter_ir.reset();
    high_pass_filter.reset();
    stat_red.reset();
    stat_ir.reset();
    finger_timestamp = millis();
  }
  if (finger_detected) 
  {
    // Filter to smooth out the sensor data by eliminating high freq. noise like sensor fluctuations, environmental noise etc
    float current_value_red = low_pass_filter_red.process(sample.red);
    float current_value_ir = low_pass_filter_ir.process(sample.ir);
    // Process data for SpO2 and BPM calculations
    stat_red.process(current_value_red);
    stat_ir.process(current_value_ir);
    // High-pass filter
    float current_value = high_pass_filter.process(current_value_red);
    //Differentiator calculates rate of change in filtered signals for BPM 
    float current_diff = differentiator.process(current_value);
    if (!isnan(current_diff) && !isnan(last_diff)) 
    {
      if (last_diff > 0 && current_diff < 0) 
      {
        crossed = true;
        crossed_time = millis();
      }
      if (current_diff > 0) 
      {
        crossed = false;
      }
      if (crossed && current_diff < -2000) 
      {
        if (last_heartbeat != 0 && crossed_time - last_heartbeat > 300) 
        {
          bpm = 60000 / (crossed_time - last_heartbeat);
          // Calculate SpO2
          float rred = (stat_red.maximum() - stat_red.minimum()) / stat_red.average();
          float rir = (stat_ir.maximum() - stat_ir.minimum()) / stat_ir.average();
          float r = rred / rir;
          // Equation of form y=ax2+bx+c to calculate the spo2 level.
          // The constants are experimental values set by the sensor library developer
          spo2 = kSpO2_A * r * r + kSpO2_B * r + kSpO2_C;
          // Trigger the buzzer based on heartbeat
          if (bpm > 0 && bpm < 250) 
          {
            tone(buzzerPin, 200, 100);  // 100ms beep duration
          }
        }
        stat_red.reset();
        stat_ir.reset();
        last_heartbeat = crossed_time;
      }
    }
    last_diff = current_diff;
  }
}
void updateThermistorData() 
{
  double adcValue = analogRead(THERMISTOR_PIN);
  // Voltage across the thermistor using the ADC value
  double Vout = adcValue * Vs / adcMax;
  // Thermistor's resistance
  double Rt = R1 * Vout / (Vs - Vout);
  // Temperature in Kelvin using the Steinhart-Hart equation
  double T = 1.0 / (1.0 / To + log(Rt / Ro) / Beta);
  temperature = 2.1+((T - 273.15) * 0.79);  // Apply correction factor (0.79)
  // Print the results for debugging
  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" | Vout: ");
  Serial.print(Vout);
  Serial.print(" | Thermistor Resistance (Rt): ");
  Serial.print(Rt);
  Serial.print(" | Temperature (C): ");
  Serial.println(temperature);
}
// Web handler for root page
void handleRoot() 
{
  String html = "<html lang='en'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 Heart Rate, SpO2, and Temperature Monitor</title>";
  html += "<style>";
  html += "body { font-family: 'Arial', sans-serif; background-color: #f2f2f2; color: #333; text-align: center; padding: 20px; }";
  html += "h1 { color: #2e8b57; font-size: 36px; margin-bottom: 20px; }";
  html += ".sensor-data { font-size: 24px; margin: 10px 0; font-weight: bold; }";
  html += ".bpm { color: #e74c3c; font-size: 28px; font-weight: bold; animation: pulse 1.5s infinite; }";
  html += ".spo2 { color: #3498db; font-size: 28px; font-weight: bold; }";
  html += ".temperature { color: #2ecc71; font-size: 28px; font-weight: bold; }";
  html += "@keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.1); } 100% { transform: scale(1); } }";
  html += ".spacer { height: 20px; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>ESP32 Heart Rate, SpO2, and Temperature Monitor</h1>";
  html += "<div class='sensor-data'>";
  html += "<p>Heart Rate: <span id='bpm' class='bpm'>" + String(bpm) + "</span> BPM</p>";
  html += "</div>";
  
  // SpO2 Section
  html += "<div class='sensor-data'>";
  html += "<p>SpO2: <span id='spo2' class='spo2'>" + String(spo2) + "</span> %</p>";
  html += "</div>";

  // Temperature Section
  html += "<div class='sensor-data'>";
  html += "<p>Temperature: <span id='temperature' class='temperature'>" + String(temperature) + "</span> °C</p>";
  html += "</div>";
  html += "<div class='spacer'></div>";
  // JavaScript to periodically update data from the server
  html += "<script>";
  html += "function updateData() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/get_data', true);";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      var response = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('bpm').innerHTML = response.bpm;"; 
  html += "      document.getElementById('spo2').innerHTML = response.spo2;"; 
  html += "      document.getElementById('temperature').innerHTML = response.temperature;"; 
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  // Update every 2 seconds
  html += "setInterval(updateData, 2000);";
  html += "</script>";
  html += "</body></html>";
  // Send HTML page to client
  server.send(200, "text/html", html);
}
// Web handler for fetching real-time data in JSON format
void handleGetData() 
{
  String json = "{";
  json += "\"bpm\": " + String(bpm) + ",";
  json += "\"spo2\": " + String(spo2) + ",";
  json += "\"temperature\": " + String(temperature);
  json += "}";
  server.send(200, "application/json", json);
}
