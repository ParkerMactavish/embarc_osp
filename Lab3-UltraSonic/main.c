/* 引入基本函式庫 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "embARC.h"
#include "embARC_debug.h"

/* 定義計數上限 */
#define MAX_COUNT 0x0fffffff

/* 定義訊號 port與 pin */
#define UltraSonicPort DFSS_GPIO_8B2_ID
#define UltraSonicPin 0

/* ISR的 Function Prototype */
void Timer0ISR();

/* 宣告變數 */
DEV_GPIO_PTR pUltraSonicGPIO;
int IntTime = 0;
int DelayOn = 0;
int IntTimeTotal = 0;

int main(void)
{
	/* 初始化 pUltraSonicGPIO 為相應的 port 與 pin */
	pUltraSonicGPIO = gpio_get_dev(UltraSonicPort);
	pUltraSonicGPIO->gpio_open((1 << UltraSonicPin));

	/* 無限執行主迴圈 */
	while (1)
	{
		/* 設定 pUltraSonicGPIO port 上的 UltraSonicPin 腳位為輸出 */
		pUltraSonicGPIO->gpio_control(GPIO_CMD_SET_BIT_DIR_OUTPUT, (1 << UltraSonicPin));

		/* 停止 TIMER_0 以避免之前被啟動過*/
		timer_stop(TIMER_0);
		/* 設定 TIMER_0 的 ISR 為 Timer0ISR  */
		int_handler_install(INTNO_TIMER0, Timer0ISR);
		/* 設定 TIMER_0 中斷優先程度為最小 */
		int_pri_set(INTNO_TIMER0, INT_PRI_MIN);
		/* 啟動 TIMER_0 中斷 */
		int_enable(INTNO_TIMER0);
		/* 啟動 TIMER_0 ，帶有中斷，同時每數到 0.000001*CPU Cycle中斷一次 */
		timer_start(TIMER_0, TIMER_CTRL_IE, 0.000001 * CLK_CPU);
		/* ↑因為 TIMER_0 每個 CPU Cycle 都會增加1，所以要數到 0.000001*CPU Cycle 就經過了 0.000001(1us) 秒 */

		/* 設定 pUltraSonicGPIO port 上的 UltraSonicPin 腳位輸出高電位 */
		pUltraSonicGPIO->gpio_write(1 << UltraSonicPin, 1 << UltraSonicPin);
		/* 設定等待5次中斷 */
		IntTimeTotal = 5;
		/* 將 DelayOn 設為 1 */
		DelayOn = 1;
		/* 等待 DelayOn 被 Timer0ISR 設為0 */
		while (DelayOn)
		{
			asm("nop;");
		}
		/* ↑5微秒結束 */

		/* 設定 pUltraSonicGPIO 上的 UltraSonicPin 腳位輸出低電位 */
		pUltraSonicGPIO->gpio_write(0 << UltraSonicPin, 1 << UltraSonicPin);
		/* 停止 TIMER_0 */
		timer_stop(TIMER_0);

		/**
		 *  將 pUltraSonicGPIO 上的 UltraSonicPin 腳位設定為輸入
		 * {
		 */
		pUltraSonicGPIO->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (1 << UltraSonicPin));
		/**
		 * }
		 */

		/**
		 * 開啟 TIMER_0 ，模式設定為 Not Halted(TIMER_CTRL_NH) ，上限數值設為 MAX_COUNT
		 * {
		 */
		timer_start(TIMER_0, TIMER_CTRL_IP, MAX_COUNT);
		/**
		 * }
		 */

		/**
		 * 設定一個變數，透過變數讀取 pUltraSonicGPIO 上的 UltraSonicPin 腳位的電位
		 * 如果電位為0就持續執行迴圈等待
		 * {
		 */
		uint32_t USI_1 = 0;
		while (USI_1 == 0)
		{
			pUltraSonicGPIO->gpio_read(&USI_1, 1 << UltraSonicPin);
		}
		/**
		 * }
		 */

		/**
		 * 利用一個變數紀錄 TIMER_0 目前的計數，作為開始的計數
		 * 如果電位為1就持續執行迴圈等待
		 * {
		 */
		uint32_t start_cnt;
		timer_current(TIMER_0, &start_cnt);
		while (USI_1 != 0)
		{
			pUltraSonicGPIO->gpio_read(&USI_1, 1 << UltraSonicPin);
		}
		/**
		 * }
		 */

		/**
		 * 再用第二個變數紀錄TIMER_0目前的技術，作為結束的計數
		 * 透過兩個計數相差的數量除以CPU Cycle(CPU_CLOCK)得到秒數
		 * 再乘以340(m/s)*100(cm/m)/2(來回)得到公分數後回傳
		 * {
		 */
		uint32_t end_cnt;
		timer_current(TIMER_0, &end_cnt);
		uint32_t duration_cnt = (end_cnt - start_cnt);
		float distance = ((float)duration_cnt / 144000000) * 17000;
		/**
		 * }
		 */
		/**
		 * 印出透過超音波測距得到的公分數
		 * {
		 */
		printf("Distence = %f\n", distance);
		/**
		 * }
		 */
	}
	return E_SYS;
}

void Timer0ISR()
{
	timer_int_clear(TIMER_0);
	if (DelayOn == 1)
	{
		IntTime++;
		if (IntTime == IntTimeTotal)
		{
			IntTime = 0;
			DelayOn = 0;
			IntTimeTotal = 0;
		}
	}
}
