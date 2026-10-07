#include <Arduino.h>
#include <SuplaDevice.h>
#include <supla/control/button.h>
#include <supla/device/status_led.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/esp_wifi.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/storage/littlefs_config.h>

#include "TAC1100.h"

// ---- Piny (numery GPIO ESP8266; Wemos D1 mini w nawiasach) ----
// RS485 (moduł MAX485 / TTL-RS485):
constexpr int8_t PIN_RS485_RX = 14;  // D5  <- RO modułu RS485
constexpr int8_t PIN_RS485_TX = 12;  // D6  -> DI modułu RS485
constexpr int8_t PIN_RS485_DE = -1;  // D7  -> DE+RE (spięte razem); -1 jeśli
                                     //        moduł ma automatyczny kierunek
// Przycisk trybu konfiguracji i dioda statusu:
constexpr int8_t PIN_CFG_BUTTON = 0;  // D3 (przycisk FLASH na płytce)
constexpr int8_t PIN_STATUS_LED = 2;  // D4 (wbudowana dioda, świeci stanem niskim)

// ---- Parametry licznika (domyślne fabryczne TAC1100) ----
constexpr uint8_t METER_ADDRESS = 1;
constexpr uint32_t METER_BAUD = 9600;  // 8N1

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;
Supla::EspWebServer suplaServer;

void setup() {
  Serial.begin(115200);

  // Strona konfiguracyjna WWW (tylko w trybie konfiguracji)
  new Supla::Html::DeviceInfo(&SuplaDevice);
  new Supla::Html::WifiParameters;
  new Supla::Html::ProtocolParameters;
  new Supla::Html::StatusLedParameters;

  // Licznik = jeden kanał pomiarowy Supli
  new TAC1100(PIN_RS485_RX, PIN_RS485_TX, PIN_RS485_DE, METER_ADDRESS,
              METER_BAUD);

  // Przytrzymanie przycisku wprowadza urządzenie w tryb konfiguracji
  // (ESP wystawia własną sieć WiFi; WiFi/serwer/e-mail ustawiasz w przeglądarce)
  auto cfgButton = new Supla::Control::Button(PIN_CFG_BUTTON, true, true);
  cfgButton->configureAsConfigButton(&SuplaDevice);

  new Supla::Device::StatusLed(PIN_STATUS_LED, true);

  SuplaDevice.setName("Licznik TAC1100");
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}
