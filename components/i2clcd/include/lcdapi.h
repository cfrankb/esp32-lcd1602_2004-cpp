#include "ilcdapi.h"

class LcdApi : public ILcdApi
{
public:
    LcdApi(int num_lines, int num_columns);
    ~LcdApi();

    void clear();
    void hide_cursor();
    void blink_cursor_on();
    void blink_cursor_off();
    void display_on();
    void display_off();
    void backlight_on();
    void backlight_off();
    void move_to(int cursor_x, int cursor_y);
    void putchar(char ch);
    void putstr(const char *s);
    void custom_char(int location, uint8_t charmap[8]);

    enum
    {
        LCD_CLR = 0x01,         //   DB0: clear display
        LCD_HOME = 0x02,        //   DB1: return to home position
        LCD_ENTRY_MODE = 0x04,  //   DB2: set entry mode
        LCD_ENTRY_INC = 0x02,   //   --DB1: increment
        LCD_ENTRY_SHIFT = 0x01, //   --DB0: shift

        LCD_ON_CTRL = 0x08,    //    DB3: turn lcd/cursor on
        LCD_ON_DISPLAY = 0x04, //    --DB2: turn display on
        LCD_ON_CURSOR = 0x02,  //    --DB1: turn cursor on
        LCD_ON_BLINK = 0x01,   //    --DB0: blinking cursor
        LCD_MOVE = 0x10,       //    DB4: move cursor/display
        LCD_MOVE_DISP = 0x08,  //    --DB3: move display (0-> move cursor)
        LCD_MOVE_RIGHT = 0x04, //    --DB2: move right (0-> left)

        LCD_FUNCTION = 0x20,        //  DB5: function set
        LCD_FUNCTION_8BIT = 0x10,   //  --DB4: set 8BIT mode (0->4BIT mode)
        LCD_FUNCTION_2LINES = 0x08, //  --DB3: two lines (0->one line)
        LCD_FUNCTION_10DOTS = 0x04, //  --DB2: 5x10 font (0->5x7 font)
        LCD_FUNCTION_RESET = 0x30,  //  See "Initializing by Instruction" section

        LCD_CGRAM = 0x40, //  DB6: set CG RAM address
        LCD_DDRAM = 0x80, //  DB7: set DD RAM address
        LCD_RS_CMD = 0,   //
        LCD_RS_DATA = 1,  //

        LCD_RW_WRITE = 0, //
        LCD_RW_READ = 1,  //
    };

protected:
    int m_num_lines;
    int m_num_columns;
    int m_cursor_x;
    int m_cursor_y;
    bool m_backlight;
    bool m_implied_newline;

    void postInit();
    void _hal_sleep_us(uint32_t usecs);

private:
};