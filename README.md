# Satellite Payload Ground Control System

Ground Control Software for Commercial Satellite Payload Applications

> A Qt-based satellite payload ground control application designed for command transmission, telemetry processing, and real-time image handling.

---

## 📌 Overview

Satellite Payload Ground Control System is a modular ground support software developed for commercial satellite payload operations.

The system provides:

* Payload command configuration and transmission
* Telemetry data reception and frame decoding
* Real-time image display and processing
* Multi-interface communication support

It is designed for ground testing, integration validation, and in-orbit operation support scenarios.

---

## ✨ Key Features

### Command Management

* Parameterized command configuration
* Protocol packaging and checksum verification
* Batch transmission support

### Telemetry Processing

* Real-time telemetry decoding
* Frame-based protocol parsing
* Data visualization and monitoring

### Image Handling

* Real-time image stream reception
* Image decoding and rendering
* Basic image enhancement processing

### Communication Interfaces

* UART (Serial Port)
* CAN Bus
* LVDS Interface

---

## 🏗 System Architecture

```
+--------------------------------------------------+
|                    UI Layer                      |
+--------------------------------------------------+
|           Command / Telemetry Modules            |
+--------------------------------------------------+
|          Frame Parsing Engine (Lua)              |
+--------------------------------------------------+
|     Communication Abstraction Layer              |
|     (UART / CAN / LVDS)                          |
+--------------------------------------------------+
|                Hardware Interface                |
+--------------------------------------------------+
```

---

## 🛠 Development Environment

| Component | Version            |
| --------- | ------------------ |
| Framework | Qt 5.12.3          |
| Compiler  | MSVC 2017 (32-bit) |
| IDE       | Qt Creator 4.9.1   |
| OS        | Windows            |

> ⚠ The project is built with MSVC 32-bit toolchain.

---

## 📦 Dependencies

### Lua 5.4.2

Download:
https://www.lua.org/download.html

### Installation

1. Download Lua 5.4.2 (Windows 32-bit version recommended).
2. Extract required headers and library files.
3. Place them into:

```
satellitePayloadGroundControl/frameFormat/lua/
```

4. Configure include path and library path in the `.pro` file if needed.

---

## 📁 Project Structure

```
satellitePayloadGroundControl/
│
├── communication/
├── frameFormat/
│   └── lua/
├── image/
├── ui/
├── resources/
├── main.cpp
└── *.pro
```

---

## 🚀 Build Instructions

### Using Qt Creator

1. Open `.pro` file
2. Select Qt 5.12.3 (MSVC 2017 32-bit)
3. Build and run

### Command Line

```
qmake
nmake
```

---

## ⚠ Known Constraints

* Windows only
* Hardware-dependent communication layer
* Cross-platform not verified

---

## 📄 License

This project is intended for technical demonstration purposes.
All rights reserved.
