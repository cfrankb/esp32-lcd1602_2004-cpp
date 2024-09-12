#include "i2clcd.h"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PIN_NUM_SDA GPIO_NUM_21
#define PIN_NUM_SCL GPIO_NUM_22
#define I2C_ADDR 0x27 // 0x3f // 0x27
#define I2C_MASTER_FREQ_HZ 10000
#define I2C_TOOL_TIMEOUT_VALUE_MS 50
#define _write(__data) _writeTo(__data, sizeof(__data))

static const char TAG[] = "i2clcd";

/// @brief Create i2clcd interface
/// @param sda gpio pin for sda
/// @param scl gpio pin for scl
/// @param i2cAddr bus address (e.g 0x27 most common)
/// @param i2cFreq typically 10000hz
/// @param num_lines number of lines (2 or 4)
/// @param num_columns number columns (16 or 20)
I2CLCD::I2CLCD(const gpio_num_t sda,
               const gpio_num_t scl,
               const uint8_t i2cAddr,
               const uint32_t i2cFreq,
               const int num_lines,
               const int num_columns) : LcdApi(num_lines, num_columns)
{
    _init(sda, scl);
    _addDevice(i2cAddr, i2cFreq);
    const uint8_t data1[]{0};
    _writeTo(data1, sizeof(data1));
    _delay(20); // Allow LCD time to powerup

    // Send reset 3 times
    _hal_write_init_nibble(LCD_FUNCTION_RESET);
    _delay(5); // Need to delay at least 4.1 msec
    _hal_write_init_nibble(LCD_FUNCTION_RESET);
    _delay(1);
    _hal_write_init_nibble(LCD_FUNCTION_RESET);
    _delay(1);
    // Put LCD into 4-bit mode
    _hal_write_init_nibble(LCD_FUNCTION);
    _delay(1);
    LcdApi::postInit();
    const uint8_t cmd = num_lines > 1 ? LCD_FUNCTION | LCD_FUNCTION_2LINES : LCD_FUNCTION;
    _hal_write_command(cmd);
}

I2CLCD::~I2CLCD()
{
    _removeDevice();
}

/// @brief  Writes an initialization nibble to the LCD.
/// @param nibble
void I2CLCD::_hal_write_init_nibble(const uint8_t nibble)
{
    // This particular function is only used during initialization.
    const uint8_t byte = ((nibble >> 4) & 0x0f) << SHIFT_DATA;
    const uint8_t data1[]{static_cast<uint8_t>(byte | MASK_E)};
    _write(data1);
    const uint8_t data2[]{byte};
    _write(data2);
}

/// @brief Write a command to the LCD. Data is latched on the falling edge of E.
/// @param cmd
void I2CLCD::_hal_write_command(const uint8_t cmd)
{
    uint8_t byte = ((m_backlight << SHIFT_BACKLIGHT) |
                    (((cmd >> 4) & 0x0f) << SHIFT_DATA));
    const uint8_t data1[]{static_cast<uint8_t>(byte | MASK_E)};
    _write(data1);
    const uint8_t data2[]{byte};
    _write(data2);

    byte = ((m_backlight << SHIFT_BACKLIGHT) |
            ((cmd & 0x0f) << SHIFT_DATA));
    const uint8_t data3[]{static_cast<uint8_t>(byte | MASK_E)};
    _write(data3);
    const uint8_t data4[]{byte};
    _write(data4);
    if (cmd <= 3)
    {
        // The home and clear commands require a worst case delay of 4.1 msec
        _delay(5);
    }
}

/// @brief Write data to the LCD. Data is latched on the falling edge of E.
/// @param data
void I2CLCD::_hal_write_data(const uint8_t data)
{
    uint8_t byte = (MASK_RS |
                    (m_backlight << SHIFT_BACKLIGHT) |
                    (((data >> 4) & 0x0f) << SHIFT_DATA));
    const uint8_t data1[]{static_cast<uint8_t>(byte | MASK_E)};
    _write(data1);
    const uint8_t data2[]{byte};
    _write(data2);

    byte = (MASK_RS |
            (m_backlight << SHIFT_BACKLIGHT) |
            ((data & 0x0f) << SHIFT_DATA));
    const uint8_t data3[]{static_cast<uint8_t>(byte | MASK_E)};
    _write(data3);
    const uint8_t data4[]{byte};
    _write(data4);
}

/// @brief Allows the hal layer to turn the backlight on
void I2CLCD::_hal_backlight_on()
{
    const uint8_t data1[]{1 << SHIFT_BACKLIGHT};
    _write(data1);
}

/// @brief Allows the hal layer to turn the backlight off
void I2CLCD::_hal_backlight_off()
{
    const uint8_t data1[]{0};
    _write(data1);
}

/// @brief Delay execution for a few miliseconds
/// @param msec
void I2CLCD::_delay(const int msec)
{
    vTaskDelay(msec / portTICK_PERIOD_MS);
}

/// @brief Wrote to 12c slave device
/// @param data data to be written
/// @param size number of bytes
/// @return true if successful
bool I2CLCD::_writeTo(const uint8_t *data, const size_t size)
{
    const esp_err_t ret = i2c_master_transmit(m_dev_handle, data, size, I2C_TOOL_TIMEOUT_VALUE_MS);
    if (ret == ESP_OK)
    {
        return true;
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(TAG, "Bus is busy");
        return false;
    }
    else
    {
        ESP_LOGW(TAG, "Write Failed");
        return false;
    }
}

/// @brief add 12c slave device to the bus
/// @return true if succesful
bool I2CLCD::_addDevice(const uint8_t i2cAddr,
                        const uint32_t i2cFreq)
{
    i2c_device_config_t i2c_dev_conf = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2cAddr, // I2C_ADDR,
        .scl_speed_hz = i2cFreq,   // I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 0,
        .flags{.disable_ack_check = 0},
    };
    if (i2c_master_bus_add_device(m_i2c_bus, &i2c_dev_conf, &m_dev_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Adding device has failed");
        return false;
    }
    ESP_LOGI(TAG, "Adding device has not failed");
    return true;
}

/// @brief remove 12c slave device to the bus
/// @return  true if succesful
bool I2CLCD::_removeDevice()
{
    if (i2c_master_bus_rm_device(m_dev_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Removing device has failed");
        return false;
    }
    ESP_LOGI(TAG, "Removing device has not failed");
    return true;
}

/// @brief initialize 12c bus master
void I2CLCD::_init(const gpio_num_t sda, const gpio_num_t scl)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = -1,    // I2C_NUM_0,
        .sda_io_num = sda, // PIN_NUM_SDA,
        .scl_io_num = scl, // PIN_NUM_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags{.enable_internal_pullup = false},
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &m_i2c_bus));
}
