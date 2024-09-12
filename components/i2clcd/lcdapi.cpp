#include "lcdapi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
#include <esp32/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S2
#include <esp32s2/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S3
#include <esp32s3/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32C3
#include <esp32c3/rom/ets_sys.h>
#else
#error "Unsupported ESP chip"
#endif

LcdApi::LcdApi(int num_lines, int num_columns)
{
    m_num_lines = num_lines > 4 ? 4 : num_lines;
    m_num_columns = num_columns > 40 ? 40 : num_columns;
    m_cursor_x = 0;
    m_cursor_y = 0;
    m_implied_newline = false;
    m_backlight = true;
}

LcdApi::~LcdApi()
{
}

/// @brief Runs the end of the i2c lcd initialization.
void LcdApi::postInit()
{
    display_off();
    backlight_on();
    clear();
    _hal_write_command(LCD_ENTRY_MODE | LCD_ENTRY_INC);
    hide_cursor();
    display_on();
}

/// @brief Clears the LCD display and moves the cursor to the top left corner.
void LcdApi::clear()
{
    _hal_write_command(LCD_CLR);
    _hal_write_command(LCD_HOME);
    m_cursor_x = 0;
    m_cursor_y = 0;
}

/// @brief Causes the cursor to be hidden.
void LcdApi::hide_cursor()
{
    _hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
}

/// @brief Turns on the cursor, and makes it blink.
void LcdApi::blink_cursor_on()
{
    _hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY |
                       LCD_ON_CURSOR | LCD_ON_BLINK);
}

/// @brief Turns on the cursor, and makes it no blink (i.e. be solid).
void LcdApi::blink_cursor_off()
{
    _hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY |
                       LCD_ON_CURSOR);
}

/// @brief Turns on (i.e. unblanks) the LCD.
void LcdApi::display_on()
{
    _hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
}

/// @brief Turns off (i.e. blanks) the LCD.
void LcdApi::display_off()
{
    _hal_write_command(LCD_ON_CTRL);
}

/// @brief Turns the backlight on.
void LcdApi::backlight_on()
{
    // This isn't really an LCD command, but some modules have backlight
    // controls, so this allows the hal to pass through the command.
    m_backlight = true;
    _hal_backlight_on();
}

/// @brief Turns the backlight off.
void LcdApi::backlight_off()
{
    //  This isn't really an LCD command, but some modules have backlight
    //  controls, so this allows the hal to pass through the command.
    m_backlight = false;
    _hal_backlight_off();
}

/// @brief Moves the cursor position to the indicated position. The cursor
///        position is zero based (i.e. cursor_x == 0 indicates first column).
/// @param cursor_x
/// @param cursor_y
void LcdApi::move_to(const int cursor_x, const int cursor_y)
{
    m_cursor_x = cursor_x;
    m_cursor_y = cursor_y;
    uint16_t addr = cursor_x & 0x3f;
    if (cursor_y & 1)
    {
        addr += 0x40; // # Lines 1 & 3 add 0x40
    }
    if (cursor_y & 2)
    { //    # Lines 2 & 3 add number of columns
        addr += m_num_columns;
    }
    _hal_write_command(LCD_DDRAM | addr);
}

/// @brief Writes the indicated character to the LCD at the current cursor
///        position, and advances the cursor by one position.
/// @param ch ascii character
void LcdApi::putchar(const char ch)
{
    if (ch == '\n')
    {
        if (!m_implied_newline)
        {
            // implied_newline means we advanced due to a wraparound,
            // so if we get a newline right after that we ignore it.
            m_cursor_x = m_num_columns;
        }
    }
    else
    {
        _hal_write_data(ch);
        m_cursor_x += 1;
    }

    if (m_cursor_x >= m_num_columns)
    {
        m_cursor_x = 0;
        m_cursor_y += 1;
        m_implied_newline = (ch != '\n');
    }

    if (m_cursor_y >= m_num_lines)
        m_cursor_y = 0;
    move_to(m_cursor_x, m_cursor_y);
}

/// @brief Write the indicated string to the LCD at the current cursor
///        position and advances the cursor position appropriately.
/// @param s string to be written
void LcdApi::putstr(const char *s)
{
    for (int i = 0; s[i]; ++i)
    {
        putchar(s[i]);
    }
}

/// @brief Write a character to one of the 8 CGRAM locations, available
///        as chr(0) through chr(7).
/// @param location
/// @param charmap
void LcdApi::custom_char(const int location, const uint8_t charmap[8])
{
    _hal_write_command(LCD_CGRAM | ((location & 0x7) << 3));
    _hal_sleep_us(40);
    for (int i = 0; i < 8; ++i)
    {
        _hal_write_data(charmap[i]);
        _hal_sleep_us(40);
    }
    move_to(m_cursor_x, m_cursor_y);
}

/// @brief Sleep for some time (given in microseconds).
/// @param usecs microseconds delay
void LcdApi::_hal_sleep_us(const uint32_t usecs)
{
    ets_delay_us(usecs);
}
