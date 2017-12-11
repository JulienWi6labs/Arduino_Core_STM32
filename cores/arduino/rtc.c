/**
  ******************************************************************************
  * @file    rtc.c
  * @author  WI6LABS
  * @version V1.0.0
  * @date    25-october-2017
  * @brief   Provides a RTC interface
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

#include "rtc.h"

#ifdef HAL_RTC_MODULE_ENABLED

#ifdef __cplusplus
 extern "C" {
#endif

/* Private define ------------------------------------------------------------*/
/* RTC prediv are set to the default values to match with the LSE clock source.
  If the clock source is modified, those prescalers must be changed */
/* Synchonuous prediv  */
#define PREDIV_S                  0x00FF

/* Asynchonuous prediv   */
#define PREDIV_A                  0x7F

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RTC_HandleTypeDef RtcHandle = {0};
static void (*RTC_CB)(void) = NULL;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief RTC MSP Initialization
  *        This function configures the hardware resources used. By default the
  *        LSE clock is used as RTC clock source.
  * @param hrtc: RTC handle pointer
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select
  *        the RTC clock source; in this case the Backup domain will be reset in
  *        order to modify the RTC Clock source, as consequence RTC registers (including
  *        the backup registers) and RCC_CSR register are set to their reset values.
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  UNUSED(hrtc);
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

#ifdef __HAL_RCC_LSEDRIVE_CONFIG
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
#endif

  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    while(1){
          
    }
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    while(1){
          
    }
  }

  __HAL_RCC_RTC_ENABLE();
}

void RTC_init(hourFormat_t format)
{
  RtcHandle.Instance = RTC;

  RtcHandle.Init.AsynchPrediv = PREDIV_A; /* RTC_ASYNCH_PREDIV; */
#if defined(STM32F1xx)
  RtcHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  UNUSED(format);
#else
  if(format == HOUR_FORMAT_12) {
    RtcHandle.Init.HourFormat = RTC_HOURFORMAT_12;
  } else {
    RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  }
  RtcHandle.Init.SynchPrediv = PREDIV_S; /* RTC_SYNCH_PREDIV; */
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
#if defined(STM32L0xx) || defined(STM32L4xx)
  RtcHandle.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
#endif // defined(STM32L0xx) || defined(STM32L4xx)
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
#endif

  HAL_RTC_Init( &RtcHandle );

  /*Sunday 1st January 2017*/
  RTC_SetDate(17, 1, 1, 7);

  /*at 0:0:0*/
  RTC_SetTime(0,0,0,0,AM);

#if !defined(STM32F1xx) && !defined(STM32F2xx)
  /*Enable Direct Read of the calendar registers (not through Shadow) */
  HAL_RTCEx_EnableBypassShadow(&RtcHandle);
#endif // !defined(STM32F1xx) && !defined(STM32F2xx)

  HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void RTC_DeInit(void)
{
  HAL_RTC_DeInit(&RtcHandle);
  RTC_CB = NULL;
}

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subSeconds, hourAM_PM_t format)
{
  RTC_TimeTypeDef RTC_TimeStruct;

  if(IS_RTC_HOUR24(hours) && IS_RTC_MINUTES(minutes) && IS_RTC_SECONDS(seconds)) {
    RTC_TimeStruct.Hours = hours;
    RTC_TimeStruct.Minutes = minutes;
    RTC_TimeStruct.Seconds = seconds;
#if !defined(STM32F1xx)
    if(format == PM) {
      RTC_TimeStruct.TimeFormat = RTC_HOURFORMAT12_PM;
    } else {
      RTC_TimeStruct.TimeFormat = RTC_HOURFORMAT12_AM;
    }
#if !defined(STM32F2xx) && !defined(STM32L1xx)
    RTC_TimeStruct.SubSeconds = subSeconds;
    RTC_TimeStruct.SecondFraction = 0;
#elif defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
    RTC_TimeStruct.SubSeconds = subSeconds;
    RTC_TimeStruct.SecondFraction = 0;
#else
    UNUSED(subSeconds);
#endif //!defined(STM32F2xx) && !defined(STM32L1xx)
    RTC_TimeStruct.DayLightSaving = RTC_STOREOPERATION_RESET;
    RTC_TimeStruct.StoreOperation = RTC_DAYLIGHTSAVING_NONE;
#else
    UNUSED(subSeconds);
    UNUSED(format);
#endif // !defined(STM32F1xx)

    HAL_RTC_SetTime(&RtcHandle , &RTC_TimeStruct, RTC_FORMAT_BIN);
  }
}

void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint32_t *subSeconds, hourAM_PM_t *format)
{
  RTC_TimeTypeDef RTC_TimeStruct;

  if((hours != NULL) && (minutes != NULL) && (seconds != NULL) && (subSeconds != NULL) && (format != NULL)) {
    HAL_RTC_GetTime(&RtcHandle , &RTC_TimeStruct, RTC_FORMAT_BIN);
    *hours = RTC_TimeStruct.Hours;
    *minutes = RTC_TimeStruct.Minutes;
    *seconds = RTC_TimeStruct.Seconds;
#if !defined(STM32F1xx)
    if(RTC_TimeStruct.TimeFormat == RTC_HOURFORMAT12_PM) {
      *format = PM;
    } else {
      *format = AM;
    }
#if !defined(STM32F2xx) && !defined(STM32L1xx)
    *subSeconds = RTC_TimeStruct.SubSeconds;
#elif defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
    *subSeconds = RTC_TimeStruct.SubSeconds;
#endif
#endif // !defined(STM32F1xx)
  }
}

void RTC_SetDate(uint8_t year, uint8_t month, uint8_t date, uint8_t day)
{
  RTC_DateTypeDef RTC_DateStruct;

  if(IS_RTC_YEAR(year) && IS_RTC_MONTH(month) && IS_RTC_DATE(date) && IS_RTC_WEEKDAY(day)) {
    RTC_DateStruct.Year = year;
    RTC_DateStruct.Month = month;
    RTC_DateStruct.Date = date;
    RTC_DateStruct.WeekDay = day;
    HAL_RTC_SetDate(&RtcHandle , &RTC_DateStruct, RTC_FORMAT_BIN);
  }
}

void RTC_GetDate(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *day)
{
  RTC_DateTypeDef RTC_DateStruct;

  if((year != NULL) && (month != NULL) && (date != NULL) && (day != NULL)) {
    HAL_RTC_GetDate(&RtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN);
    *year = RTC_DateStruct.Year;
    *month = RTC_DateStruct.Month;
    *date = RTC_DateStruct.Date;
    *day = RTC_DateStruct.WeekDay;
  }
}

void RTC_StartAlarm(uint8_t date, uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subSeconds, hourAM_PM_t format)
{
  RTC_AlarmTypeDef RTC_AlarmStructure;

  if(IS_RTC_DATE(date) && IS_RTC_HOUR24(hours) && IS_RTC_MINUTES(minutes) && IS_RTC_SECONDS(seconds)) {
    /* Set RTC_AlarmStructure with calculated values*/
    RTC_AlarmStructure.Alarm = RTC_ALARM_A;
    RTC_AlarmStructure.AlarmTime.Seconds = seconds;
    RTC_AlarmStructure.AlarmTime.Minutes = minutes;
    RTC_AlarmStructure.AlarmTime.Hours = hours;
#if !defined(STM32F1xx)
#if !defined(STM32F2xx) && !defined(STM32L1xx)
    RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_SS14_10;
    RTC_AlarmStructure.AlarmTime.SubSeconds = subSeconds;
#elif defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
    RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_SS14_10;
    RTC_AlarmStructure.AlarmTime.SubSeconds = subSeconds;
#else
    UNUSED(subSeconds);
#endif
    if(format == PM) {
      RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_PM;
    } else {
      RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    }
    RTC_AlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    RTC_AlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    RTC_AlarmStructure.AlarmDateWeekDay = date;
    RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
#else
    UNUSED(subSeconds);
    UNUSED(format);
    UNUSED(date);
#endif // !defined(STM32F1xx)

    /* Set RTC_Alarm */
    HAL_RTC_SetAlarm_IT(&RtcHandle, &RTC_AlarmStructure, RTC_FORMAT_BIN);
  }
}

void RTC_StopAlarm(void)
{
  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle, RTC_FLAG_ALRAF);

  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&RtcHandle, RTC_ALARM_A);
}

void RTC_GetAlarm(uint8_t *date, uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint32_t *subSeconds, hourAM_PM_t *format)
{
  RTC_AlarmTypeDef RTC_AlarmStructure;

  if((date != NULL) && (hours != NULL) && (minutes != NULL) && (seconds != NULL) && (subSeconds != NULL) && (format != NULL)) {
    HAL_RTC_GetAlarm(&RtcHandle, &RTC_AlarmStructure, RTC_ALARM_A, RTC_FORMAT_BIN);

    *seconds = RTC_AlarmStructure.AlarmTime.Seconds;
    *minutes = RTC_AlarmStructure.AlarmTime.Minutes;
    *hours = RTC_AlarmStructure.AlarmTime.Hours;
#if !defined(STM32F1xx)
    *date = RTC_AlarmStructure.AlarmDateWeekDay;
    if(RTC_AlarmStructure.AlarmTime.TimeFormat == RTC_HOURFORMAT12_PM) {
      *format = PM;
    } else {
      *format = AM;
    }
#if !defined(STM32F2xx) && !defined(STM32L1xx)
    *subSeconds = RTC_AlarmStructure.AlarmTime.SubSeconds;
#elif defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
    *subSeconds = RTC_AlarmStructure.AlarmTime.SubSeconds;
#endif
#endif // !defined(STM32F1xx)
  }
}

void attachAlarmCallback(void (*func)(void))
{
  RTC_CB = func;
}

void detachAlarmCallback(void)
{
  RTC_CB = NULL;
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  UNUSED(hrtc);

  if(RTC_CB != NULL)
    RTC_CB();
}

void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&RtcHandle);
}

#ifdef __cplusplus
}
#endif

#endif // HAL_RTC_MODULE_ENABLED

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
