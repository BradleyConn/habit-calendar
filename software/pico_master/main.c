/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_bsp_printf.h"
#include "py32f0xx_hal.h"
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
#define CALENDAR_CHIP_ID 6
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

// 10200 works for 9600 baud
#define BAUD_RATE (10300)

#define CALENDAR_FRONT_UART_PIN GPIO_PIN_3
#define CALENDAR_FRONT_UART_MODULE GPIOA

#define CALENDAR_BACK_UART_PIN GPIO_PIN_1
#define CALENDAR_BACK_UART_MODULE GPIOF

typedef enum { front = 0,
               back = 1 } front_back;
typedef enum { tx = 0,
               rx = 1 } tx_rx;

/* Private variables ---------------------------------------------------------*/
uint32_t Settings[SETTING_SIZE];
uint32_t Button_held[NUM_BUTTONS];
bool flash_needs_update = false;
const uint16_t Colors[NUM_COLORS] = {0xf00, 0xb40, 0x880, 0x4b0, 0x0f0, 0x0b4, 0x088, 0x04b,
                                     0x00f, 0x40b, 0x808, 0xb04, 0x555};
bool timer_elapsed = false;
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
static void sleep_us(uint32_t us);
// Turn into front interface back interface
static void uart_init(front_back fb, tx_rx txrx);
static void uart_send_byte(front_back fb, uint8_t data);
static uint8_t uart_rx_byte(front_back fb);
// static bool uart_rx_byte_timeout(uint8_t *byte, uint32_t timeout_ms);
// static void lptim_init();

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
    APP_LedConfig();
    init_buttons();
    uart_init(front, tx);
    uart_init(back, rx);

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

    while (1) {
        check_buttons();
        if (flash_needs_update) {
            update_flash();
        }

        // TODO: Try a server response approach. Only do action if instructed - but break to check buttons
        //                      .... Wait a second if we poll more frequently than a human button push we don't need to break...
        //  Use half duplex uart approach!
        //  Wait on front uart query
        //  Forward to back uart
        //  Wait on back uart response
        //  Check button states
        //  Add to payload and forward to front uart

        // front_uart_init_rx();
        uart_init(front, rx);
        // val = front_uart_rx();
        uint8_t query_char = uart_rx_byte(front);
        // don't need val just trigger on anything
        // If it's not the last chip in the chain forward it
        // Otherwise just send the state
        uint8_t data = '*';
        if (CALENDAR_CHIP_ID != 7) {
            // back_uart_init_tx();
            uart_init(back, tx);
            // back_uart_send(query_char);
            uart_send_byte(back, query_char);
            // back_uart_init_rx();
            uart_init(back, rx);
            // for (expected_len) {
            // back_uart_rx()
            data = uart_rx_byte(back);
            // TODO: watchdog?
            //}
        }
        // check_buttons();
        check_buttons();
        if (flash_needs_update) {
            update_flash();
        }

        // front_uart_init_tx();
        uart_init(front, tx);
        // for (expected_len) {
        // front_uart_tx(data[i]);
        HAL_Delay(5000);
        // TODO: pack the bytes

        if (Settings[0]) {
            uart_send_byte(front, 'Y');
        } else {
            uart_send_byte(front, 'N');
        }
#if 0            
        HAL_Delay(500);

        if (data == '*') {
            uart_send_byte(front, 'g');
        } else if (data == 'Y') {
            uart_send_byte(front, 'e');
        } else if (data == 'N') {
            uart_send_byte(front, 'c');
        } else {
            uart_send_byte(front, 'o');
        }

                HAL_Delay(1);

        if(data < 100) {
            uart_send_byte(front, '0');
        }
        else if (data < 200) {
            uart_send_byte(front, '1');
            data = data - 100;
        }
        else {
            uart_send_byte(front, '2');
            data = data - 200;
        }

                HAL_Delay(1);

        if (data < 10) {
            uart_send_byte(front, '0');
        }
        else if (data < 20) {
            uart_send_byte(front, '1');
            data = data - 10;
        }
        else if (data < 30) {
            uart_send_byte(front, '2');
            data = data - 20;
        }
        else if (data < 40) {
            uart_send_byte(front, '3');
            data = data - 30;
        }
        else if (data < 50) {
            uart_send_byte(front, '4');
            data = data - 40;
        }
        else if (data < 60) {
            uart_send_byte(front, '5');
            data = data - 50;
        }
        else if (data < 70) {
            uart_send_byte(front, '6');
            data = data - 60;
        }
        else if (data < 80) {
            uart_send_byte(front, '7');
            data = data - 70;
        }
        else if (data < 90) {
            uart_send_byte(front, '8');
            data = data - 80;
        }
        else {
            uart_send_byte(front, '9');
            data = data - 90;
        }

                HAL_Delay(500);

        if (data == 0) {
            uart_send_byte(front, '0');
        }
        else if (data == 1) {
            uart_send_byte(front, '1');
        }
        else if (data == 2) {
            uart_send_byte(front, '2');
        }
        else if (data == 3) {
            uart_send_byte(front, '3');
        }
        else if (data == 4) {
            uart_send_byte(front, '4');
        }
        else if (data == 5) {
            uart_send_byte(front, '5');
        }
        else if (data == 6) {
            uart_send_byte(front, '6');
        }
        else if (data == 7) {
            uart_send_byte(front, '7');
        }
        else if (data == 8) {
            uart_send_byte(front, '8');
        }
        else if (data == 9) {
            uart_send_byte(front, '9');
        }
        else {
            uart_send_byte(front, '!');
        }
#endif

        HAL_Delay(10);
        uart_send_byte(front, data);
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
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
    // button_helper(8, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3));
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

// XXX: There is 0.8 us of overhead in this function! (should probably subtract by 1)
// This was created experimentally by watching an oscilloscope - it is not that accurate
void sleep_us(uint32_t us)
{
    for (volatile int x = 0; x < us; x++) {
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
    }
}

void uart_init(front_back fb, tx_rx txrx)
{
    // Just enable them all
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;

    if (txrx == tx) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        if (fb == front) {
            GPIO_InitStruct.Pin = CALENDAR_FRONT_UART_PIN;
            HAL_GPIO_Init(CALENDAR_FRONT_UART_MODULE, &GPIO_InitStruct);
            HAL_GPIO_WritePin(CALENDAR_FRONT_UART_MODULE, CALENDAR_FRONT_UART_PIN, GPIO_PIN_SET); // Initialize TX pin high (idle state)
        } else {
            GPIO_InitStruct.Pin = CALENDAR_BACK_UART_PIN;
            HAL_GPIO_Init(CALENDAR_BACK_UART_MODULE, &GPIO_InitStruct);
            HAL_GPIO_WritePin(CALENDAR_BACK_UART_MODULE, CALENDAR_BACK_UART_PIN, GPIO_PIN_SET); // Initialize TX pin high (idle state)
        }
    } else {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        if (fb == front) {
            GPIO_InitStruct.Pin = CALENDAR_FRONT_UART_PIN;
            HAL_GPIO_Init(CALENDAR_FRONT_UART_MODULE, &GPIO_InitStruct);
        } else {
            GPIO_InitStruct.Pin = CALENDAR_BACK_UART_PIN;
            HAL_GPIO_Init(CALENDAR_BACK_UART_MODULE, &GPIO_InitStruct);
        }
    }
}

void uart_send_byte(front_back fb, uint8_t data)
{

    GPIO_TypeDef *GPIO_Module;
    uint16_t GPIO_Pin;
    if (fb == front) {
        GPIO_Module = CALENDAR_FRONT_UART_MODULE;
        GPIO_Pin = CALENDAR_FRONT_UART_PIN;
    } else {
        GPIO_Module = CALENDAR_BACK_UART_MODULE;
        GPIO_Pin = CALENDAR_BACK_UART_PIN;
    }
    // Start bit (low)
    HAL_GPIO_WritePin(GPIO_Module, GPIO_Pin, GPIO_PIN_RESET);
    sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate

    // Send data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        if ((data >> i) & 1) {
            HAL_GPIO_WritePin(GPIO_Module, GPIO_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIO_Module, GPIO_Pin, GPIO_PIN_RESET);
        }
        sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
    }

    // Stop bit (high)
    HAL_GPIO_WritePin(GPIO_Module, GPIO_Pin, GPIO_PIN_SET);
    sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
}

uint8_t uart_rx_byte(front_back fb)
{
    uint8_t data = 0;

    GPIO_TypeDef *GPIO_Module;
    uint16_t GPIO_Pin;
    if (fb == front) {
        GPIO_Module = CALENDAR_FRONT_UART_MODULE;
        GPIO_Pin = CALENDAR_FRONT_UART_PIN;
    } else {
        GPIO_Module = CALENDAR_BACK_UART_MODULE;
        GPIO_Pin = CALENDAR_BACK_UART_PIN;
    }

    // Wait for start bit
    while (HAL_GPIO_ReadPin(GPIO_Module, GPIO_Pin) == GPIO_PIN_SET) {
    }

    ////HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, GPIO_PIN_RESET);

    // Wait for half a bit period and sample in the middle of the bit
    sleep_us(1000000 / (2 * BAUD_RATE));
    sleep_us(1000000 / (BAUD_RATE));

    // Receive data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        data |= (HAL_GPIO_ReadPin(GPIO_Module, GPIO_Pin) << i);
        ////HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, HAL_GPIO_ReadPin(CALENDAR_UART_RX_MODULE, CALENDAR_UART_RX));
        sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
    }

    // HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, GPIO_PIN_SET);

    // Wait for stop bit
    while (HAL_GPIO_ReadPin(GPIO_Module, GPIO_Pin) == GPIO_PIN_RESET) {
    }

    ////HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, GPIO_PIN_SET);

    return data;
}

#if 0
// timeout should be like 1000 us minimum
bool uart_rx_byte_timeout(uint8_t *byte, uint32_t timeout_us)
{
    uint8_t data = 0;
    uint32_t timeout_loop_counter = 0;

    // Wait for start bit
    while (HAL_GPIO_ReadPin(CALENDAR_UART_RX_MODULE, CALENDAR_UART_RX) == GPIO_PIN_SET) {
        timeout_loop_counter++;
        if (timeout_loop_counter == timeout_us >> 2) {
            return false;
        }
        // 1 us in the sleep function is really 1.8ish us but who cares
        sleep_us(1);
    }

    ////HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, GPIO_PIN_RESET);

    // Wait for half a bit period and sample in the middle of the bit
    sleep_us(1000000 / (2 * BAUD_RATE));
    sleep_us(1000000 / (BAUD_RATE));

    // Receive data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        data |= (HAL_GPIO_ReadPin(CALENDAR_UART_RX_MODULE, CALENDAR_UART_RX) << i);
        ////HAL_GPIO_WritePin(CALENDAR_UART_TX_MODULE, CALENDAR_UART_TX, HAL_GPIO_ReadPin(CALENDAR_UART_RX_MODULE, CALENDAR_UART_RX));
        sleep_us(1000000 / BAUD_RATE); // Delay to match baud rate
    }

    timeout_loop_counter = 0;
    // Wait for stop bit
    while (HAL_GPIO_ReadPin(CALENDAR_UART_RX_MODULE, CALENDAR_UART_RX) == GPIO_PIN_RESET) {
        timeout_loop_counter++;
        if (timeout_loop_counter == timeout_us >> 2) {
            // for now consider is succesful anyway.
            break;
        }
        // 1 us in the sleep function is really 1.8ish us but who cares
        sleep_us(1);
    }

    *byte = data;
    return true;
}

LPTIM_HandleTypeDef lpi_handle;

void lptim_init()
{
    lpi_handle.Instance = LPTIM1;
    lpi_handle.Init.Prescaler = LPTIM_PRESCALER_DIV1;
    lpi_handle.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    HAL_LPTIM_Init(&lpi_handle);
    // TODO: Is this needed?
    // HAL_StatusTypeDef HAL_LPTIM_RegisterCallback(LPTIM_HandleTypeDef *lphtim, HAL_LPTIM_CallbackIDTypeDef CallbackID, pLPTIM_CallbackTypeDef pCallback);

    // TXXX: Screw the interrupt, just set the timer running and poll it.
    // Hmmmmm.... Maybe screw all of it and just run a for loop counter...
}

/*void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
}*/
#endif
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    while (1) {
    }
}
#endif /* USE_FULL_ASSERT */
