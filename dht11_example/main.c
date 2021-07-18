/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup bsp_example_main main.c
 * @{
 * @ingroup bsp_example
 * @brief BSP Example Application main file.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "nrf_error.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_delay.h"

#define BUTTON_PREV_ID           0                           /**< Button used to switch the state. */
#define BUTTON_NEXT_ID           1                           /**< Button used to switch the state. */

#define DHT11_PIN NRF_GPIO_PIN_MAP(0,31)
float dht11_temp=0.0;
float dht11_hum=0.0;


static void dht11_read()
{
  char data_buf[5];
  char ch;
  int i,u,stage,error=0;

  for (i=0;i<5;i++)
  {
    data_buf[i]=0;
  }

  //Ask for the data
  nrf_gpio_cfg_input(DHT11_PIN,  NRF_GPIO_PIN_PULLUP);
  nrf_delay_us(50);
  nrf_gpio_cfg_output(DHT11_PIN);
  nrf_gpio_pin_write(DHT11_PIN,0);
  nrf_delay_ms(20);
  nrf_gpio_cfg_input(DHT11_PIN,  NRF_GPIO_PIN_PULLUP);
  nrf_delay_us(40);

  //wait for the reply
  for (i=0;i<30;i++)
  {
    if(stage==0)
    {
        if(nrf_gpio_pin_read(DHT11_PIN)==0)
        {
            stage=1;
        }
    }
    else
    {
        if(nrf_gpio_pin_read(DHT11_PIN)>0)
        {
            stage=2;
            break;
        }
    }
    nrf_delay_us(5);
  }

  if(stage==2) //Got correct reply
  {
      //nrf_gpio_cfg_input(DHT11_PIN,  NRF_GPIO_PIN_PULLUP);
      for (i=0;i<5;i++)
      {
        ch=0;
        for(u=7;u>=0;u--)
        {
          
          //wait until lvl will be low
          stage=0;
          while(nrf_gpio_pin_read(DHT11_PIN)>0)
          {
            stage++;
            nrf_delay_us(5);
            if(stage>20) //looks like something wrong, we're stuck
            {
                NRF_LOG_ERROR("Error reading data from DHT11. Error waiting for 52us low period \r\n");
                error=1;
                break;
            }
          }
          stage=0;
          //here lvl should be low, wait for it will be high
          while(nrf_gpio_pin_read(DHT11_PIN)==0)
          {
            stage++;
            nrf_delay_us(5);
            if(stage>30) //looks like something wrong, we're stuck
            {
                NRF_LOG_ERROR("Error reading data from DHT11. Error waiting for bit high period \r\n");
                error=1;
                break;
            }
          }

          //Let's wait for 30 us and check bit lvl
          nrf_delay_us(30);

          //record the data
          if(nrf_gpio_pin_read(DHT11_PIN)>0)
          {
            ch |= 1 << u; //There is 1 on the bus
          }
          else
          {
            ch &= ~(1 << u); //There us 0 on the bus
          }

        }
        data_buf[i]=ch;
      }

  }
  else
  {
    NRF_LOG_ERROR("Got no valid reply from DHT11. \r\n");
    error=1;
  }

  //read the results
  if(error==0)
  {
      /*for (i=0;i<5;i++)
      {
        NRF_LOG_INFO("Data[%d]: %d. \r\n",i,data_buf[i]);
      }*/
      dht11_temp=(int)(data_buf[2])*1.0+(int)(data_buf[3])*0.1;
      dht11_hum=(int)(data_buf[0])*1.0;
  }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint8_t buf_t[128], buf_h[128];

    uint32_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("BSP example started.");

    //bsp_configuration();
    NRF_LOG_FLUSH();
    while (true)
    {
        dht11_temp=-100;
        dht11_read();
        if(dht11_temp!=-100)//Only send the data if there is no errors
        {
            //NRF_LOG seems to be not working with floats. Here is the small WA.
            snprintf(buf_t, sizeof(buf_t), "%.1f", dht11_temp);
            snprintf(buf_h, sizeof(buf_h), "%.1f", dht11_hum);
            NRF_LOG_INFO("DHT11 data: temp: %s , hum: %s  \r\n",nrf_log_push(buf_t),nrf_log_push(buf_h));
        }
        else
        {
            NRF_LOG_ERROR("Error reading data from DHT11 \r\n");
        }
        NRF_LOG_FLUSH();
        nrf_delay_ms(5000);
    }
}


/** @} */
