# Requirements:
## ESP32 Microcontroller
## NTC 10K Ohm Thermistor
## MAX3010x Sensor
# Description:
## •	ESP32 microcontroller: This is the central processing unit that makes the MAX30102 sensor, thermistor, and the web server functional.
## •	MAX30102 sensor: This sensor measures the heart rate and SpO2 using photoplethysmography (PPG) technique. This sensor contains two LEDs, one emits red light and the other emits infrared light. Based on the amount of oxygen present in hemoglobin, the lights are absorbed. Hemoglobin with more oxygen absorb more infrared light, while the hemoglobin with less oxygen absorbs more red light and less infrared light. Some light which is not absorbed gets scattered or reflected back to the photosensor which collects the light data. Then by comparing the ratio of red and infrared light absorbed, the sensor data is used to calculate the ratio of oxygenated hemoglobin to deoxygenated hemoglobin in the blood which is used to find the blood oxygen saturation (SpO2) level.
## For heart rate, when heart pushes blood through blood vessels then the intensity of light absorbed and reflected fluctuates, these fluctuations or changes are used to calculate the pulse rate.
## •	Thermistor: A temperature sensor that is used to measure temperature. In this case it is a 10kΩ NTC thermistor. The thermistor’s resistance changes with temperature, and this variation is measured to calculate the body temperature. The sensor is calibrated using the Steinhart-Hart equation, ensuring accurate temperature readings across the relevant temperature range.
## •	Buzzer: A device that produces sound indicating the heart beats.
