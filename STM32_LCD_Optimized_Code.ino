#include "stm32f1xx_hal.h"

#define LCD_ADDR 0x27 << 1 // Typical I2C address for PCF8574 backpack

I2C_HandleTypeDef hi2c1;

void systemClockConfig(void);
void i2c1Init(void);
void lcd_sendCommand(uint8_t cmd);
void lcd_sendData(uint8_t data);
void lcd_sendNibble(uint8_t nibble, uint8_t rs);

typedef struct {
    void (*init)(void);
    void (*clear)(void);
    void (*setCursor)(uint8_t row, uint8_t col);
    void (*print)(const char *str);
} LCD_TypeDef;

void lcd_init(void);
void lcd_clear(void);
void lcd_setCursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);

// LCD object
LCD_TypeDef lcd = {
    .init = lcd_init,
    .clear = lcd_clear,
    .setCursor = lcd_setCursor,
    .print = lcd_print
};

int main(void) {
    setup();
    while (1) {
        loop();
    }
}

void systemClockConfig(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void i2c1Init(void) {
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

void lcd_sendNibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble << 4) | (rs ? 0x01 : 0x00) | 0x08;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    data |= 0x04;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    data &= ~0x04;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
}

void lcd_sendCommand(uint8_t cmd) {
    lcd_sendNibble(cmd >> 4, 0);
    lcd_sendNibble(cmd & 0x0F, 0);
}

void lcd_sendData(uint8_t data) {
    lcd_sendNibble(data >> 4, 1);
    lcd_sendNibble(data & 0x0F, 1);
}

void lcd_init(void) {
    HAL_Delay(50);
    lcd_sendNibble(0x03, 0);
    HAL_Delay(5);
    lcd_sendNibble(0x03, 0);
    HAL_Delay(1);
    lcd_sendNibble(0x03, 0);
    lcd_sendNibble(0x02, 0);
    lcd_sendCommand(0x28);
    lcd_sendCommand(0x0C);
    lcd_sendCommand(0x01);
    HAL_Delay(2);
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_sendCommand(0x80 | (col + row_offsets[row]));
}

void lcd_clear(void) {
    lcd_sendCommand(0x01);
    HAL_Delay(2);
    lcd_setCursor(0, 0);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_sendData(*str++);
    }
}

void setup() {
    HAL_Init();   
    systemClockConfig();
    i2c1Init();
    lcd.init();

    lcd.setCursor(0, 0);
    lcd.print("Setup Complete!");
}

void delay(uint32_t ms) {
    HAL_Delay(ms);
}

void loop() {
    lcd.setCursor(2, 3);
    lcd.print("Running Loop...");
    delay(1000);

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Cleared Display!");
    delay(1000);
}
