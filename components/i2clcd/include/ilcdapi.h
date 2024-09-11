#include "inttypes.h"

class ILcdApi
{

public:
    ILcdApi() {};
    ~ILcdApi() {};

protected:
    virtual void _hal_write_command(uint8_t cmd) = 0;
    virtual void _hal_write_data(uint8_t cmd) = 0;
    virtual void _hal_backlight_on() = 0;
    virtual void _hal_backlight_off() = 0;
};