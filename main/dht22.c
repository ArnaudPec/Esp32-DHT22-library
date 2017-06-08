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
 *
 *    Short note on data coding scheme:
 *
 *    Low level of ~50us followed by high level
 *    high level duration determines the symbol:
 *         - high level of ~26us is a 0 whereas ~70us is 1
 *
 *    It is therefore possible to deduce the symbol by comparing
 *    high/low level duration.
 *
 *                  26us           70us
 *        --+     +------+     +---------+
 *          |     |      |     |         |
 *          |     |      |     |         |
 *          |     |      |     |         |
 *          +-----+      +-----+         +---
 *
 *                   "0"           "1"
 *
 *    See DHT22 datasheet for more in-depth informations.
 *
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
/*#include "dht22.h"*/

int DHT_GPIO = 26;
int timeout;

void pulse_init()
{
    gpio_set_direction(DHT_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(DHT_GPIO, 0);
    ets_delay_us(20000);
    gpio_set_level(DHT_GPIO, 1);
    ets_delay_us(40);

    gpio_set_direction(DHT_GPIO, GPIO_MODE_INPUT);
}

int checksum(int *data)
{
    return (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF));
}

void set_DHT_pin(int pin){
    DHT_GPIO = pin;
}

int wait_change_level(int level, int time)
{
    int cpt = 0;
    while (gpio_get_level(DHT_GPIO) == level) {
       if (cpt > time) {
           timeout = 1;
       }
       ++cpt;
       ets_delay_us(1);
    }
    return cpt;
}

float * get_data()
{
    int i, val[80] = {0}, bytes[5] = {0}, data[2]={0};
    char out[40] = {0};
    timeout = 0;

    portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

    portENTER_CRITICAL(&my_spinlock); // timing critical start
    {
        // Init sequence MCU side: pulse and wait
        pulse_init();
        ets_delay_us(10);

        // DHT22 sending init sequence(high/low/high)
        wait_change_level(1, 40);
        wait_change_level(0, 80);
        wait_change_level(1, 80);

        // And now the reading adventure begins
        for (i = 0; i < 80; i += 2) {

            val[i] = wait_change_level(0, 80);
            val[i+1] = wait_change_level(1, 80);
        }
    }
    portEXIT_CRITICAL(&my_spinlock); // timing critical end

    for (i = 2; i < 80; i+=2)
        out[i/2] = (val[i] < val[i+1]) ? 1 : 0;

    for (i = 0; i < 40; ++i) {
        bytes[i/8] <<= 1;
        bytes[i/8] |= out[i];
    }

    if(timeout)
        printf("%s\n", "Timeout error");
    else if(!checksum(bytes))
        printf("%s\n", "Checksum error");
    else
    {
        for (i = 0; i < 2; ++i)
            data[i] = (bytes[2*i] << 8) + bytes[2*i+1];

        printf("hum: %d\n", data[0]);
        printf("temp: %d\n", data[1]);
    }
    return 0;
}
void read_task(void *pvParameter)
{
    gpio_pad_select_gpio(DHT_GPIO);
    while (1) {
        get_data();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreate(&read_task, "read_task", 2048, NULL, 5, NULL);
}
