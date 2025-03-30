#include <Wire.h>
#include <lvgl.h>
#include "ui.h"
#include "gfx_conf.h"
#include "ui_events.c"
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_draw_buf1[screenWidth * screenHeight / 8];
static lv_color_t disp_draw_buf2[screenWidth * screenHeight / 8];
static lv_disp_drv_t disp_drv;

StaticJsonDocument<512> syncDataMotoDocument;
StaticJsonDocument<1024> syncBLEDocument;

#define SERVICE_UUID "120b6593-52fa-47e0-98d1-2edf65812cab"
#define CHARACTERISTIC_UUID "a7c897ab-e868-4d30-904e-6843a04cc3dd"
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t*)&color_p->full);

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    //Set the coordinates
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    lv_label_set_text(ui_BLUETOOTH_TXT, "Conectado");
  }

  void onDisconnect(BLEServer* pServer) {
    lv_label_set_text(ui_DETALLE_MOTO_TXT, "Euromot Rex 150");
    lv_label_set_text(ui_BLUETOOTH_TXT, "No conectado");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      //Serial.print("Received: ");
      for (int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }
      Serial.println();

      DeserializationError error = deserializeJson(syncBLEDocument, rxValue);

      //String  = syncBLEDocument[""];
      String documentType = syncBLEDocument["documentType"];
      String userName = syncBLEDocument["userName"];
      String motoName = syncBLEDocument["motoName"];
      String city = syncBLEDocument["city"];
      String temperature = syncBLEDocument["temperature"];
      String wind = syncBLEDocument["wind"];
      String descriptionWeather = syncBLEDocument["descriptionWeather"];
      String day = syncBLEDocument["day"];
      String month = syncBLEDocument["month"];
      String year = syncBLEDocument["year"];
      String hour = syncBLEDocument["hour"];
      String minute = syncBLEDocument["minute"];
      String second = syncBLEDocument["second"];

      String detalleMoto = String(userName.c_str()) + " - " + String(motoName.c_str());

      StaticJsonDocument<1024> syncClockDue;
      syncClockDue["documentType"] = "syncClock";
      syncClockDue["day"] = day;
      syncClockDue["month"] = month;
      syncClockDue["year"] = year;
      syncClockDue["hour"] = hour;
      syncClockDue["minute"] = minute;
      syncClockDue["second"] = second;
      // Calcular el tamaño necesario para el buffer
      size_t jsonSize = measureJson(syncClockDue);
      // Crear un buffer del tamaño necesario
      char buffer[jsonSize + 1];
      // Serializar el objeto JSON al buffer
      serializeJson(syncClockDue, buffer, sizeof(buffer));

      int documentTypeCode = 0;
      if (documentType == "syncInicialBLE") {
        documentTypeCode = 1;
      } else if (documentType == "syncUbication") {
        documentTypeCode = 2;
      } else if (documentType == "syncWeather") {
        documentTypeCode = 3;
      } else if (documentType == "syncClock") {
        documentTypeCode = 4;
      } else if (documentType == "syncMotoData") {
        documentTypeCode = 5;
      }

      switch (documentTypeCode) {
        case 1:
          lv_label_set_text(ui_DETALLE_MOTO_TXT, detalleMoto.c_str());
          break;
        case 2:
          lv_label_set_text(ui_UBICACION_TXT, city.c_str());
          break;
        case 3:
          lv_label_set_text(ui_CLIMA_TXT, temperature.c_str());
          lv_label_set_text(ui_VIENTO_TXT, wind.c_str());
          if (descriptionWeather == "Clear") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_clear_png);
          } else if (descriptionWeather == "Cloudy") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_cloudy_png);
          } else if (descriptionWeather == "Heavy rain") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_rain_png);
          } else if (descriptionWeather == "Fog") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_fog_png);
          } else if (descriptionWeather == "Heavy showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_showers_png);
          } else if (descriptionWeather == "Heavy showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_showers_png);
          } else if (descriptionWeather == "Heavy snow") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_snow_png);
          } else if (descriptionWeather == "Heavy snow showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_snow_showers_png);
          } else if (descriptionWeather == "Light rain") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_rain_png);
          } else if (descriptionWeather == "Light showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_showers_png);
          } else if (descriptionWeather == "Light sleet") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_sleet_png);
          } else if (descriptionWeather == "Light sleet showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_sleet_showers_png);
          } else if (descriptionWeather == "Light snow") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_snow_png);
          } else if (descriptionWeather == "Light snow showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_light_snow_showers_png);
          } else if (descriptionWeather == "Partly cloudy") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_partly_cloudy_png);
          } else if (descriptionWeather == "Snow showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_heavy_snow_showers_png);
          } else if (descriptionWeather == "Sunny") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_sunny_png);
          } else if (descriptionWeather == "Thundery heavy rain") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_thundery_heavy_rain_png);
          } else if (descriptionWeather == "Thundery showers") {
            lv_img_set_src(ui_CLIMA_IMG, &ui_img_thundery_showers_png);
          }
          break;
        case 4:
          while (!Serial.availableForWrite()) {}
          Serial.write(buffer, jsonSize);
          break;
        case 5:
          lv_label_set_text(ui_CONSEJO_TXT, "Sincronización de datos en curso, espere un momento...");
          break;
        default:
          Serial.println("Tipo de documento desconocido");
          Serial.println("No se han recibido datos de la app (BLE)");
          break;
      }
    }
  }
};

void setup() {
  Serial.begin(9600);

  //GPIO init
#if defined(CrowPanel_50) || defined(CrowPanel_70)
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  pinMode(17, OUTPUT);
  digitalWrite(17, LOW);
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
  pinMode(42, OUTPUT);
  digitalWrite(42, LOW);
#endif

  //Display Prepare
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  delay(200);

  lv_init();

  delay(100);

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * screenHeight / 8);
  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.full_refresh = 1;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver  */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  tft.fillScreen(TFT_BLACK);

  ui_init();

  /*Iniciar módulo Bluetooth BLE en el ESP32 de la consola*/
  BLEDevice::init("Euromot Rex 150 RNB");

  pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  pServer->setCallbacks(new MyServerCallbacks());
  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Server started");

  Serial.println("Setup done");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');  // Lee los datos disponibles en el monitor serial

    // Deserializa los datos recibidos en un objeto JSON
    DeserializationError error = deserializeJson(syncDataMotoDocument, data);

    if (!error) {
      // Accede a los valores del objeto JSON recibido
      String documentnumber = syncDataMotoDocument["documentNumber"];
      String dateTime = syncDataMotoDocument["datetime"];
      String voltageBattery = syncDataMotoDocument["voltagebattery"];
      String motorTemperature = syncDataMotoDocument["motortemperature"];
      int motorTemp = motorTemperature.toInt();
      String gasolineLevel = syncDataMotoDocument["gasolinelevel"];
      int gasoline = gasolineLevel.toInt();
      String gear = syncDataMotoDocument["gear"];
      String velocity = syncDataMotoDocument["velocity"];
      int velocityMoto = velocity.toInt();
      String rpm = syncDataMotoDocument["rpm"];
      int RPMMoto = rpm.toInt();
      String kilometer = syncDataMotoDocument["kilometer"];

      if (gear == "0"){
        gear = "N";
      }

      lv_label_set_text(ui_RPM_TXT, rpm.c_str());
      lv_arc_set_value(ui_RPMX1000, RPMMoto);
      lv_label_set_text(ui_VELOCIDAD_TXT, velocity.c_str());
      lv_arc_set_value(ui_VELOCIMETRO, velocityMoto);
      lv_label_set_text(ui_CAMBIOS_TXT, gear.c_str());
      lv_label_set_text(ui_HORA_TXT, dateTime.c_str());
      lv_label_set_text(ui_BATERIA_TXT, voltageBattery.c_str());
      lv_label_set_text(ui_TEMPERATURA_MOTOR_TXT, motorTemperature.c_str());
      lv_label_set_text(ui_GASOLINA_TXT, gasolineLevel.c_str());
      lv_bar_set_value(ui_GASOLINE, gasoline, LV_ANIM_ON);
      lv_label_set_text(ui_KILOMETRAJE_TXT, kilometer.c_str());
    }
  }
  lv_timer_handler();
}