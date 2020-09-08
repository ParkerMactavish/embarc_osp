/* ------------------------------------------
 * Copyright (c) 2017, Synopsys, Inc. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
modification,
 * are permitted provided that the following conditions are met:

 * 1) Redistributions of source code must retain the above copyright notice,
this
 * list of conditions and the following disclaimer.

 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
and/or
 * other materials provided with the distribution.

 * 3) Neither the name of the Synopsys, Inc., nor the names of its contributors
may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
--------------------------------------------- */
#include <stdio.h>

#include "embARC.h"
#include "embARC_debug.h"
#include "iic1602lcd.h"

/* Definitions for Ultra Sonic Ranger */
/* Define Max Number for 32-bit Integer */
#define MAX_COUNT 0xffffffff

/* Define Ultra Sonic Port and Pin */
#define UltraSonicPort DFSS_GPIO_8B2_ID
#define UltraSonicPin 0

/* Declare Ultra Sonic GPIO Object Pointer */
DEV_GPIO_PTR pUltraSonicGpio;

/* Declare Timer0 ISR Function Prototype */
void Timer0_ISR();

/* Declare Global Variable */
/** How many times interrupt had occurred **/
volatile int CurrentIntTimes = 0;
/** How many times interrupt we need **/
int TotalIntTimes = 0;
/* Definition for Ultra Sonic Ranger End */

/* Declare LCD Object to control LCD */
pLCD_t lcd_obj;

int main(void)
{
  /* Initialize LCD object with IIC 0 */
  lcd_obj = LCD_Init(DFSS_IIC_0_ID);

  /**
   * 透過 UltraSonicPort 得到 pUltraSonicGpio
   * 並將位於 pUltraSonicGpio 物件中的 UltraSonicPin 打開
   * Hint: gpio_get_dev, gpio_obj->gpio_open
   * {
   */
  /** Initialize GPIO pointer with ultra sonic GPIO port **/
  pUltraSonicGpio = gpio_get_dev(UltraSonicPort);
  /** Open GPIO pin for ultra sonic **/
  pUltraSonicGpio->gpio_open((1 << UltraSonicPin));
  /**
   * }
   */

  while (1)
  {
    /** Set up UltraSonicPin on pUltraSonicGpio as output **/
    pUltraSonicGpio->gpio_control(GPIO_CMD_SET_BIT_DIR_OUTPUT, (void *)(1 << UltraSonicPin));

    /** Stop timer 0 first in case it has been started before **/
    timer_stop(TIMER_0);
    /** Set up interrupt handler of timer 0 as Timer0_ISR */
    int_handler_install(INTNO_TIMER0, Timer0_ISR);
    /** Set priority of interrupt timer 0 as minimum level */
    int_pri_set(INTNO_TIMER0, INT_PRI_MIN);
    /** Enable timer 0 interrupt **/
    int_enable(INTNO_TIMER0);
    /** Activate timer 0, with interrupt enabled, interrupt every 0.000001(1u) CPU cycles **/
    timer_start(TIMER_0, TIMER_CTRL_IE, 0.000001 * CLK_CPU);
    /** ↑Timer 0 increases by 1 every CPU cycle; thus interrupt every 0.000001 CPU cycles means every 1us **/

    /** Set UltraSonicPin on pUltraSonicGpio as high level **/
    pUltraSonicGpio->gpio_write(1 << UltraSonicPin, 1 << UltraSonicPin);

    /** Set current interrupt times as 0 **/
    CurrentIntTimes = 0;
    /** Set total interrupt times we need as 5 **/
    TotalIntTimes = 5;
    /** ↑Indicating that to reach total interrupt time we need, we will delay for 5 us **/
    /** Idling and waiting for ISR to increase current interrupt times **/
    while (CurrentIntTimes < TotalIntTimes)
      ;

    /* Set UltraSonicPin on pUltraSonicGpio as low level */
    pUltraSonicGpio->gpio_write(0 << UltraSonicPin, 1 << UltraSonicPin);
    /* Stop Timer 0 for next stage */
    timer_stop(TIMER_0);

    /**
		 *  將 pUltraSonicGpio 上的 UltraSonicPin 腳位設定為輸入
		 * {
		 */
    pUltraSonicGpio->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)(1 << UltraSonicPin));
    /**
		 * }
		 */

    /**
		 * 設定一個變數，透過變數讀取 pUltraSonicGpio 上的 UltraSonicPin 腳位的電位
		 * 如果電位為0就持續執行迴圈等待
		 * {
		 */
    uint32_t UltraSonicPinValue = 0;
    while (UltraSonicPinValue == 0)
    {
      pUltraSonicGpio->gpio_read(&UltraSonicPinValue, 1 << UltraSonicPin);
    }
    /**
		 * }
		 */

    /**
		 * 利用timer_start開始計數
		 * 並且持續讀取 pUltraSonicGpio 上的 UltraSonicPin腳位
		 * 如果電位為1就持續執行迴圈等待
		 * {
		 */
    timer_start(TIMER_0, TIMER_CTRL_IP, MAX_COUNT);
    while (UltraSonicPinValue != 0)
    {
      pUltraSonicGpio->gpio_read(&UltraSonicPinValue, 1 << UltraSonicPin);
    }
    /**
		 * }
		 */

    /**
		 * 利用一個變數紀錄 TIMER_0 目前的計數，作為開始到結束經過的 CPU cycle數
		 * 透過兩個計數相差的數量，除以 CPU Cycle(CLK_CPU) 得到秒數
		 * 再乘以 340(m/s)*100(cm/m)/2(來回) 得到公分數
		 * {
		 */
    uint32_t DurationCnt;
    timer_current(TIMER_0, &DurationCnt);
    float Distance = ((float)DurationCnt * 17000) / CLK_CPU;
    /**
		 * }
		 */
    /**
		 * 印出透過超音波測距得到的公分數
		 * {
		 */
    if (Distance > 15)
    {
      lcd_obj->set_Color(GREEN);
    }
    else
    {
      lcd_obj->set_Color(RED);
    }
    lcd_obj->clear();
    lcd_obj->set_CursorPos(0, 0);
    lcd_obj->printf("%d.%d", (int)(Distance), (int)(Distance * 100) % 100);
    /**
		 * }
		 */

    board_delay_ms(200, 0);
  }

  return E_SYS;
}

void Timer0_ISR()
{
  timer_int_clear(TIMER_0);
  if (CurrentIntTimes < TotalIntTimes)
  {
    CurrentIntTimes++;
  }
}
