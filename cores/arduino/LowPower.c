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

#include "Arduino.h"
#include "LowPower.h"

#ifdef HAL_PWR_MODULE_ENABLED

#ifdef __cplusplus
 extern "C" {
#endif

static UART_HandleTypeDef* WakeUpUart = NULL;
static void (*WakeUpUartCb)( void ) = NULL;

void LowPower_init(){
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Ensure that HSI is wake-up system clock */ 
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

  /* Check if the system was resumed from StandBy mode */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  { 
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
  }

  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF1) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
  }

  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF2) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
  }

  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF3) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF3);
  }

  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF4) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF4);
  }

  /* Check and Clear the Wakeup flag */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF5) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF5);
  }

}

void LowPower_EnableWakeUpPin(uint32_t pin, uint32_t mode) {  
  PinName p = digitalPinToPinName(pin);
  uint32_t wupPin = pinmap_find_function(p, PinMap_WKUP);
  switch (wupPin) {
    case 1 :
      if (mode == RISING) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_HIGH);
      }
      else {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
      }
      break;

    case 2 :
      if (mode == RISING) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH);
      }
      else {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);
      }
      break;

    case 3 :
      if (mode == RISING) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_HIGH);
      }
      else {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_LOW);
      }
      break;

    case 4 :
      if (mode == RISING) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_HIGH);
      }
      else {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_LOW);
      }
      break;

    case 5 :
      if (mode == RISING) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN5_HIGH);
      }
      else {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN5_LOW);
      }
      break;

    default :
      break;
  }
}

void LowPower_sleep(){
  /*Suspend Tick increment to prevent wakeup by Systick interrupt. 
    Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
  HAL_SuspendTick();

  /* Enter Sleep Mode , wake up is done once User push-button is pressed */
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* Resume Tick interrupt if disabled prior to SLEEP mode entry */
  HAL_ResumeTick();

  if (WakeUpUartCb != NULL) {
    WakeUpUartCb();
  }
}

void LowPower_stop(){

  if (WakeUpUart != NULL) {
    HAL_UARTEx_EnableStopMode(WakeUpUart);
  }

  // Enter Stop mode
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  // Exit Stop mode reset clocks
  SystemClock_Config();

  if (WakeUpUart != NULL) {
    HAL_UARTEx_DisableStopMode(WakeUpUart);

    if (WakeUpUartCb != NULL) {
      WakeUpUartCb();
    }
  }
}

void LowPower_standby(){
  HAL_PWR_EnterSTANDBYMode();
}

void LowPower_shutdown(){
  HAL_PWREx_EnterSHUTDOWNMode();
}


void LowPower_EnableWakeUpUart(serial_t* serial, void (*FuncPtr)( void ) ) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  UART_WakeUpTypeDef WakeUpSelection;

  // Save Uart handler
  WakeUpUart = &(serial->handle);

  // Save callback
  WakeUpUartCb = FuncPtr;

  /* make sure that no UART transfer is on-going */ 
  while(__HAL_UART_GET_FLAG(WakeUpUart, USART_ISR_BUSY) == SET);
  /* make sure that UART is ready to receive
   * (test carried out again later in HAL_UARTEx_StopModeWakeUpSourceConfig) */
  while(__HAL_UART_GET_FLAG(WakeUpUart, USART_ISR_REACK) == RESET);

  /* set the wake-up event:
   * specify wake-up on RXNE flag */
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY;
  HAL_UARTEx_StopModeWakeUpSourceConfig(WakeUpUart, WakeUpSelection);

  /* Enable the UART Wake UP from STOP1 mode Interrupt */
  __HAL_UART_ENABLE_IT(WakeUpUart, UART_IT_WUF);
}

#ifdef __cplusplus
}
#endif

#endif // HAL_PWR_MODULE_ENABLED

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
