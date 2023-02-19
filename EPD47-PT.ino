#include "configs.h"

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#ifndef HAS_SCREEN
  #define MenuFunctions_h
  #define Display_h
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "epd_driver.h"
#include <Wire.h>
#include <touch.h>
#include "pins.h"

#include <WiFi.h>
#include <Wire.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <Arduino.h>

#include "Assets.h"
#include "WiFiScan.h"
#include "SDInterface.h"
#include "Web.h"
#include "Buffer.h"
#include "BatteryInterface.h"
#include "TemperatureInterface.h"
#include "LedInterface.h"
#include "esp_interface.h"
#include "settings.h"
#include "CommandLine.h"
#include "lang_var.h"
#include "flipperLED.h"

#ifdef HAS_SCREEN
  #include "Display.h"
  #include "MenuFunctions.h"
  #include "a32u4_interface.h"
#endif

WiFiScan wifi_scan_obj;
SDInterface sd_obj;
Web web_obj;
Buffer buffer_obj;
BatteryInterface battery_obj;
TemperatureInterface temp_obj;
LedInterface led_obj;
EspInterface esp_obj;
Settings settings_obj;
CommandLine cli_obj;
flipperLED flipper_led;

#ifdef HAS_SCREEN
  Display display_obj;
  MenuFunctions menu_function_obj;
  A32u4Interface a32u4_obj;
#endif

const String PROGMEM version_number = MARAUDER_VERSION;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, PIN, NEO_GRB + NEO_KHZ800);

uint32_t currentTime  = 0;

void setup()
{
  pinMode(FLASH_BUTTON, INPUT);

  #ifdef HAS_SCREEN
    display_obj.RunSetup();
    display_obj.tft.setRotation(1);
    display_obj.tft.setTextColor(TFT_WHITE, TFT_BLACK);
  #endif

  // Draw the title screen
  #ifdef HAS_SCREEN
    display_obj.drawJpeg("/marauder3L.jpg", 0 , 0);     // 240 x 320 image
  #endif

  #ifdef HAS_SCREEN
    //showCenterText(version_number, 250);
    display_obj.tft.drawCentreString(version_number.c_str(), TFT_WIDTH/2, TFT_HEIGHT/2 - 30, 2);
  #endif

  #ifdef HAS_SCREEN
    delay(2000);

    display_obj.clearScreen();

    display_obj.tft.setTextColor(TFT_CYAN, TFT_BLACK);

    display_obj.tft.println(text_table0[0]);

    delay(2000);

    display_obj.tft.println("Marauder " + version_number + "\n");

    display_obj.tft.println(text_table0[1]);
  #endif

  settings_obj.begin();

  // Initialize WiFi
  wifi_scan_obj.setupWiFi();

  // Start the web server
  web_obj.begin();

  #ifdef MARAUDER_FLIPPER
    flipper_led.RunSetup();
  #endif

  // Enable backlight
  display_obj.tft.setBacklight(255);
}

void loop()
{
  // Update the battery and temperature interfaces
  battery_obj.loop();
  temp_obj.loop();

  // Handle the command line
  cli_obj.handleCommand();

  // Handle button presses
  if (digitalRead(FLASH_BUTTON) == LOW) {
    settings_obj.loadSettingsFromSD();
    delay(1000);
  }

  // Handle the LED strip
  led_obj.handleLED();

  // Handle the ESP interface
 
esp_obj.handleESP();

// Handle the flipper LED
#ifdef MARAUDER_FLIPPER
flipper_led.handleFlipperLED();
#endif

// Handle the web server
web_obj.handleClient();

// Check for new firmware updates
esp_obj.checkForUpdates();

// Check for OTA updates
esp_obj.handleOTA();

// Handle the touch interface
touch_handler();

// Handle the A32u4 interface
#ifdef HAS_SCREEN
a32u4_obj.handleA32u4();
#endif

// Handle the menu functions
#ifdef HAS_SCREEN
menu_function_obj.handleMenuFunctions();
#endif

// Handle the display
#ifdef HAS_SCREEN
display_obj.handleDisplay();
#endif

// Watchdog timer reset
esp_task_wdt_reset();

// Delay to avoid overloading the system
currentTime = millis();
if(currentTime % 10 == 0) {
delay(1);
}
}
