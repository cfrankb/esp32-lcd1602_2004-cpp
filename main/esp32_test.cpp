#include <stdio.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <rom/ets_sys.h>
#include "i2clcd.h"

#define PIN_NUM_SDA GPIO_NUM_21
#define PIN_NUM_SCL GPIO_NUM_22
#define I2C_ADDR 0x27 //  0x27, 0x3f etc
#define I2C_MASTER_FREQ_HZ 10000
#define I2C_TOOL_TIMEOUT_VALUE_MS 50
#define NUM_ROWS 2
#define NUM_COLS 16

static const char TAG[] = "main";

extern "C" void app_main(void)
{
    I2CLCD i2clcd(PIN_NUM_SDA, PIN_NUM_SCL, I2C_ADDR, I2C_MASTER_FREQ_HZ, NUM_ROWS, NUM_COLS);
    while (1)
    {
        i2clcd.putstr("I2C LCD Tutorial");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        i2clcd.clear();
        i2clcd.putstr("Lets Count 0-10!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        i2clcd.clear();
        for (int i = 0; i < 11; ++i)
        {
            char tmp[5];
            sprintf(tmp, "%d", i);
            i2clcd.putstr(tmp);
            i2clcd.backlight_on();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            i2clcd.backlight_off();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            i2clcd.clear();
        }
        i2clcd.backlight_on();
    }
}
