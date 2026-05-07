#include "node_test.h"

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"
#include "usart.h"

#include "stepperMotor.h"
#include "motor_control.h"
#include "v_VL53L4CD.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

#define TEST_PRINT_STATUS   0

/*----------------------------------------------------------------------------*/
/* UART CLI */

#if TEST_ENABLE_UART_CLI

static uint8_t cli_rx_byte;

static char cli_rx_buf[64];
static volatile uint8_t cli_rx_len = 0;

static char cli_line[64];
static volatile uint8_t cli_line_ready = 0;

static void Test_Cli_StartRx(void)
{
    cli_rx_len = 0;
    cli_line_ready = 0;

    HAL_UART_Receive_IT(&huart2, &cli_rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2)
    {
        char ch = (char)cli_rx_byte;

        if (ch == ';' || ch == '\r' || ch == '\n')
        {
            if (cli_rx_len > 0)
            {
                cli_rx_buf[cli_rx_len] = '\0';

                strncpy(cli_line, cli_rx_buf, sizeof(cli_line));
                cli_line[sizeof(cli_line) - 1] = '\0';

                cli_line_ready = 1;
                cli_rx_len = 0;
            }
        }
        else
        {
            if (cli_rx_len < sizeof(cli_rx_buf) - 1)
            {
                cli_rx_buf[cli_rx_len++] = ch;
            }
            else
            {
                cli_rx_len = 0;
            }
        }

        // همیشه RX بعدی را دوباره فعال کن
        HAL_UART_Receive_IT(&huart2, &cli_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2)
    {
        // اگر overrun/error رخ داد، RX را دوباره فعال می‌کنیم
        cli_rx_len = 0;
        HAL_UART_Receive_IT(&huart2, &cli_rx_byte, 1);
    }
}

static void Test_Cli_Process(void)
{
    if (!cli_line_ready)
        return;

    cli_line_ready = 0;

    char line[64];
    strncpy(line, cli_line, sizeof(line));
    line[sizeof(line) - 1] = '\0';

    printf("CLI LINE: [%s]\r\n", line);
    // Trim leading spaces before parsing CLI command
    char *cmd = line;

    // Trim leading spaces before parsing CLI command
    while (*cmd == ' ' || *cmd == '\t')
    {
        cmd++;
    }

    int motor_idx = 0;
    int delta_steps = 0;
    unsigned long mask_ul = 0;
    int argc = 0;
    char *argv[1 + (MOTOR_COUNT * 2)];


    if (sscanf(cmd, "mj %d %d", &motor_idx, &delta_steps) == 2)
    {
        printf("CLI: mj idx=%d delta=%d\r\n", motor_idx, delta_steps);
        Motor_JogSteps((uint8_t)motor_idx, (int16_t)delta_steps);
    }
    else if (sscanf(cmd, "mjm %lx %d", &mask_ul, &delta_steps) == 2)
    {
        // mjm <mask_hex> <delta>
        // Group jog command.
        //
        // Example:
        // mjm 0000000F 200
        // moves motors 0,1,2,3 by +200 steps.
        printf("CLI: mjm mask=0x%08lX delta=%d\r\n", mask_ul, delta_steps);
        Motor_JogMaskSteps((uint32_t)mask_ul, (int16_t)delta_steps);
    }
    else if (sscanf(cmd, "mpos %d", &motor_idx) == 1)
    {
        const actuator_t *m = Motor_Get((uint8_t)motor_idx);

        if (m == NULL)
        {
            printf("CLI: mpos invalid idx=%d\r\n", motor_idx);
        }
        else
        {
            uint32_t rem = stepper_remaining((uint8_t)motor_idx);

            printf("M[%d]: pos=%ld target=%ld rem=%lu moving=%u fault=%u fault_type=%u\r\n",
                   motor_idx,
                   (long)m->current_pos,
                   (long)m->target_pos,
                   (unsigned long)rem,
                   m->is_moving,
                   m->fault,
                   (unsigned)m->fault_type);
        }
    }
    else if (sscanf(cmd, "mhome %d", &motor_idx) == 1)
    {
        // mhome <idx>
        // Move motor back to software home position (position = 0).
        //
        // IMPORTANT:
        // This is NOT physical homing.
        // We assume startup position is home.
        // The motor returns by reversing the stored software position.
        printf("CLI: mhome idx=%d\r\n", motor_idx);
        Motor_GoHome((uint8_t)motor_idx);
    }
    else if (strcmp(cmd, "mhomeall") == 0)
    {
        // mhomeall
        // Return all moved motors back to software home.
        //
        // IMPORTANT:
        // This is NOT physical homing.
        // It only reverses stored software positions.
        printf("CLI: mhomeall\r\n");
        Motor_GoHomeAll();
    }
    else if (sscanf(cmd, "mzero %d", &motor_idx) == 1)
    {
        // mzero <idx>
        // Force software position to zero WITHOUT moving the motor.
        //
        // Use only when operator is sure the motor is physically at home.
        printf("CLI: mzero idx=%d\r\n", motor_idx);
        Motor_ForceSetHome((uint8_t)motor_idx);
    }
    else if (strncmp(cmd, "mjv", 3) == 0)
    {
        // mjv <idx0> <delta0> <idx1> <delta1> ...
        //
        // Supports up to MOTOR_COUNT motors.
        //
        // Example:
        // mjv 0 100 1 200 2 -150
        //
        // Current implementation:
        // Sequential execution, not simultaneous.

        argc = 0;

        char *tok = strtok(cmd, " \t");

        while (tok != NULL && argc < (int)(1 + (MOTOR_COUNT * 2)))
        {
            argv[argc++] = tok;
            tok = strtok(NULL, " \t");
        }

        if (argc < 3 || ((argc - 1) % 2) != 0)
        {
            printf("CLI: mjv bad args\r\n");
        }
        else
        {
            printf("CLI: mjv motors=%d\r\n", (argc - 1) / 2);

            for (int i = 1; i < argc; i += 2)
            {
                int idx = atoi(argv[i]);
                int delta = atoi(argv[i + 1]);

                printf("  motor=%d delta=%d\r\n", idx, delta);
                Motor_JogSteps((uint8_t)idx, (int16_t)delta);
            }
        }
    }
    else
    {
        printf("CLI: unknown [%s]\r\n", cmd);
    }

    // Safety re-arm
    HAL_UART_Receive_IT(&huart2, &cli_rx_byte, 1);
}

#endif

/*----------------------------------------------------------------------------*/
/* MOTOR ONE AUTO TEST */

#if TEST_ENABLE_MOTOR_ONE

static void Test_MotorOne_Init(void)
{
    printf("TEST: MOTOR ONE START idx=%u\r\n", TEST_MOTOR_INDEX);

    // سرعت مشترک همه موتورهای فعال.
    // NOTE:
    // در این پروژه همه موتورها از PWM مشترک استفاده می‌کنند.
    stepper_set_speed_hz(150);
}

/*----------------------------------------------------------------------------*/

static void Test_MotorOne_Process(void)
{
    static uint8_t phase = 0;
    static TickType_t lastChange = 0;

    TickType_t now = xTaskGetTickCount();

#if TEST_PRINT_STATUS
    static TickType_t lastPrint = 0;

    if ((now - lastPrint) > pdMS_TO_TICKS(500))
    {
        const actuator_t *m = Motor_Get(TEST_MOTOR_INDEX);
        uint32_t rem = stepper_remaining(TEST_MOTOR_INDEX);

        printf("M[%u]: pos=%ld target=%ld rem=%lu moving=%u fault=%u fault_type=%u\r\n",
               TEST_MOTOR_INDEX,
               (long)m->current_pos,
               (long)m->target_pos,
               (unsigned long)rem,
               m->is_moving,
               m->fault,
               (unsigned)m->fault_type);

        lastPrint = now;
    }
#endif

    const actuator_t *m = Motor_Get(TEST_MOTOR_INDEX);

    switch (phase)
    {
        case 0:
            printf("TEST: MOTOR[%u] MOVE TO POS=1000\r\n", TEST_MOTOR_INDEX);
            Motor_SetTarget(TEST_MOTOR_INDEX, 1000);
            phase = 1;
            break;

        case 1:
            if (m->fault)
            {
                printf("TEST: MOTOR[%u] FAULT fault_type=%u POS=%ld\r\n",
                       TEST_MOTOR_INDEX,
                       (unsigned)m->fault_type,
                       (long)m->current_pos);

                phase = 4;
                break;
            }

            if (m->is_moving == 0)
            {
                printf("TEST: MOTOR[%u] REACHED POS=%ld\r\n",
                       TEST_MOTOR_INDEX,
                       (long)m->current_pos);

                lastChange = now;
                phase = 2;
            }
            break;

        case 2:
            if ((now - lastChange) > pdMS_TO_TICKS(TEST_MOTOR_PAUSE_MS))
            {
                printf("TEST: MOTOR[%u] GO HOME\r\n", TEST_MOTOR_INDEX);
                Motor_GoHome(TEST_MOTOR_INDEX);
                phase = 3;
            }
            break;

        case 3:
            if (m->fault)
            {
                printf("TEST: MOTOR[%u] HOME FAULT fault_type=%u POS=%ld\r\n",
                       TEST_MOTOR_INDEX,
                       (unsigned)m->fault_type,
                       (long)m->current_pos);

                phase = 4;
                break;
            }

            if (m->is_moving == 0)
            {
                printf("TEST: MOTOR[%u] HOME DONE POS=%ld\r\n",
                       TEST_MOTOR_INDEX,
                       (long)m->current_pos);

                phase = 4;
            }
            break;

        case 4:
            break;

        default:
            phase = 0;
            break;
    }
}

#endif

/*----------------------------------------------------------------------------*/
/* SENSOR OFFSET CAL TEST */

#if TEST_ENABLE_SENSOR_OFFSET_CAL

	static void Test_SensorOffsetCal_RunOnce(void)
	{
		int16_t measured_offset = 0;

		// صبر برای کامل شدن init سنسورها.
		osDelay(pdMS_TO_TICKS(8000));

		// هنگام calibration، SensorTask را pause می‌کنیم.
		// IMPORTANT:
		// VL53L4CD_CalibrateOffset خودش ranging را کنترل می‌کند.
		g_sensor_cal_busy = 1;
		osDelay(pdMS_TO_TICKS(100));

		printf("TEST: SENSOR[%u] OFFSET CAL START target=%dmm\r\n",
			   TEST_SENSOR_INDEX,
			   TEST_SENSOR_OFFSET_TARGET_MM);

		uint8_t st = VL53L4CD_CalibrateOffset_One(TEST_SENSOR_INDEX,
												  TEST_SENSOR_OFFSET_TARGET_MM,
												  &measured_offset);

		printf("TEST: SENSOR[%u] OFFSET CAL DONE st=%u offset=%dmm\r\n",
			   TEST_SENSOR_INDEX,
			   st,
			   measured_offset);

		g_sensor_cal_busy = 0;

		osDelay(pdMS_TO_TICKS(1000));
		printf("TEST: SENSOR OFFSET CAL RELEASED\r\n");
	}

#endif

/*----------------------------------------------------------------------------*/

	void TestTask(void *argument)
	{
	#if TEST_ENABLE_UART_CLI
	    Test_Cli_StartRx();
	    printf("CLI READY\r\n");
	#endif

	#if TEST_ENABLE_SENSOR_OFFSET_CAL
	    Test_SensorOffsetCal_RunOnce();
	#endif

	#if TEST_ENABLE_MOTOR_ONE
	    Test_MotorOne_Init();
	#endif

	    for (;;)
	    {
	        Motor_Process();

	#if TEST_ENABLE_UART_CLI
	        Test_Cli_Process();
	#endif

	#if TEST_ENABLE_MOTOR_ONE
	        Test_MotorOne_Process();
	#endif

	        osDelay(10);
	    }
	}
/*----------------------------------------------------------------------------*/
