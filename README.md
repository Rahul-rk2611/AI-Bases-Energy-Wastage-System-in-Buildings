# AIoT-Based Smart Energy Monitoring and Occupancy Detection System

## Project Overview

The AIoT-Based Smart Energy Monitoring and Occupancy Detection System is an intelligent automation project developed using ESP32, ESP32-CAM, and multiple IoT sensors. The primary objective of this system is to reduce unnecessary energy consumption by automatically detecting room occupancy and controlling electrical appliances accordingly. The system combines Artificial Intelligence, Internet of Things (IoT), and real-time monitoring technologies to create an efficient and smart energy management solution suitable for smart homes, offices, classrooms, and industrial environments.

The ESP32-CAM module is used for AI-based human detection, while the ESP32 microcontroller processes sensor data collected from PIR sensors, ultrasonic sensors, and current sensors. Based on the occupancy status and sensor readings, the relay module automatically switches electrical appliances ON or OFF to prevent energy wastage. A real-time web dashboard is also developed for monitoring occupancy status, current consumption, power usage, and relay conditions through Wi-Fi communication.

---

## Features

The system provides AI-based human occupancy detection using the ESP32-CAM module integrated with Edge Impulse machine learning models. It supports real-time monitoring of room occupancy, current consumption, power usage, and appliance status. The project includes automatic relay control for appliance automation and also provides manual relay control through a web dashboard. Wireless communication between ESP32 and ESP32-CAM is established through Wi-Fi for real-time data transmission and monitoring.

---

## Hardware Components

The project consists of several hardware modules including the ESP32 development board, ESP32-CAM module, PIR motion sensor, ultrasonic sensor, current sensor, voltage sensor, relay module, SMPS power supply, LEDs, breadboard, and jumper wires. The ESP32 acts as the main processing unit, while the ESP32-CAM performs AI-based occupancy detection. The sensors continuously collect environmental and electrical data for monitoring and automation purposes.

---

## Software Requirements

The project is developed using Arduino IDE and Embedded C programming. Edge Impulse is used for training and deploying the machine learning model for human detection. HTML, CSS, and JavaScript are used for designing the web dashboard interface. ESP32 Wi-Fi libraries and HTTP communication libraries are used for establishing wireless communication between devices.

---

## Working Principle

The system continuously monitors room occupancy and electrical parameters using multiple sensors. The PIR sensor detects motion, while the ultrasonic sensor measures distance and occupancy presence. The ESP32-CAM captures images and performs AI-based human detection using a trained machine learning model. The ESP32 receives occupancy information and sensor data, processes the information, and determines whether the room is occupied or empty.

If human presence is detected, the relay module keeps the appliances active. If no occupancy is detected, the relay automatically switches OFF connected appliances to reduce unnecessary energy consumption. The current sensor continuously measures electrical current, and power consumption is calculated in real time. All monitored data is displayed through a web-based dashboard for user interaction and monitoring.

---

## Dashboard Description

The system includes a real-time IoT web dashboard that displays occupancy status, current values, power consumption, relay status, and sensor readings. The dashboard provides a user-friendly interface for monitoring system performance remotely through Wi-Fi. Users can also manually control the relay module directly from the dashboard whenever required. The dashboard updates data periodically for continuous monitoring and automation control.

---

## Applications

This project can be used in smart homes, classrooms, offices, libraries, laboratories, industries, and commercial buildings for intelligent energy management. It is highly suitable for smart building automation systems where reducing energy wastage and improving power efficiency are important requirements.

---

## Advantages

The proposed system helps reduce unnecessary power consumption by automating appliance control based on occupancy detection. It improves energy efficiency, supports real-time monitoring, and enables remote access through IoT technology. The integration of Artificial Intelligence improves occupancy detection accuracy compared to traditional sensor-only systems. The project is also cost-effective, scalable, and suitable for future smart automation applications.

---

## Future Enhancements

The system can be further enhanced by integrating cloud storage, mobile applications, voice assistant support, MQTT communication protocols, and advanced AI models for improved accuracy and scalability. Additional sensors and multiple room monitoring capabilities can also be added for large-scale smart building applications.

---

## Conclusion

The AIoT-Based Smart Energy Monitoring and Occupancy Detection System successfully demonstrates the integration of Artificial Intelligence and IoT technologies for smart energy management. The project effectively reduces energy wastage through intelligent occupancy detection and automated appliance control. The system provides real-time monitoring, remote accessibility, and smart automation features, making it suitable for modern energy-efficient smart environments.
