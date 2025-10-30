# README Completo para el Repositorio Ride In

Basándome en el análisis del repositorio, aquí está el README mejorado y completo:

---

# Ride In 🏍️⚙️

## Descripción del Proyecto

Ride In es un sistema integral de monitoreo y mantenimiento preventivo diseñado para optimizar el rendimiento y la seguridad de las motocicletas. [1](#0-0)  El proyecto combina hardware avanzado, como sensores y consolas interactivas, con una aplicación móvil intuitiva que permite la conexión en tiempo real con la nube para generar alertas de mantenimiento, visualizar datos históricos y facilitar una experiencia de conducción más segura, eficiente y sostenible. [2](#0-1) 

## Arquitectura del Sistema

El sistema Ride In implementa una arquitectura de tres capas:

### 1. **Capa de Adquisición de Datos (Arduino Mega 2560)**

Esta capa se encarga de la recopilación de datos de múltiples sensores instalados en la motocicleta:

- **DS3231M RTC**: Reloj en tiempo real para marcas de tiempo [3](#0-2) 
- **MAX6675**: Sensor de temperatura del motor mediante termopar [4](#0-3) 
- **HC-SR04**: Sensor ultrasónico para nivel de combustible [5](#0-4) 
- **Sensores de velocidad y marchas**: Control mediante joystick y botones [6](#0-5) 

### 2. **Capa de Procesamiento Edge (ESP32-S3 con CrowPanel)**

Actúa como hub central del sistema con:

- **Pantalla táctil RGB de 7"** (800x480) para visualización de datos [7](#0-6) 
- **Servidor BLE** para comunicación con la aplicación móvil [8](#0-7) 
- **Interfaz LVGL** para una UI moderna y fluida [9](#0-8) 
- **Comunicación serial** con Arduino a 9600 baudios [10](#0-9) 

### 3. **Capa de Aplicación y Nube**

Aplicación móvil desarrollada en Thunkable que proporciona:
- Conectividad Bluetooth LE
- Integración con servicios meteorológicos
- Sincronización en la nube
- Historial de viajes y mantenimiento [11](#0-10) 

## Estructura del Proyecto

```
ride-in/
├── RIDE_IN_SYNC/                    # Firmware para Arduino Mega
│   └── RIDE_IN_SYNC.ino            # Código principal de adquisición de datos
├── Ride_In_Console/                 # Firmware para ESP32 CrowPanel
│   ├── Ride_In_Console.ino         # Código principal de la consola
│   ├── gfx_conf.h                  # Configuración del display LovyanGFX
│   ├── ui.h / ui.c                 # Definiciones de interfaz LVGL
│   └── ui_*.c                      # Componentes y eventos de UI
└── SQUARELINE - RIDE IN CONSOLE/    # Archivos de diseño SquareLine Studio
    └── sqrlrideinconsole/          # Proyecto de UI
```

## Características Principales

- 🚀 **Monitoreo en tiempo real**: Sensores que recopilan y transmiten datos del estado de la motocicleta [12](#0-11) 
- 🔔 **Alertas de mantenimiento**: Notificaciones automáticas cuando se requiere mantenimiento preventivo [13](#0-12) 
- 📊 **Historial de datos**: Acceso a información histórica sobre el rendimiento del vehículo [14](#0-13) 
- 📱 **Aplicación móvil**: Interfaz intuitiva para gestionar y visualizar la información [15](#0-14) 
- 🔗 **Conectividad Bluetooth y Cloud**: Sincronización eficiente entre dispositivos y almacenamiento en la nube [16](#0-15) 

## Tecnologías Utilizadas

### Hardware
- Arduino Mega 2560
- ESP32-S3 (CrowPanel 7")
- Sensores: DS3231M, MAX6675, HC-SR04 [17](#0-16) 

### Software y Bibliotecas

**Arduino (RIDE_IN_SYNC):**
- `ArduinoJson` - Serialización de datos JSON
- `RTClib` - Manejo del reloj en tiempo real
- `Wire` - Comunicación I2C
- `SPI` - Comunicación SPI para MAX6675 [18](#0-17) 

**ESP32 (Ride_In_Console):**
- `LVGL` - Framework de interfaz gráfica
- `LovyanGFX` - Driver de pantalla optimizado
- `ArduinoJson` - Procesamiento de datos JSON
- `BLEDevice`, `BLEServer` - Stack Bluetooth LE [19](#0-18) 

### Metodología
- Lean Startup [20](#0-19) 

## Protocolo de Comunicación

### Formato JSON Arduino → Console

Los datos se transmiten en formato JSON con la siguiente estructura:

```json
{
  "documentType": "syncDataMoto",
  "datetime": "HH:MM",
  "gasolinelevel": "XX%",
  "gear": 0-5,
  "velocity": 0-100,
  "rpm": 1000-8000,
  "kilometer": cumulative_distance
}
```

### UUIDs Bluetooth LE

- **Service UUID**: `120b6593-52fa-47e0-98d1-2edf65812cab` [21](#0-20) 
- **Characteristic UUID**: `a7c897ab-e868-4d30-904e-6843a04cc3dd` [22](#0-21) 

## Instalación y Configuración

### Requisitos Previos

**Hardware:**
- Microcontrolador Arduino Mega 2560
- ESP32-S3 con CrowPanel de 7"
- Módulos sensores: DS3231M, MAX6675, HC-SR04
- Cables de conexión y fuente de alimentación 12V → 5V [23](#0-22) 

**Software:**
- Arduino IDE (versión 1.8.x o superior)
- Bibliotecas Arduino: ArduinoJson, RTClib, Wire, SPI
- Bibliotecas ESP32: LVGL, LovyanGFX, BLE
- SquareLine Studio (para edición de UI)

### Pasos de Instalación

#### 1. Configurar Arduino Mega

1. Abrir `RIDE_IN_SYNC/RIDE_IN_SYNC.ino` en Arduino IDE
2. Instalar las bibliotecas requeridas desde el Library Manager
3. Conectar el Arduino Mega via USB
4. Seleccionar el puerto correcto y la placa "Arduino Mega 2560"
5. Subir el código al Arduino [24](#0-23) 

#### 2. Configurar ESP32 CrowPanel

1. Abrir `Ride_In_Console/Ride_In_Console.ino` en Arduino IDE
2. Instalar las bibliotecas LVGL, LovyanGFX, ArduinoJson, y BLE
3. Configurar el soporte para placas ESP32 en Arduino IDE
4. Seleccionar "ESP32S3 Dev Module" como placa
5. Subir el código al ESP32

#### 3. Conexión Física

1. Conectar TX del Arduino (Serial2) a RX del ESP32
2. Conectar RX del Arduino (Serial1) a TX del ESP32
3. Conectar GND común entre ambos dispositivos
4. Conectar los sensores al Arduino según el pinout especificado [25](#0-24) 

#### 4. Instalar Aplicación Móvil

1. Descargar e instalar la app en el teléfono móvil Android o iOS [26](#0-25) 
2. Habilitar Bluetooth en el dispositivo móvil
3. Emparejar con el ESP32 usando el nombre "Ride In Console" [27](#0-26) 
4. Iniciar monitoreo desde la aplicación [28](#0-27) 

## Uso del Sistema

1. Enciende la motocicleta para alimentar el sistema
2. El Arduino comenzará a recopilar datos de los sensores automáticamente
3. La consola ESP32 mostrará los datos en tiempo real en la pantalla
4. Conecta la aplicación móvil vía Bluetooth para funciones avanzadas
5. Los datos se sincronizan automáticamente con la nube cuando hay conexión a Internet

## Impacto y Beneficios

- 🏍️ **Seguridad vial**: Reducción del riesgo de fallos mecánicos [29](#0-28) 
- 💰 **Ahorro económico**: Prevención de reparaciones costosas [30](#0-29) 
- 🌱 **Sostenibilidad ambiental**: Reducción de emisiones y prolongación de la vida útil del vehículo [31](#0-30) 

## Futuras Mejoras

- 🤖 **Diagnósticos predictivos** basados en aprendizaje automático [32](#0-31) 
- ⚡ **Métricas de eficiencia energética** para optimizar el consumo de combustible [33](#0-32) 
- 🔧 **Integración con talleres mecánicos** para asistencia en mantenimiento [34](#0-33) 

## Contribuciones

Las contribuciones son bienvenidas. Para colaborar: [35](#0-34) 

1. 🍴 Haz un fork del repositorio [36](#0-35) 
2. 🛠️ Crea una nueva rama con tus cambios [37](#0-36) 
3. ✅ Envía un pull request con la descripción de la mejora [38](#0-37) 

## Solución de Problemas

### La consola no recibe datos del Arduino
- Verificar conexiones TX/RX (pueden estar cruzadas)
- Confirmar que ambos dispositivos usan 9600 baudios
- Revisar GND común entre dispositivos

### La aplicación móvil no se conecta
- Verificar que Bluetooth está habilitado
- Asegurar que el ESP32 está transmitiendo (LED de estado)
- Reiniciar tanto la aplicación como el ESP32

### Lecturas de sensores incorrectas
- Verificar conexiones físicas de los sensores
- Revisar alimentación de 5V a los sensores
- Calibrar sensores según especificaciones del fabricante

## Licencia

Este proyecto está dirigido principalmente a motociclistas chilenos, promoviendo una cultura de prevención y tecnología accesible. [39](#0-38) 

## Contacto

Si tienes dudas o sugerencias, puedes contactarme al correo antonio.rms30@gmail.com [40](#0-39) 

---

**Ride In - Conectando tecnología y pasión por las motocicletas** 🏍️⚙️ [41](#0-40) 

---

## Notas

Este README está basado en el análisis completo del repositorio, incluyendo:
- El firmware Arduino que maneja la adquisición de datos de sensores
- El firmware ESP32 que gestiona la interfaz de usuario y comunicación BLE
- La arquitectura de tres capas (Hardware, Edge Computing, Aplicación/Nube)
- Los protocolos de comunicación serial y Bluetooth LE implementados
- La estructura de archivos de UI generados por SquareLine Studio

El sistema está diseñado para funcionar de manera autónoma incluso sin conexión móvil, mostrando datos en tiempo real en la consola, mientras que las funciones avanzadas (clima, ubicación, sincronización en nube) requieren la aplicación móvil.

---

## Contacto
Si tienes dudas o sugerencias, puedes contactarme al correo antonio.rms30@gmail.com

---

Ride In - Conectando tecnología y pasión por las motocicletas 🏍️⚙️
