# FreeLament Extrusion System

Smart filament extrusion system that recycles plastic straws into reusable 3D printing filament using dual ESP32 microcontrollers, PID temperature control, and live monitoring.

---

## Features

* PID-controlled heating system
* RPM-controlled auger motor
* ESP32 web dashboard
* ESP32-CAM live monitoring
* Diameter sensor and filament winder
* LCD + rotary encoder controls
* PHP + MySQL authentication backend

---

## Hardware

### Main ESP32

Handles:

* PID temperature control
* Heating element
* Motor RPM
* LCD display
* Web dashboard

### Secondary ESP32

Handles:

* Diameter sensing
* Filament winding
* Web dashboard

### ESP32-CAM

Handles:

* Live video streaming

---

## Database Setup

```sql
CREATE DATABASE esp32_database;

USE esp32_database;

CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL
);

CREATE TABLE espcodes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    unique_code VARCHAR(100) UNIQUE NOT NULL
);
```

---

## Installation

### Arduino Libraries

Install:

* WiFi
* WebServer
* SPIFFS
* max6675
* PID_v1
* LiquidCrystal_I2C
* esp32-camera

### Backend

Place backend files inside:

```bash
xampp/htdocs/capstone/
```

Start:

* Apache
* MySQL

Import:

* `database.sql`

---

## System Workflow

1. Power on ESP32
2. User sets temperature and RPM
3. Barrel preheats
4. Auger motor starts
5. Plastic extrudes through nozzle
6. Cooling and winding begin
7. Filament is collected

---

## Safety Features

* Emergency shutdown
* Over-temperature protection
* Cooling system
* Motor guards
* Cable management

---

## Future Improvements

* Cloud dashboard
* Mobile app
* Automatic diameter correction
* AI-assisted extrusion tuning
