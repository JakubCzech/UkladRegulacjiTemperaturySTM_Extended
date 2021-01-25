/*
 * heater_pwm_config.c
 *
 *  Created on: Jan 25, 2021
 *      Author: HB
 */


/* Includes ------------------------------------------------------------------*/
#include "heater_pwm.h"
#include "heater_pwm_config.h"
#include "main.h"
#include "tim.h"

/* Typedef -------------------------------------------------------------------*/

/* Define --------------------------------------------------------------------*/

/* Macro ---------------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
HEATER_PWM_HandleTypeDef heaterpwm1 = {
  .Timer = &htim3, .Channel = TIM_CHANNEL_1, .Duty = 0
};

/* Private function prototypes -----------------------------------------------*/

/* Private function ----------------------------------------------------------*/

/* Public function -----------------------------------------------------------*/