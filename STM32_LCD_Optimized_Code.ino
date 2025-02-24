#include "stm32f1xx_hal.h"

#define LCD_ADDR 0x27 << 1 // Typical I2C address for PCF8574 backpack

I2C_HandleTypeDef hi2c1;// here we initialize the I2C1 HANDLE

void SystemClock_Config(void);
void I2C1_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Init(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_SendNibble(uint8_t nibble, uint8_t rs);
void LCD_Clear(void);

// Alias for familiar function names
#define lcd_clear() LCD_Clear()
#define lcd_setCursor(row, col) LCD_SetCursor(row, col)

int main(void) {
    setup();
    while (1) {
        loop();
    }
}

void SystemClock_Config(void) {
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

void I2C1_Init(void) {
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // SCL = PB6, SDA = PB7
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

void LCD_SendNibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble << 4) | (rs ? 0x01 : 0x00) | 0x08; // Backlight on
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    data |= 0x04; // Enable pin high
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    data &= ~0x04; // Enable pin low
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1); // i will use hal delay for now rather that Low level for fast initializing
}

void LCD_SendCommand(uint8_t cmd) {
    LCD_SendNibble(cmd >> 4, 0);
    LCD_SendNibble(cmd & 0x0F, 0);
}

void LCD_SendData(uint8_t data) {
    LCD_SendNibble(data >> 4, 1);
    LCD_SendNibble(data & 0x0F, 1);
}

void LCD_Init(void) {
    HAL_Delay(50);
    LCD_SendNibble(0x03, 0);  //we are setting the cursor
    HAL_Delay(5);
    LCD_SendNibble(0x03, 0);
    HAL_Delay(1);
    LCD_SendNibble(0x03, 0);
    LCD_SendNibble(0x02, 0);
    LCD_SendCommand(0x28); // 4-bit mode, 2 lines, 5x8 dots
    LCD_SendCommand(0x0C); // Display on, cursor off, blink off
    LCD_SendCommand(0x01); // Clear display
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    LCD_SendCommand(0x80 | (col + row_offsets[row]));
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01); // Clear display
    HAL_Delay(2);          // Clear command requires a delay
    LCD_SetCursor(0, 0);   // Reset cursor to home position
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

void setup() {
    HAL_Init();   
    SystemClock_Config();
    I2C1_Init();
    LCD_Init();

    lcd_setCursor(0, 0);
    LCD_Print("Setup Complete!");
}

void loop() {
    lcd_setCursor(2, 4);
    LCD_Print("Running Loop...");
    HAL_Delay(1000);

    lcd_clear();
    lcd_setCursor(1, 0);
    LCD_Print("Cleared Display!");
    HAL_Delay(1000);
}
