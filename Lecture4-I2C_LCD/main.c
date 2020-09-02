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

int main(void)
{
  /* 宣告一個 DEV_IIC_PTR 的變數來存放 I2C 物件的指標 */
  DEV_IIC_PTR iic;
  /**
   * 透過 iic_get_dev(uint32_t id) 傳入DFSS_IIC_x_ID 得到 DEV_IIC_PTR 的物件指標
   * 並且將他 assign 給 上面的 iic 變數
   * {
   */
  iic = iic_get_dev(DFSS_IIC_0_ID);

  /**
   * }
   */

  /**
   * 透過 DEV_IIC 裡面的 iic_open 將 iic 開啟成 DEV_MASTER_MODE
   * 同時給予 IIC_SPEED_STANDARD 的傳輸速度即可
   * {
   */
  iic->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
  /**
   * }
   */

  /** 為了讓同學能比較清楚看見出現的字，所以我利用 Lcd_Init_with_I2C_Dev 去初始化背光等等參數 **/
  Lcd_Init_with_I2C_Dev(iic);

  Lcd_Write('H');
  Lcd_Write('e');
  Lcd_Write('l');
  Lcd_Write('l');
  Lcd_Write('o');
  Lcd_Write(',');
  Lcd_Write('W');
  Lcd_Write('o');
  Lcd_Write('r');
  Lcd_Write('l');
  Lcd_Write('d');
  Lcd_Write('!');

  while (1)
  {
  }

  return E_SYS;
}
