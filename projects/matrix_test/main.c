#include "hal_gpio.h"
#include "hal_uart.h"
#include "bl702_glb.h"

#define COL_NUM 11
#define ROW_NUM 7

static const uint8_t matrix_cols[COL_NUM] = {20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31};
static const uint8_t matrix_rows[ROW_NUM] = {0, 1, 2, 3, 4, 5, 6};

static uint8_t states[2][COL_NUM] = {0};
static uint8_t flag = 0;

void init_keyboard_gpio(void)
{
    // Need to set col pins only, because row pins is set in bflb_platform_init(0)
    for (uint8_t col = 0; col < COL_NUM; ++col) {
        // unselect_col()
        gpio_write(matrix_cols[col], 1);
        gpio_set_mode(matrix_cols[col], GPIO_OUTPUT_PP_MODE);
    }
}

void scan_keyboard(uint8_t *states)
{
    for (uint8_t col = 0; col < COL_NUM; ++col) {
        states[col] = 0;

        // select_col()
        gpio_write(matrix_cols[col], 0);

        // matrix_output_select_delay()
        bflb_platform_delay_us(1);

        for (uint8_t row = 0; row < ROW_NUM; ++row) {
            // readPin(row_pins[row_index])
            if (gpio_read(matrix_rows[row]) == 0) {
                states[col] |= 1U << row;
            }
        }

        // unselect_col()
        gpio_write(matrix_cols[col], 1);

        // matrix_output_unselect_delay()
        bflb_platform_delay_us(30);
    }
}

bool detect_keyboard_changed(uint8_t *prev, uint8_t *curr)
{
    for (uint8_t col = 0; col < COL_NUM; ++col) {
        if (prev[col] != curr[col]) {
            return true;
        }
    }
    return false;
}

void print_keyboard_states(uint8_t *states)
{
    MSG("@");
    for (uint8_t col = 0; col < COL_NUM; ++col) {
        for (uint8_t row = 0; row < ROW_NUM; ++row) {
            if (states[col] & (1U << row)) {
                uint8_t keycode = col * ROW_NUM + row;
                MSG(" %d", keycode);
            }
        }
    }
    MSG("\r\n");
}

int main()
{
    GLB_Select_Internal_Flash();
    bflb_platform_init(0);

    init_keyboard_gpio();
    print_keyboard_states(states[flag]);
    flag ^= 1U;

    while (1) {
        scan_keyboard(states[flag]);

        if (detect_keyboard_changed(states[flag ^ 1U], states[flag])) {
            print_keyboard_states(states[flag]);
            flag ^= 1U;
        }
    }
}
