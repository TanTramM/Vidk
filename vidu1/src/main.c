#include "stm32f4xx.h"
#include "system_timetick.h"

int main(void)
{
  	GPIO_InitTypeDef  GPIO_InitStructure;
	
	/* Enable SysTick at 10ms interrupt */
	//SysTick_Config(SystemCoreClock/100);

  /* GPIOD Peripheral clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure PD12, PD13 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
    
  while (1)
  {
    /* Set PD12 PD15 */
    	GPIO_SetBits(GPIOD,GPIO_Pin_12);
		GPIO_SetBits(GPIOD,GPIO_Pin_15);
		delay_01ms(10000);
		
		/* Clear PD12 PD15 */
		GPIO_ResetBits(GPIOD,GPIO_Pin_12);
		GPIO_ResetBits(GPIOD,GPIO_Pin_15);
		delay_01ms(10000);
		
		a++;
  }
}


