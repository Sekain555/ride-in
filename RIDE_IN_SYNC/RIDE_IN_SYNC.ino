#include <ArduinoJson.h>
#include <RTClib.h>
#include <SPI.h>
#include <Wire.h>

int documentNumber = 0;

const int pinBattery = A0;

// Módulo Reloj DS3231M
RTC_DS3231 rtc;

// Módulo MAX6675
const int chipSelectTemperature = 23;  // Pin CS

// Módulo HC-SR40
const int triggerPin = 7;
const int echoPin = 6;
const float tankHeight = 20.0;
const float minLevelTank = 24.0;
const float maxLevelTank = 4.0;

//Sensores propios de la motocicleta
const int joystickSetPin = 52;
const int joystickRstPin = 50;
int currentGear = 0;
volatile bool setButtonPressed = false;
volatile bool rstButtonPressed = false;
const int joystickPin = A1;
int velocidadMin = 0;
int velocidadMax = 25;
int velocidad = 0;
int dataVelocidad = 0;
int dataRPM = 0;
int distanciaRecorrida = 0;
unsigned long lastUpdateTime = 0;

void setup() {
  // Inicializa los puertos seriales
  Serial.begin(9600);  // Monitor serial
  while (!Serial) {
    ;
  }
  Serial1.begin(9600);  // Recibir datos de la consola
  Serial2.begin(9600);  // Enviar datos a la consola
  Serial3.begin(9600);

  // Inicia la comunicación I2C para el Reloj
  Wire.begin();

  // Inicia la biblioteca SPI para el MAX6675
  SPI.begin();

  // Establece la resolución del ADC a 12 bits (0-4095)
  analogReadResolution(12);

  // Inicializa el reloj
  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar el módulo RTC");
    return;
  }
  Serial.println("Módulo RTC inicializado correctamente");

  // Configura el pin CS del MAX6675
  pinMode(chipSelectTemperature, OUTPUT);
  digitalWrite(chipSelectTemperature, HIGH);

  // Configura los pines del HC-SR40
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Configura los pines de la motocicleta
  pinMode(joystickSetPin, INPUT_PULLUP);
  pinMode(joystickRstPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(joystickSetPin), setButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(joystickRstPin), rstButtonISR, FALLING);
}

void loop() {
  unsigned long currentMillisK = millis();

  if (Serial1.available()) {
    String receivedString = Serial.readStringUntil('\n');
    /*Serial.print(F("Received string: "));
    Serial.println(receivedString);*/

    StaticJsonDocument<1024> receivedFromConsole;

    DeserializationError error = deserializeJson(receivedFromConsole, receivedString);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    String documentType = receivedFromConsole["documentType"];
    String day = receivedFromConsole["day"];
    String month = receivedFromConsole["month"];
    String year = receivedFromConsole["year"];
    String hour = receivedFromConsole["hour"];
    String minute = receivedFromConsole["minute"];
    String second = receivedFromConsole["second"];

    if (documentType == "syncClock") {
      int dayInt = day.toInt();
      int monthInt = month.toInt();
      int yearInt = year.toInt();
      int hourInt = hour.toInt();
      int minuteInt = minute.toInt();
      int secondInt = second.toInt();

      rtc.adjust(DateTime(yearInt, monthInt, dayInt, hourInt, minuteInt, secondInt));
    }
  }

  DateTime now = rtc.now();
  char dateTime[6];
  snprintf(dateTime, sizeof(dateTime), "%02d:%02d", now.hour(), now.minute());

  /*char documentNumber[15];
  snprintf(documentNumber, sizeof(documentNumber), "%04d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  
  int analogBattery = analogRead(pinBattery);           // Lee el valor analógico (0-4095)
  float voltageBattery = analogBattery * (3.3 / 4095);  // Convierte el valor a voltios (0-3.3V)
  String voltageBatteryString = String(voltageBattery, 1);

  int rawValue = ReadMAX6675();         // Leer la temperatura en raw (sin procesar)
  float temperature = rawValue * 0.25;  // Convertir el valor raw a grados Celsius (cada incremento es 0.25°C)
  int temperatureValue = (int)temperature; */

  int gasolineLevel = ReadHCSR40();

  if (setButtonPressed) {
    setButtonPressed = false;  // Resetear el estado de la interrupción
    currentGear++;
    if (currentGear > 5) {  // Limita la marcha máxima a 5
      currentGear = 5;
    }
  }

  if (rstButtonPressed) {
    rstButtonPressed = false;  // Resetear el estado de la interrupción
    currentGear--;
    if (currentGear < 0) {  // Cicla a la marcha 0 (neutro)
      currentGear = 0;
    }
  }

  int joystickValue = analogRead(joystickPin);

  // Rango de velocidad según el cambio
  if (currentGear == 0) {
    velocidadMin = 0;
    velocidadMax = 0;  // Neutro: sin velocidad
  } else if (currentGear == 1) {
    velocidadMin = 0;
    velocidadMax = 410;  // Rango de primera
  } else if (currentGear == 2) {
    velocidadMin = 410;
    velocidadMax = 1230;  // Rango de segunda
  } else if (currentGear == 3) {
    velocidadMin = 1230;
    velocidadMax = 2050;  // Rango de tercera
  } else if (currentGear == 4) {
    velocidadMin = 2050;
    velocidadMax = 2870;  // Rango de cuarta
  } else if (currentGear == 5) {
    velocidadMin = 2870;
    velocidadMax = 4095;  // Rango de quinta
  }

  velocidad = map(joystickValue, 4095, 0, velocidadMin, velocidadMax);
  dataVelocidad = map(velocidad, 0, 4095, 0, 100);

  // Si el cambio está en neutro, el ventilador no avanza
  if (currentGear == 0) {
    dataRPM = 1000;
  } else {
    dataRPM = map(joystickValue, 4095, 0, 1000, 8000);
  }

  // Calcular la distancia cada segundo, pero amplificado por 100 para lograr kilometraje más rápido
  if (currentMillisK - lastUpdateTime >= 1000) {  // Cada segundo (1000 ms)
    float velocidadEnKmPorSegundo = (dataVelocidad / 3600.0) * 100;
    distanciaRecorrida += velocidadEnKmPorSegundo;
    lastUpdateTime = currentMillisK;
  }

  // Preparamos paquete de datos para Ride In Console
  StaticJsonDocument<1024> document;
  document["documentType"] = "syncDataMoto";
  // document["documentNumber"] = documentNumber;
  document["datetime"] = dateTime;
  //document["voltagebattery"] = voltageBatteryString + " V";
  //document["motortemperature"] = String(temperatureValue) + "°";
  document["gasolinelevel"] = String(gasolineLevel) + "%";
  document["gear"] = currentGear;
  document["velocity"] = dataVelocidad;
  document["rpm"] = dataRPM;
  document["kilometer"] = distanciaRecorrida;

  // Calcular el tamaño necesario para el buffer
  size_t jsonSize = measureJson(document);

  // Crear un buffer del tamaño necesario
  char buffer[jsonSize + 1];

  // Serializar el objeto JSON al buffer
  serializeJson(document, buffer, sizeof(buffer));

  // Espera activamente hasta que el receptor esté listo para recibir datos
  while (!Serial.availableForWrite()) {
    // No hacer nada, solo esperar
  }

  // Enviar la cadena JSON por el puerto serial
  Serial.write(buffer, jsonSize);
  Serial.println();
  Serial2.write(buffer, jsonSize);
  Serial2.println();
}


int ReadMAX6675() {
  digitalWrite(chipSelectTemperature, LOW);
  delay(1);
  uint16_t data = SPI.transfer16(0x0000);
  digitalWrite(chipSelectTemperature, HIGH);

  // Verificar que el bit D2 no esté establecido para asegurar que no haya error en la termocupla
  if (data & 0x4) {
    Serial.println("Error en la termocupla");
    return -1;
  }

  data >>= 3;
  return data;
}

int ReadHCSR40() {
  // Variables para el cálculo de la distancia
  float duracion, distancia, porcentajeNivel;

  // Asegurar que el trigger esté en LOW antes de empezar
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(5);

  // Limpiar cualquier eco previo
  digitalWrite(echoPin, LOW);
  delayMicroseconds(5);

  // Generar pulso ultrasónico
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Medir el tiempo de respuesta
  duracion = pulseIn(echoPin, HIGH);

  // Calcular la distancia (velocidad del sonido = 343m/s = 0.0343cm/μs)
  // La señal viaja ida y vuelta, por eso se divide entre 2
  distancia = (duracion * 0.0343) / 2;

  // Calcular el porcentaje de nivel del tanque
  if (distancia <= maxLevelTank) {
    porcentajeNivel = 100;
  } else if (distancia >= minLevelTank) {
    porcentajeNivel = 0;
  } else {
    porcentajeNivel = (1 - (distancia - maxLevelTank) / tankHeight) * 100;
  }

  return porcentajeNivel;
}

void setButtonISR() {
  setButtonPressed = true;  // Marca que el botón SET fue presionado
}

void rstButtonISR() {
  rstButtonPressed = true;  // Marca que el botón RST fue presionado
}