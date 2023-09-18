/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_bsp_printf.h"
#include "py32f0xx_hal.h"
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
#define FLASH_USER_START_ADDR 0x08004000
#define FLASH_USER_START_ADDR_REDUNDANT 0x08003000
#define SETTING_SIZE 64
#define NUM_BUTTONS 12
#define BRIGHTNESS_INDEX 13
#define SPEED_INDEX 14
#define COLOR_INDEX 15
#define NUM_COLORS 13
#define RAINBOW_MODE_INDEX 16
#define FLASHVALID_INDEX 17
#define VALID_FLAG 0xaa
#define MAX_BRIGHTNESS 0x0F
#define NUM_SPEED_SETTINGS 0x08

#define BAUD_RATE 9600
#define CALENDAR_UART_TX GPIO_PIN_1
#define CALENDAR_UART_RX GPIO_PIN_0

/* Private variables ---------------------------------------------------------*/
uint32_t Settings[SETTING_SIZE];
uint32_t Button_held[NUM_BUTTONS];
bool flash_needs_update = false;
const uint16_t Colors[NUM_COLORS] = {0xf00, 0xb40, 0x880, 0x4b0, 0x0f0, 0x0b4, 0x088, 0x04b,
                                     0x00f, 0x40b, 0x808, 0xb04, 0x555};
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_LedConfig(void);
static void APP_FlashErase(uint8_t page);
static void APP_FlashProgram(uint8_t page);
static void APP_ErrorHandler(void);
static void init_buttons(void);
static void button_helper(uint8_t id, GPIO_PinState status);
static void check_buttons(void);
static void check_button_combo(void);

inline void zero()
{
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
}
inline void one()
{
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BSRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
    GPIOF->BRR = (uint32_t)GPIO_PIN_0;
}

inline void send_byte(uint8_t byte)
{
    for (int b = 0; b < 8; b++) {
        if (byte & 0x80) {
            one();
        } else {
            zero();
        }
        byte = byte << 1;
    }
}

void update_flash()
{
    // Primary
    HAL_Delay(1);
    HAL_FLASH_Unlock();
    HAL_Delay(1);
    APP_FlashErase(0);
    HAL_Delay(1);
    APP_FlashProgram(0);
    HAL_Delay(1);
    HAL_FLASH_Lock();
    HAL_Delay(1);
    // Redundant
    HAL_FLASH_Unlock();
    HAL_Delay(1);
    APP_FlashErase(1);
    HAL_Delay(1);
    APP_FlashProgram(1);
    HAL_Delay(1);
    HAL_FLASH_Lock();
    HAL_Delay(1);

    flash_needs_update = false;
}
int main(void)
{
    HAL_Init();

    APP_SystemClockConfig();
    // XXX: This conflicts with uart pins
    //APP_LedConfig();
    init_buttons();

    // Copy flash into the buffer
    uint32_t *flash_program_start = (uint32_t *)FLASH_USER_START_ADDR;
    for (int x = 0; x < SETTING_SIZE; x++) {
        Settings[x] = flash_program_start[x];
    }
    // Currupt flash! Read the redundent page
    if (Settings[FLASHVALID_INDEX] != VALID_FLAG) {
        uint32_t *flash_program_start_two = (uint32_t *)FLASH_USER_START_ADDR_REDUNDANT;
        for (int x = 0; x < SETTING_SIZE; x++) {
            Settings[x] = flash_program_start_two[x];
        }
    }
    // Either way should be "valid" now
    Settings[FLASHVALID_INDEX] = VALID_FLAG;

    int count = 0;
    int update_count_count = 0;
    while (1) {
        // Always updated this counter
        update_count_count++;
        uint8_t brightness = ((uint8_t)Settings[BRIGHTNESS_INDEX]) & 0x0f;
        if (brightness == 0) {
            brightness = 0x01;
        }
        check_buttons();
        if (flash_needs_update) {
            update_flash();
        }
        uint32_t speed = 1 << ((uint8_t)Settings[SPEED_INDEX]);
        HAL_Delay(5);
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // This should be outside the loop... Let's see if it's quick enough anyway

        // The speed depends on brightness as there are more or less values to go through.
        uint8_t g = 0;
        uint8_t r = 0;
        uint8_t b = 0;
        if (Settings[RAINBOW_MODE_INDEX] == 1) {
            if (update_count_count > 20 * speed / brightness) {
                update_count_count = 0;
                count = count + 1;
                count = count % (brightness * 3);
            }
            // divide by 0xf by shifting
            uint8_t grb = count / brightness;
            uint8_t val = count % brightness;
            if (grb == 0) {
                r = val;
                g = 0;
                b = brightness - val;
            } else if (grb == 1) {
                g = val;
                b = 0;
                r = brightness - val;
            } else if (grb == 2) {
                b = val;
                r = 0;
                g = brightness - val;
            }
        } else {

            uint32_t color_index = Settings[COLOR_INDEX];
            uint32_t color = Colors[color_index];
            uint8_t color_reduction = MAX_BRIGHTNESS - brightness;
            r = ((uint8_t)((color & 0xf00) >> 8));
            g = ((uint8_t)((color & 0x0f0) >> 4));
            b = ((uint8_t)((color & 0x00f) >> 0));
            // Full scale single color has full brightness control
            if (color_index == 0 ||
                color_index == 4 ||
                color_index == 8) {
                r = r - color_reduction;
                g = g - color_reduction;
                b = b - color_reduction;
            }
            // Half scale two even color has half brightness control
            else if (color_index == 2 ||
                     color_index == 6 ||
                     color_index == 10) {
                r = r - (color_reduction / 2);
                g = g - (color_reduction / 2);
                b = b - (color_reduction / 2);
            }
            // White is a special case and gets 0x0f/0x05 controls
            else if (color_index == 12) {
                r = r - (color_reduction / 3);
                g = g - (color_reduction / 3);
                b = b - (color_reduction / 3);
            }
            // Two colors, a 1:3 ratio. Needs special treatment
            else if (color_index == 1 ||
                     color_index == 3 ||
                     color_index == 5 ||
                     color_index == 7 ||
                     color_index == 9 ||
                     color_index == 11) {
                uint8_t high = (color_reduction / 4) * 3;
                uint8_t low = (color_reduction / 4) * 1;
                if (color_index == 1) {
                    r = r - high;
                    g = g - low;
                } else if (color_index == 3) {
                    g = g - high;
                    r = r - low;
                } else if (color_index == 5) {
                    g = g - high;
                    b = b - low;
                } else if (color_index == 7) {
                    b = b - high;
                    g = g - low;
                } else if (color_index == 9) {
                    b = b - high;
                    r = r - low;
                } else if (color_index == 11) {
                    r = r - high;
                    b = b - low;
                }
            }
            if (r > 0x0f) {
                r = 0;
            }
            if (g > 0x0f) {
                g = 0;
            }
            if (b > 0x0f) {
                b = 0;
            }
        }
        // The timing is "sensitive" but that's more per bit than it is
        // in total. It takes 6000ns to latch so there's actually
        // plenty of time between bits
        // Make sure optimizations isn't changing the order of things
        // Check disassembling if something seems wonky.
        // XXX: This conflicts with uart pins
        /*for (int x = 0; x < NUM_BUTTONS; x++) {
            if (Settings[x] == 0) {
                send_byte(g);
                send_byte(r);
                send_byte(b);
            } else {
                send_byte(0);
                send_byte(0);
                send_byte(0);
            }
        }*/
    }
}

static void APP_LedConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}
static void APP_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI
#if defined(RCC_LSE_SUPPORT)
                                       | RCC_OSCILLATORTYPE_LSE
#endif
                                       | RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* HSI ON */
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* Set HSI clock 24MHz */
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* No division */
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         /* OFF */
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* OFF */
#if defined(RCC_LSE_SUPPORT)
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF; /* OFF */
#endif
#if defined(RCC_LSE_SUPPORT)
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF; /* OFF */
#endif

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        APP_ErrorHandler();
    }

    /* Reinitialize AHB,APB bus clock */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; /* Select HSI as SYSCLK source */
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* APH clock, no division */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      /* APB clock, no division */

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        APP_ErrorHandler();
    }
}

static void APP_FlashErase(uint8_t page)
{
    uint32_t SECTORError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    // Erase type = sector
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGEERASE;
    // Erase address start
    if (page == 0) {
        EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
    } else {
        EraseInitStruct.PageAddress = FLASH_USER_START_ADDR_REDUNDANT;
    }
    // Number of sectors
    EraseInitStruct.NbPages = 1;
    // Erase
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
        APP_ErrorHandler();
    }
}
static void APP_FlashProgram(uint8_t page)
{
    uint32_t flash_program_start = FLASH_USER_START_ADDR;
    uint32_t flash_program_end = FLASH_USER_START_ADDR + sizeof(Settings);
    if (page == 0) {
        flash_program_start = FLASH_USER_START_ADDR;
        flash_program_end = FLASH_USER_START_ADDR + sizeof(Settings);
    } else {
        flash_program_start = FLASH_USER_START_ADDR_REDUNDANT;
        flash_program_end = FLASH_USER_START_ADDR_REDUNDANT + sizeof(Settings);
    }
    uint32_t *src = (uint32_t *)Settings;

    while (flash_program_start < flash_program_end) {
        // Write to flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, flash_program_start, src) == HAL_OK) {
            // Move flash point to next page
            flash_program_start += FLASH_PAGE_SIZE;
            // Move data point
            src += FLASH_PAGE_SIZE / 4;
        }
    }
}
void APP_ErrorHandler(void)
{
    while (1)
        ;
}

static void init_buttons(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void button_helper(uint8_t id, GPIO_PinState status)
{
    if (status == GPIO_PIN_RESET) {
        if (Button_held[id] != 1) {
            Button_held[id] = 1;
            if (Settings[id] == 0) {
                Settings[id] = 1;
            } else {
                Settings[id] = 0;
            }
            flash_needs_update = true;
        }
    } else {
        Button_held[id] = 0;
    }
}
void check_buttons()
{
    button_helper(0, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3));
    button_helper(1, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5));
    button_helper(2, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6));
    button_helper(3, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0));
    button_helper(4, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7));
    button_helper(5, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0));
    button_helper(6, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2));
    button_helper(7, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1));
    button_helper(8, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3));
    button_helper(9, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2));
    button_helper(10, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
    button_helper(11, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4));

    check_button_combo();
}

bool brightness_up_held = false;
bool brightness_down_held = false;
bool color_up_held = false;
bool color_down_held = false;
bool speed_up_held = false;
bool speed_down_held = false;
bool rainbow_mode_held = false;
bool erase_extra_settings_held = false;
bool clear_all_buttons_held = false;
void check_button_combo()
{
    // Brightness changing
    // Right is up
    if (Button_held[0] == 1 && Button_held[8] == 1) {
        if (brightness_up_held == false) {
            brightness_up_held = true;
            Settings[BRIGHTNESS_INDEX] = (Settings[BRIGHTNESS_INDEX] + 1) % (MAX_BRIGHTNESS + 1);
            flash_needs_update = true;
        }
    } else {
        brightness_up_held = false;
    }
    // Left is down
    if (Button_held[0] == 1 && Button_held[7] == 1) {
        if (brightness_down_held == false) {
            brightness_down_held = true;
            Settings[BRIGHTNESS_INDEX] = (Settings[BRIGHTNESS_INDEX] - 1);
            if (Settings[BRIGHTNESS_INDEX] > MAX_BRIGHTNESS || Settings[BRIGHTNESS_INDEX] < 0) {
                Settings[BRIGHTNESS_INDEX] = MAX_BRIGHTNESS;
            }
            flash_needs_update = true;
        }
    } else {
        brightness_down_held = false;
    }
    // Color changing
    // Right row is up
    if (Button_held[0] == 1 && Button_held[9] == 1) {
        if (color_up_held == false) {
            color_up_held = true;
            Settings[COLOR_INDEX] = (Settings[COLOR_INDEX] + 1) % NUM_COLORS;
            flash_needs_update = true;
        }
    } else {
        color_up_held = false;
    }
    // Left is down
    if (Button_held[0] == 1 && Button_held[6] == 1) {
        if (color_down_held == false) {
            color_down_held = true;
            Settings[COLOR_INDEX] = Settings[COLOR_INDEX] - 1;
            if (Settings[COLOR_INDEX] > NUM_COLORS || Settings[COLOR_INDEX] < 0) {
                Settings[COLOR_INDEX] = NUM_COLORS - 1;
            }

            flash_needs_update = true;
        }
    } else {
        color_down_held = false;
    }

    // Rainbow speed settings
    // Right is faster
    if (Button_held[0] == 1 && Button_held[10] == 1) {
        if (speed_up_held == false) {
            speed_up_held = true;
            Settings[SPEED_INDEX] = Settings[SPEED_INDEX] - 1;
            if (Settings[SPEED_INDEX] > NUM_SPEED_SETTINGS || Settings[SPEED_INDEX] < 0) {
                Settings[SPEED_INDEX] = NUM_SPEED_SETTINGS;
            }
            flash_needs_update = true;
        }
    } else {
        speed_up_held = false;
    }
    // Left is slower
    if (Button_held[0] == 1 && Button_held[5] == 1) {
        if (speed_down_held == false) {
            speed_down_held = true;
            Settings[SPEED_INDEX] = (Settings[SPEED_INDEX] + 1);
            if (Settings[SPEED_INDEX] >= NUM_SPEED_SETTINGS) {
                Settings[SPEED_INDEX] = 0;
            }
            flash_needs_update = true;
        }
    } else {
        speed_down_held = false;
    }

    if (Button_held[0] == 1 && Button_held[11] == 1) {
        if (rainbow_mode_held == false) {
            rainbow_mode_held = true;
            if (Settings[RAINBOW_MODE_INDEX] == 0) {
                Settings[RAINBOW_MODE_INDEX] = 1;
            } else {
                Settings[RAINBOW_MODE_INDEX] = 0;
            }
            flash_needs_update = true;
        }
    } else {
        rainbow_mode_held = false;
    }
    // 4 corners held is a settings reset!
    if (Button_held[0] == 1 && Button_held[3] == 1 && Button_held[8] == 1 && Button_held[11] == 1) {
        if (erase_extra_settings_held == false) {
            erase_extra_settings_held = true;
            // Skip past the buttons!
            for (int x = NUM_BUTTONS; x < SETTING_SIZE; x++) {
                Settings[x] = 0;
            }
            Settings[FLASHVALID_INDEX] = VALID_FLAG;
            flash_needs_update = true;
        }
    } else {
        erase_extra_settings_held = false;
    }

    if (Button_held[0] == 1 && Button_held[3] == 1 && Button_held[8] == 1 && Button_held[10] == 1) {
        if (clear_all_buttons_held == false) {
            clear_all_buttons_held = true;
            for (int x = 0; x < NUM_BUTTONS; x++) {
                Settings[x] = 1;
            }
            flash_needs_update = true;
        }
    } else {
        clear_all_buttons_held = false;
    }
}

void uart_init()
{
    GPIO_InitTypeDef GPIO_InitStruct_tx;

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct_tx.Pin = CALENDAR_UART_TX;
    GPIO_InitStruct_tx.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_tx.Pull = GPIO_PULLUP;
    GPIO_InitStruct_tx.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_tx);
    HAL_GPIO_WritePin(GPIOF, CALENDAR_UART_TX, GPIO_PIN_SET); // Initialize TX pin high (idle state)

    GPIO_InitTypeDef GPIO_InitStruct_rx;

    GPIO_InitStruct_rx.Pin = CALENDAR_UART_RX;
    GPIO_InitStruct_rx.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct_rx.Pull = GPIO_NOPULL;
    GPIO_InitStruct_rx.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_rx);
}

void uart_send_byte(uint8_t data)
{
    // Start bit (low)
    HAL_GPIO_WritePin(GPIOF, CALENDAR_UART_TX, GPIO_PIN_RESET);
    sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate

    // Send data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        if ((data >> i) & 1) {
            HAL_GPIO_WritePin(GPIOF, CALENDAR_UART_TX, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOF, CALENDAR_UART_TX, GPIO_PIN_RESET);
        }

        sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
    }

    // Stop bit (high)
    HAL_GPIO_WritePin(GPIOF, CALENDAR_UART_TX, GPIO_PIN_SET);
    sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
}

// TODO: Make this non-blocking
uint8_t uart_receive_byte()
{
    uint8_t data = 0;

    // Wait for start bit
    while (HAL_GPIO_ReadPin(GPIOF, CALENDAR_UART_RX)) {
    }

    // Wait for half a bit period and sample in the middle of the bit
    sleep_us(1000000 / (2 * BAUD_RATE));

    // Receive data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        data |= (HAL_GPIO_ReadPin(GPIOF, CALENDAR_UART_RX) << i);
        sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
    }

    // Wait for stop bit
    while (!HAL_GPIO_ReadPin(GPIOF, CALENDAR_UART_RX)) {
    }

    return data;
}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    while (1) {
    }
}
#endif /* USE_FULL_ASSERT */
