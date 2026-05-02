# Requirements

- ESP32 Microcontroller  
- NTC 10K Ohm Thermistor  
- MAX3010x Sensor  

# Description

- **ESP32 Microcontroller**:  
  This is the central processing unit that makes the MAX30102 sensor, thermistor, and the web server functional.

- **MAX30102 Sensor**:  
  This sensor measures heart rate and SpO₂ using the photoplethysmography (PPG) technique. It contains two LEDs—one emits red light and the other emits infrared light. Based on the amount of oxygen present in hemoglobin, the light is absorbed differently:
  - Oxygen-rich hemoglobin absorbs more infrared light.
  - Oxygen-poor hemoglobin absorbs more red light and less infrared light.  

  Some of the light that is not absorbed is reflected back to a photosensor, which collects the data. By comparing the ratio of absorbed red and infrared light, the sensor calculates the ratio of oxygenated to deoxygenated hemoglobin, which is used to determine blood oxygen saturation (SpO₂).

  For heart rate measurement, when the heart pumps blood through the vessels, the intensity of absorbed and reflected light fluctuates. These fluctuations are used to calculate the pulse rate.

- **Thermistor (10kΩ NTC)**:  
  A temperature sensor used to measure body temperature. Its resistance changes with temperature, and this variation is used to calculate temperature. The sensor is calibrated using the Steinhart–Hart equation, ensuring accurate readings across the relevant temperature range.

- **Buzzer**:  
  A device that produces sound to indicate heartbeats.
