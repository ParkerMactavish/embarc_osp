/* Necessary Headers Include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "embARC.h"
#include "embARC_debug.h"

/* Max Number for 32-bit Integer */
#define MAX_COUNT 0xffffffff

/* Ultra Sonic Port and Pin */
#define UltraSonicPort DFSS_GPIO_8B2_ID
#define UltraSonicPin 0

/* Ultra Sonic GPIO Object Pointer */
static DEV_GPIO_PTR pUltraSonicGpio;

/* Timer0 ISR Function Prototype */
void Timer0_ISR();

/* Global Variable Declaration */
/* 目前中斷的次數 */
int CurrentIntTimes = 0;
/* 總共需要的中斷次數 */
int TotalIntTimes = 0;
int InDelay = 0;

int main(void)
{
	pUltraSonicGpio = gpio_get_dev(UltraSonicPort);
	pUltraSonicGpio->gpio_open((1 << UltraSonicPin));
	while (1)
	{
		/* 設定 pUltraSonicGpio port 上的UltraSonicPin腳位為輸出 */
		pUltraSonicGpio->gpio_control(GPIO_CMD_SET_BIT_DIR_OUTPUT, (void *)(1 << UltraSonicPin));

		/* 停止TIMER_0以避免之前被啟動過*/
		timer_stop(TIMER_0);
		/* 設定TIMER_0的ISR為USITran */
		int_handler_install(INTNO_TIMER0, Timer0_ISR);
		/* 設定TIMER_0中斷優先程度為最小 */
		int_pri_set(INTNO_TIMER0, INT_PRI_MIN);
		/* 啟動TIMER_0中斷 */
		int_enable(INTNO_TIMER0);
		/* 啟動TIMER_0，帶有中斷，同時每數到1E-6*CPU Cycle中斷一次 */
		/* 因為TIMER_0每個CPU Cycle都會增加1，所以要數到1E-6*CPU Cycle就經過了1E-6秒 */
		timer_start(TIMER_0, TIMER_CTRL_IE, 0.000001 * BOARD_CPU_CLOCK);

		// pUltraSonicGpio->gpio_write(0 << UltraSonicPin, 1 << UltraSonicPin);
		// // TotalIntTimes=5;
		// TotalIntTimes = 2;
		// DelayFuntion();

		/* 設定pUltraSonicGpio port上的UltraSonicPin腳位輸出高電位 */
		pUltraSonicGpio->gpio_write(1 << UltraSonicPin, 1 << UltraSonicPin);
		// TotalIntTimes=10;
		/* 設定等待5微秒 */
		TotalIntTimes = 5;

		/* 等待涵式 */
		InDelay = 1;
		while (InDelay)
		{
			asm("nop;");
		}

		/* 設定pUltraSonicGpio port上的UltraSonicPin腳位輸出低電位 */
		pUltraSonicGpio->gpio_write(0 << UltraSonicPin, 1 << UltraSonicPin);
		/* 暫停TIMER_0 */
		timer_stop(TIMER_0);

		/**
		 *  將pUltraSonicGpio port上的UltraSonicPin腳位設定為輸入
		 * {
		 */
		pUltraSonicGpio->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)(1 << UltraSonicPin));
		/**
		 * }
		 */

		/**
		 * 開啟TIMER_0，模式設定為Not Halted(TIMER_CTRL_NH)，上限數值設為MAX_COUNT
		 * {
		 */
		timer_start(TIMER_0, TIMER_CTRL_NH, MAX_COUNT);
		/**
		 * }
		 */

		/**
		 * 設定一個變數，透過變數讀取pUltraSonicGpio上的UltraSonicPin腳位的電位
		 * 如果電位為0就持續執行迴圈等待
		 * {
		 */
		uint32_t USI_1 = 0;
		while (USI_1 == 0)
		{
			// pUltraSonicGpio->gpio_read(&USI_1, 1 << IOTDK_USI_1_1);
			pUltraSonicGpio->gpio_read(&USI_1, 1 << UltraSonicPin);
		}
		/**
		 * }
		 */

		/**
		 * 利用一個變數紀錄TIMER_0目前的計數，作為開始的計數
		 * 如果電位為1就持續執行迴圈等待
		 * {
		 */
		uint32_t start_cnt = _arc_aux_read(AUX_TIMER0_CNT);
		while (USI_1 != 0)
		{
			// pUltraSonicGpio->gpio_read(&USI_1, 1 << IOTDK_USI_1_1);
			pUltraSonicGpio->gpio_read(&USI_1, 1 << UltraSonicPin);
		}
		/**
		 * }
		 */

		/**
		 * 再用第二個變數紀錄TIMER_0目前的技術，作為結束的計數
		 * 透過兩個計數相差的數量除以CPU Cycle(BOARD_CPU_CLOCK)得到秒數
		 * 再乘以340(m/s)*100(cm/m)/2(來回)得到公分數後回傳
		 * {
		 */
		uint32_t end_cnt = _arc_aux_read(AUX_TIMER0_CNT);
		uint32_t time = (end_cnt - start_cnt);
		float dis = ((float)time / 144000000) * 17000;
		/**
		 * }
		 */
		/**
		 * 印出透過超音波測距得到的公分數
		 * {
		 */
		printf("Distence = %f\n", dis);
		/**
		 * }
		 */
	}
	return E_SYS;
}

void Timer0_ISR()
{
	timer_int_clear(TIMER_0);
	if (InDelay == 1)
	// if (CurrentIntTimes < TotalIntTimes)
	{
		CurrentIntTimes++;
		if (CurrentIntTimes == TotalIntTimes)
		{
			CurrentIntTimes = 0;
			InDelay = 0;
			TotalIntTimes = 0;
		}
	}
}
