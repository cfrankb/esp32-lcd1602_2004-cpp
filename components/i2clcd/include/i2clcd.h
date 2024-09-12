#ifndef __i2CLCD__H
#define __i2CLCD__H

#include <driver/i2c_master.h>
#include "lcdapi.h"

class I2CLCD : public LcdApi
{
public:
    I2CLCD(const gpio_num_t sda,
           const gpio_num_t scl,
           const uint8_t i2cAddr,
           const uint32_t i2cFreq,
           const int num_lines,
           const int num_columns);
    ~I2CLCD();

private:
    enum : uint8_t
    {
        // # PCF8574 pin definitions
        MASK_RS = 0x01,      // # P0
        MASK_RW = 0x02,      // # P1
        MASK_E = 0x04,       // # P2
        SHIFT_BACKLIGHT = 3, // # P3
        SHIFT_DATA = 4,      // # P4-P7
    };

    void _init(const gpio_num_t sda, const gpio_num_t scl);
    bool _addDevice(const uint8_t i2cAddr,
                    const uint32_t i2cFreq);
    bool _removeDevice();
    bool _writeTo(const uint8_t *data, const size_t size);
    void _delay(const int msec);
    void _hal_write_command(const uint8_t cmd) override;
    void _hal_write_data(const uint8_t data);
    void _hal_write_init_nibble(const uint8_t nibble);
    void _hal_backlight_on() override;
    void _hal_backlight_off() override;
    i2c_master_bus_handle_t m_i2c_bus = nullptr;
    i2c_master_dev_handle_t m_dev_handle = nullptr;
};

#endif