
#include <iostream>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "jsoncpp/value.h"
#include "jsoncpp/json.h"

#include "esp_firebase/app.h"
#include "esp_firebase/rtdb.h"

#include "wifi_utils.h"

#include "firebase_config.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

using namespace ESPFirebase;
extern "C" void app_main(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);  // GPIO 34
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_0);  // GPIO 34
    adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11); // GPIO 34

    wifiInit(SSID, PASSWORD); // blocking until it connects
    printf("Connected to wifi\n");

    // // Config and Authentication
    FirebaseApp app = FirebaseApp(API_KEY);
    RTDB db = RTDB(&app, DATABASE_URL);
    Json::Value batt_data, sys_data;
    while (1)
    {
        int32_t sum = 0;
        for (int i = 0; i < 100; i++)
        {
            sum += adc1_get_raw(ADC1_CHANNEL_0);
        }
        int batt_adc = sum / 100;

        sum = 0;
        for (int i = 0; i < 100; i++)
        {
            sum += adc1_get_raw(ADC1_CHANNEL_1);
        }
        int sys_adc = sum / 100;

        double volt_adc = adc1_get_raw(ADC1_CHANNEL_2);
        // float current1 = (0.0221 * batt_adc) - 0.3574;
        double current1 = std::round(((0.02 * batt_adc) + 0.2) * 100) / 100;
        double current2 = std::round(0.0009 * sys_adc * 100) / 100;

        // float current2 = (0.0007 * adc2) + 0.174;
        double batt_voltage = (volt_adc / 4096) * 8.27;
        int batt_voltage_int = batt_voltage * 100;
        printf("Batt Voltage: %d\n", batt_voltage_int);
        batt_voltage = batt_voltage_int / 100.0;
        printf("Batt Voltage: %f\n", batt_voltage);

        //  Write data
            Json::Value batt_data, sys_data;

        batt_data["Current"] = current1;
        batt_data["Voltage"] = batt_voltage;
        db.putData("Power/Battery", batt_data);
        sys_data["Current"] = current2;
        sys_data["Power"] = std::round(current2 * 5.0 * 100) / 100;
        db.putData("Power/System", sys_data);
        printf("Data uploaded\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }

    // Read data
    // Json::Value root = db.getData("/Sensors"); // retrieve person3 from database, set it to "" to get entire database
    // ESP_LOGI("MAIN", "Humidity: %s", root["Humidity"].toStyledString().c_str());
    // ESP_LOGI("MAIN", "Temperature: %s", root["Temperature"].toStyledString().c_str());
    // ESP_LOGI("MAIN", "Moisture: %s", root["Moisture"].toStyledString().c_str());
    // printf("Retrieved data\n");
}
