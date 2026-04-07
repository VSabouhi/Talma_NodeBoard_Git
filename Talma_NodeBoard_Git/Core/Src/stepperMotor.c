#include "stepperMotor.h"
#include "main.h"
#include "motor_cmd.h"
#include <stdio.h>
#include "tim.h"

/*----------------------------------------------------------------------------*/
static volatile uint32_t g_step_hz = 1000;   // پیش‌فرض
/*----------------------------------------------------------------------------*/
typedef struct {
  GPIO_TypeDef* gate_port; uint16_t gate_pin; // PixXX_PW : gate buffer
  GPIO_TypeDef* dir_port;  uint16_t dir_pin;  // PixXX_Dir
  GPIO_TypeDef* en_port;   uint16_t en_pin;   // PixXX_En  (A4988: ENABLE active-low)
} motor_hw_t;
/*----------------------------------------------------------------------------*/
static motor_hw_t hw[STEPPER_MOTOR_COUNT];
static volatile uint8_t g_any_active = 0;
/*----------------------------------------------------------------------------*/
// --------------------- runtime state ---------------------
static volatile uint32_t g_active_mask = 0;
static volatile int32_t  g_rem[STEPPER_MOTOR_COUNT];
/*----------------------------------------------------------------------------*/
void v_motor_speed_update(uint16_t v_data);

/*----------------------------------------------------------------------------*/
static inline void driver_enable(uint8_t id, bool en)
{
  HAL_GPIO_WritePin(hw[id].en_port, hw[id].en_pin, en ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
/*----------------------------------------------------------------------------*/
static inline void gate_enable(uint8_t id, bool en)
{
  HAL_GPIO_WritePin(hw[id].gate_port, hw[id].gate_pin, en ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
/*----------------------------------------------------------------------------*/
static inline void set_dir(uint8_t id, stepper_dir_t dir)
{
  HAL_GPIO_WritePin(hw[id].dir_port, hw[id].dir_pin, (dir == STEPPER_DIR_CCW) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static inline void pwm_start_if_needed(void)
{
  // سرعت: ARR=10000 (ولی v_motor_speed_update باید ARR/CCR را تنظیم کند، نه Start)
  v_motor_speed_update(200);

  // اگر CCR صفر باشد PWM خروجی ندارد
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
  if (__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4) == 0)
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, arr/2);

  __HAL_TIM_SET_COUNTER(&htim3, 0);
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);   // ⭐ مهم

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_Base_Start_IT(&htim3);
}
/*----------------------------------------------------------------------------*/


static inline void pwm_stop_if_idle(void)
{
  if (g_active_mask == 0)
  {
    // قطع PWM و وقفه update
    HAL_TIM_Base_Stop_IT(&htim3);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
  }
}

/*----------------------------------------------------------------------------*/

static uint32_t tim3_timclk_hz(void)
{
  uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
  if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) return 2u * pclk1;
  return pclk1;
}
/*----------------------------------------------------------------------------*/
void stepper_set_speed_hz(uint32_t hz)
{
  if (hz < 1) hz = 1;
  if (hz > 50000) hz = 50000;
  g_step_hz = hz;
}
/*----------------------------------------------------------------------------*/
static void tim3_apply_step_hz(uint32_t hz)
{
  uint32_t timclk = tim3_timclk_hz();
  uint32_t psc    = htim3.Init.Prescaler;
  uint32_t tick   = timclk / (psc + 1u);

  uint32_t arr = tick / hz;
  if (arr > 0) arr -= 1;
  if (arr < 10) arr = 10;
  if (arr > 0xFFFF) arr = 0xFFFF;

  __HAL_TIM_SET_AUTORELOAD(&htim3, (uint16_t)arr);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (uint16_t)(arr / 2)); // 50%
  __HAL_TIM_SET_COUNTER(&htim3, 0);
}
/*----------------------------------------------------------------------------*/
void stepper_init(void)
{
  // موتور 0 = Pix01 ... موتور 31 = Pix32
  // Gate = PixXX_PW
  // Dir  = PixXX_Dir
  // En   = PixXX_En

  hw[0]  = (motor_hw_t){Pix01_PW_GPIO_Port, Pix01_PW_Pin, Pix01_Dir_GPIO_Port, Pix01_Dir_Pin, Pix01_En_GPIO_Port, Pix01_En_Pin};
  hw[1]  = (motor_hw_t){Pix02_PW_GPIO_Port, Pix02_PW_Pin, Pix02_Dir_GPIO_Port, Pix02_Dir_Pin, Pix02_En_GPIO_Port, Pix02_En_Pin};
  hw[2]  = (motor_hw_t){Pix03_PW_GPIO_Port, Pix03_PW_Pin, Pix03_Dir_GPIO_Port, Pix03_Dir_Pin, Pix03_En_GPIO_Port, Pix03_En_Pin};
  hw[3]  = (motor_hw_t){Pix04_PW_GPIO_Port, Pix04_PW_Pin, Pix04_Dir_GPIO_Port, Pix04_Dir_Pin, Pix04_En_GPIO_Port, Pix04_En_Pin};
  hw[4]  = (motor_hw_t){Pix05_PW_GPIO_Port, Pix05_PW_Pin, Pix05_Dir_GPIO_Port, Pix05_Dir_Pin, Pix05_En_GPIO_Port, Pix05_En_Pin};
  hw[5]  = (motor_hw_t){Pix06_PW_GPIO_Port, Pix06_PW_Pin, Pix06_Dir_GPIO_Port, Pix06_Dir_Pin, Pix06_En_GPIO_Port, Pix06_En_Pin};
  hw[6]  = (motor_hw_t){Pix07_PW_GPIO_Port, Pix07_PW_Pin, Pix07_Dir_GPIO_Port, Pix07_Dir_Pin, Pix07_En_GPIO_Port, Pix07_En_Pin};
  hw[7]  = (motor_hw_t){Pix08_PW_GPIO_Port, Pix08_PW_Pin, Pix08_Dir_GPIO_Port, Pix08_Dir_Pin, Pix08_En_GPIO_Port, Pix08_En_Pin};

  hw[8]  = (motor_hw_t){Pix09_PW_GPIO_Port, Pix09_PW_Pin, Pix09_Dir_GPIO_Port, Pix09_Dir_Pin, Pix09_En_GPIO_Port, Pix09_En_Pin};
  hw[9]  = (motor_hw_t){Pix10_PW_GPIO_Port, Pix10_PW_Pin, Pix10_Dir_GPIO_Port, Pix10_Dir_Pin, Pix10_En_GPIO_Port, Pix10_En_Pin};
  hw[10] = (motor_hw_t){Pix11_PW_GPIO_Port, Pix11_PW_Pin, Pix11_Dir_GPIO_Port, Pix11_Dir_Pin, Pix11_En_GPIO_Port, Pix11_En_Pin};
  hw[11] = (motor_hw_t){Pix12_PW_GPIO_Port, Pix12_PW_Pin, Pix12_Dir_GPIO_Port, Pix12_Dir_Pin, Pix12_En_GPIO_Port, Pix12_En_Pin};
  hw[12] = (motor_hw_t){Pix13_PW_GPIO_Port, Pix13_PW_Pin, Pix13_Dir_GPIO_Port, Pix13_Dir_Pin, Pix13_En_GPIO_Port, Pix13_En_Pin};
  hw[13] = (motor_hw_t){Pix14_PW_GPIO_Port, Pix14_PW_Pin, Pix14_Dir_GPIO_Port, Pix14_Dir_Pin, Pix14_En_GPIO_Port, Pix14_En_Pin};
  hw[14] = (motor_hw_t){Pix15_PW_GPIO_Port, Pix15_PW_Pin, Pix15_Dir_GPIO_Port, Pix15_Dir_Pin, Pix15_En_GPIO_Port, Pix15_En_Pin};
  hw[15] = (motor_hw_t){Pix16_PW_GPIO_Port, Pix16_PW_Pin, Pix16_Dir_GPIO_Port, Pix16_Dir_Pin, Pix16_En_GPIO_Port, Pix16_En_Pin};

  hw[16] = (motor_hw_t){Pix17_PW_GPIO_Port, Pix17_PW_Pin, Pix17_Dir_GPIO_Port, Pix17_Dir_Pin, Pix17_En_GPIO_Port, Pix17_En_Pin};
  hw[17] = (motor_hw_t){Pix18_PW_GPIO_Port, Pix18_PW_Pin, Pix18_Dir_GPIO_Port, Pix18_Dir_Pin, Pix18_En_GPIO_Port, Pix18_En_Pin};
  hw[18] = (motor_hw_t){Pix19_PW_GPIO_Port, Pix19_PW_Pin, Pix19_Dir_GPIO_Port, Pix19_Dir_Pin, Pix19_En_GPIO_Port, Pix19_En_Pin};
  hw[19] = (motor_hw_t){Pix20_PW_GPIO_Port, Pix20_PW_Pin, Pix20_Dir_GPIO_Port, Pix20_Dir_Pin, Pix20_En_GPIO_Port, Pix20_En_Pin};
  hw[20] = (motor_hw_t){Pix21_PW_GPIO_Port, Pix21_PW_Pin, Pix21_Dir_GPIO_Port, Pix21_Dir_Pin, Pix21_En_GPIO_Port, Pix21_En_Pin};
  hw[21] = (motor_hw_t){Pix22_PW_GPIO_Port, Pix22_PW_Pin, Pix22_Dir_GPIO_Port, Pix22_Dir_Pin, Pix22_En_GPIO_Port, Pix22_En_Pin};
  hw[22] = (motor_hw_t){Pix23_PW_GPIO_Port, Pix23_PW_Pin, Pix23_Dir_GPIO_Port, Pix23_Dir_Pin, Pix23_En_GPIO_Port, Pix23_En_Pin};
  hw[23] = (motor_hw_t){Pix24_PW_GPIO_Port, Pix24_PW_Pin, Pix24_Dir_GPIO_Port, Pix24_Dir_Pin, Pix24_En_GPIO_Port, Pix24_En_Pin};

  hw[24] = (motor_hw_t){Pix25_PW_GPIO_Port, Pix25_PW_Pin, Pix25_Dir_GPIO_Port, Pix25_Dir_Pin, Pix25_En_GPIO_Port, Pix25_En_Pin};
  hw[25] = (motor_hw_t){Pix26_PW_GPIO_Port, Pix26_PW_Pin, Pix26_Dir_GPIO_Port, Pix26_Dir_Pin, Pix26_En_GPIO_Port, Pix26_En_Pin};
  hw[26] = (motor_hw_t){Pix27_PW_GPIO_Port, Pix27_PW_Pin, Pix27_Dir_GPIO_Port, Pix27_Dir_Pin, Pix27_En_GPIO_Port, Pix27_En_Pin};
  hw[27] = (motor_hw_t){Pix28_PW_GPIO_Port, Pix28_PW_Pin, Pix28_Dir_GPIO_Port, Pix28_Dir_Pin, Pix28_En_GPIO_Port, Pix28_En_Pin};
  hw[28] = (motor_hw_t){Pix29_PW_GPIO_Port, Pix29_PW_Pin, Pix29_Dir_GPIO_Port, Pix29_Dir_Pin, Pix29_En_GPIO_Port, Pix29_En_Pin};
  hw[29] = (motor_hw_t){Pix30_PW_GPIO_Port, Pix30_PW_Pin, Pix30_Dir_GPIO_Port, Pix30_Dir_Pin, Pix30_En_GPIO_Port, Pix30_En_Pin};
  hw[30] = (motor_hw_t){Pix31_PW_GPIO_Port, Pix31_PW_Pin, Pix31_Dir_GPIO_Port, Pix31_Dir_Pin, Pix31_En_GPIO_Port, Pix31_En_Pin};
  hw[31] = (motor_hw_t){Pix32_PW_GPIO_Port, Pix32_PW_Pin, Pix32_Dir_GPIO_Port, Pix32_Dir_Pin, Pix32_En_GPIO_Port, Pix32_En_Pin};

  // همه خاموش
  g_active_mask = 0;
  for (int i = 0; i < STEPPER_MOTOR_COUNT; i++)
  {
    g_rem[i] = 0;
    gate_enable(i, false);
    driver_enable(i, false); // /EN high
  }

  pwm_stop_if_idle();
}

/*----------------------------------------------------------------------------*/
stepper_status_t stepper_move_motor(uint8_t motor_id, int32_t steps_signed)
{
  if (motor_id >= STEPPER_MOTOR_COUNT) return STEPPER_BAD_PARAM;
  if (steps_signed == 0) return STEPPER_OK;

  // 1) جهت
  if (steps_signed >= 0) set_dir(motor_id, STEPPER_DIR_CW);
  else                  set_dir(motor_id, STEPPER_DIR_CCW);

  // 2) فعال‌سازی درایور و گیت
  driver_enable(motor_id, true);   // EN=0 (active-low)
  gate_enable(motor_id, true);     // Gate=1

  // 3) اعمال فرکانس مشترک روی TIM3 (ARR/CCR)
  tim3_apply_step_hz(g_step_hz);

  // 4) شروع PWM (STEP مشترک)
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

  // 5) دیباگ
  printf("MOVE motor=%u dir=%s hz=%lu\r\n",
         motor_id,
         (steps_signed >= 0) ? "CW" : "CCW",
         (unsigned long)g_step_hz);

  return STEPPER_OK;
}
/*----------------------------------------------------------------------------*/

stepper_status_t stepper_move_mask(uint32_t mask, int32_t steps_signed, stepper_dir_t dir)
{
  if (mask == 0) return STEPPER_BAD_PARAM;
  if (steps_signed == 0) return STEPPER_OK;

  int32_t steps = (steps_signed > 0) ? steps_signed : -steps_signed;

  for (uint8_t i = 0; i < STEPPER_MOTOR_COUNT; i++)
  {
    if (mask & (1u << i))
    {
      set_dir(i, dir);
      g_rem[i] = steps;
      driver_enable(i, true);
      gate_enable(i, true);
      g_active_mask |= (1u << i);
    }
  }

  pwm_start_if_needed();
  return STEPPER_OK;
}
/*----------------------------------------------------------------------------*/

void stepper_stop_motor(uint8_t motor_id)
{
  if (motor_id >= STEPPER_MOTOR_COUNT) return;

  gate_enable(motor_id, false);     // اول پالس را قطع کن
  driver_enable(motor_id, false);   // بعد /EN را disable کن

  g_rem[motor_id] = 0;
  g_active_mask &= ~(1u << motor_id);

  pwm_stop_if_idle();
}
/*----------------------------------------------------------------------------*/
void stepper_stop_all(void)
{
  for (uint8_t i = 0; i < STEPPER_MOTOR_COUNT; i++)
  {
    gate_enable(i, false);
    driver_enable(i, false);
    g_rem[i] = 0;
  }
  g_active_mask = 0;
  pwm_stop_if_idle();
}
/*----------------------------------------------------------------------------*/
uint32_t stepper_active_mask(void) { return g_active_mask; }

/*----------------------------------------------------------------------------*/
int32_t stepper_remaining(uint8_t motor_id)
{
  if (motor_id >= STEPPER_MOTOR_COUNT) return 0;
  return g_rem[motor_id];
}
/*----------------------------------------------------------------------------*/
// هر Period تایمر = یک rising edge PWM = یک STEP برای موتورهای active
void stepper_on_tick_isr(void)
{
  uint32_t mask = g_active_mask;
  if (mask == 0) return;

  for (uint8_t i = 0; i < STEPPER_MOTOR_COUNT; i++)
  {
    if (mask & (1u << i))
    {
      if (g_rem[i] > 0)
      {
        g_rem[i]--;
        if (g_rem[i] == 0)
        {
          // این موتور تمام شد
          gate_enable(i, false);
          driver_enable(i, false);
          g_active_mask &= ~(1u << i);
        }
      }
    }
  }

  // اگر همه تمام شدند، PWM را هم خاموش کن
  pwm_stop_if_idle();
}
/*----------------------------------------------------------------------------*/
void v_motor_speed_update(uint16_t v_data)
{
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
	__HAL_TIM_SET_AUTORELOAD(&htim3, v_data);
	TIM3->CCR1 = v_data/2;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
