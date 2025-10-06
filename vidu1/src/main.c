#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ======================= PROTOTYPES ======================= */
void SystemClock_Config(void);
void GPIO_Init_LEDs(void);
void Delay(volatile uint32_t t);
void delay_us(uint16_t period);
void delay_01ms(uint16_t period);

/* ======================= GLOBAL ======================= */
static TIM_HandleTypeDef htim6;
static uint8_t tim6_inited = 0;

/* ======================= DELAY FUNCTIONS ======================= */

/* Vòng lặp rỗng, không chính xác */
void Delay(volatile uint32_t t)
{
    while (t--) { __NOP(); }
}

/* Lấy clock thực tế của TIM6 (APB1 timer clock) */
static uint32_t TIM6_GetClkHz(void)
{
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        return pclk1 * 2U;
    else
        return pclk1;
}

/* Cấu hình TIM6 với tần số target_hz */
static void TIM6_Setup(uint32_t target_hz)
{
    __HAL_RCC_TIM6_CLK_ENABLE();

    uint32_t timclk = TIM6_GetClkHz();
    uint32_t psc    = timclk / target_hz;
    if (psc == 0) psc = 1;
    psc -= 1U;

    htim6.Instance = TIM6;
    htim6.Init.Prescaler         = (uint16_t)psc;
    htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim6.Init.Period            = 1000 - 1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim6);
    tim6_inited = 1;
}

/* Đảm bảo TIM6 đã init với target_hz */
static void TIM6_Ensure(uint32_t target_hz)
{
    if (!tim6_inited)
    {
        TIM6_Setup(target_hz);
    }
    else
    {
        uint32_t timclk = TIM6_GetClkHz();
        uint32_t want_psc = timclk / target_hz;
        if (want_psc == 0) want_psc = 1;
        want_psc -= 1U;

        if (__HAL_TIM_GET_PRESCALER(&htim6) != (uint16_t)want_psc)
        {
            HAL_TIM_Base_DeInit(&htim6);
            TIM6_Setup(target_hz);
        }
    }
}

/* Delay theo microsecond */
void delay_us(uint16_t period)
{
    TIM6_Ensure(1000000U);  // 1 tick = 1 µs
    __HAL_TIM_SET_AUTORELOAD(&htim6, (uint16_t)(period - 1));
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);

    HAL_TIM_Base_Start(&htim6);
    while (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) == RESET) { }
    HAL_TIM_Base_Stop(&htim6);
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
}

/* Delay theo đơn vị 0.1 ms (100 µs) */
void delay_01ms(uint16_t period)
{
    TIM6_Ensure(10000U);   // 1 tick = 0.1 ms
    __HAL_TIM_SET_AUTORELOAD(&htim6, (uint16_t)(period - 1));
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);

    HAL_TIM_Base_Start(&htim6);
    while (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) == RESET) { }
    HAL_TIM_Base_Stop(&htim6);
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
}

/* ======================= MAIN ======================= */

int main(void)
{
    /* 1️⃣ Khởi tạo HAL (SysTick, NVIC, Flash latency, v.v.) */
    HAL_Init();

    /* 2️⃣ Cấu hình clock hệ thống (84 MHz từ HSE/PLL) */
    SystemClock_Config();

    /* 3️⃣ Bật clock & cấu hình GPIO PD12, PD15 làm Output */
    GPIO_Init_LEDs();

    /* 4️⃣ Chớp LED mỗi 1 giây */
    while (1)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_SET);
        delay_01ms(10000);   // 10000 x 0.1ms = 1s

        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_RESET);
        delay_01ms(10000);   // 1s
    }
}

/* ======================= SUPPORT ======================= */

/* Hàm cấu hình GPIO LED */
void GPIO_Init_LEDs(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_15;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/* Cấu hình clock hệ thống: 84 MHz (HSE 8MHz + PLL) */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 1️⃣ Bật HSE và bật PLL (HSE * 336 / 4 = 84MHz) */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;   // 336/4 = 84 MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* 2️⃣ Cấu hình AHB, APB1, APB2 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   // AHB = 84 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     // APB1 = 42 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     // APB2 = 84 MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    /* 3️⃣ Cập nhật SystemCoreClock */
    SystemCoreClockUpdate();
}
