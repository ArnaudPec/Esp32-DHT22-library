/*
 * Simple library for DHT11/22 sensor
 *
 * Arnaud Pecoraro 2017
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "dht22.h"

void read_task(void *pvParameter)
{
    while (1) {
        printf("Humidity %1.2f\n", get_hum());
        printf("Temperature Celsius:  %1.2f\n", get_tempc());
        printf("Temperature Fahrenheit:  %1.2f\n\n", get_tempf());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    set_DHT_pin(26);
    xTaskCreate(&read_task, "read_task", 2048, NULL, 5, NULL);
}
