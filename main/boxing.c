#include <stdio.h>
#include <adxl345.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#define SCL_PIN 22    /*!< gpio number for i2c clock */
#define SDA_PIN 21    /*!< gpio number for i2c data */
static const char *TAG = "boxing";

static void sensorTask(void *pvParameters) {
    ESP_LOGI(TAG,"sensor task started\n");

    initAccelerometer(SCL_PIN, SDA_PIN);
    ESP_LOGI(TAG,"accelerometer started");

    while (1) {
        ESP_LOGI(TAG,"inside while");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

}

void app_main() {
    ESP_LOGI("system","system inited");

    xTaskCreate(&sensorTask,	//pvTaskCode
                "sensorTask",//pcName
                4096,//usStackDepth
                NULL,//pvParameters
                4,//uxPriority
                NULL//pxCreatedTask
    );

}