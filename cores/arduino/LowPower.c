/**
  ******************************************************************************
  * @file    LowPower.c
  * @author  WI6LABS
  * @version V1.0.0
  * @date    17 - November -2017
  * @brief   Provides a Low Power interface
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "LowPower.h"

#ifdef HAL_PWR_MODULE_ENABLED

#ifdef __cplusplus
 extern "C" {
#endif

void LowPower_init(){
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  RTC_HandleTypeDef rtc_handle;

  // Disable RTC in case it was activated in another sketch
  __HAL_RCC_RTC_ENABLE();
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  rtc_handle.Instance = RTC; 
  HAL_RTC_DeInit(&rtc_handle);
  __HAL_RCC_RTC_DISABLE();  

  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Check if the system was resumed from StandBy mode */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  { 
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
  }


//  HAL_PWR_EnableBkUpAccess();
  
  /* Check if the system was resumed from shutdown mode,
     resort to RTC back-up register RTC_BKP31R to verify 
     whether or not shutdown entry flag was set by software
     before entering shutdown mode.  */
//  if (READ_REG(RTC->BKP31R) == 1)
//  {
//     WRITE_REG( RTC->BKP31R, 0x0 );  /* reset back-up register */
//  }


  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF2) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
  }

}

void LowPower_stop(){
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Ensure that MSI is wake-up system clock */ 
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);


  // Enter Stop mode
//  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);

  // Exit Stop mode reset clocks
  SystemClock_Config();
}

void LowPower_standby(){
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Enable wakeup pin WKUP2 */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);

  HAL_PWR_EnterSTANDBYMode();

  // Exit Stop mode reset clocks
  SystemClock_Config();

}

void LowPower_shutdown(){
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Disable all used wakeup sources: WKUP pin */
//  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);

  /* Clear wake up Flag */
//  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);

  /* Enable wakeup pin WKUP2 */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);

  /* Set RTC back-up register RTC_BKP31R to indicate
     later on that system has entered shutdown mode  */
//  WRITE_REG( RTC->BKP31R, 0x1 );
  /* Enter shutdown mode */

  HAL_PWREx_EnterSHUTDOWNMode();

}

// Configure all GPIO as analog to reduce current consumption on non used IOs
void LowPower_all_gpio_analog(){
  GPIO_InitTypeDef GPIO_InitStructure;

  // Enable GPIOs clock
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

  __GPIOA_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();
  __GPIOC_CLK_DISABLE();
  __GPIOD_CLK_DISABLE();
  __GPIOE_CLK_DISABLE();
  __GPIOF_CLK_DISABLE();
  __GPIOG_CLK_DISABLE();
  __GPIOH_CLK_DISABLE();

}

#ifdef __cplusplus
}
#endif

#endif // HAL_PWR_MODULE_ENABLED

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
