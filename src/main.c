#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#include "joystick.h"
#include "led7seg.h"
#include "speaker.h"
#include "oled.h"
#include "snake.h"
#include "rotary.h"
#include "pca9532.h"

#define ROTARY_ROUNDS 30
#define ROTARY_TICK 15

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);
}

static void init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_adc(void)
{
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init ADC pin connect
	 * AD0.5 on P1.31
	 */
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 31;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	/* Configuration for ADC :
	 * 	Frequency at 0,2Mhz
	 *  ADC channel 5, no Interrupt
	 */
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_5,DISABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_5,ENABLE);
}

int main(void)
{
    static uint8_t frame_delay = 120;

    init_i2c();
    init_ssp();
    init_adc();

    joystick_init();
    led7seg_init();
    speaker_init();
    oled_init();
    rotary_init();
    pca9532_init();
    snake_init();

    while(TRUE) {

    	while(!gameover())
    	{
    		// get joystick input
    		switch (joystick_read())
    		{
				case JOYSTICK_CENTER:							  break;
				case JOYSTICK_LEFT: 	snake_chdir(SNAKE_LEFT);  break;
				case JOYSTICK_RIGHT: 	snake_chdir(SNAKE_RIGHT); break;
				case JOYSTICK_UP: 		snake_chdir(SNAKE_UP);    break;
				case JOYSTICK_DOWN: 	snake_chdir(SNAKE_DOWN);  break;

				default: break;
    		}

    		// move and draw the snake
    		snake_move();

			// check rotary
			uint8_t rotary_input;
			for (int i = 0; i < ROTARY_ROUNDS; i++)
			{
				rotary_input = rotary_read();
				
				if (rotary_input == ROTARY_RIGHT)
				{
					if (frame_delay - ROTARY_TICK < 0)
						frame_delay = 0;
					else
						frame_delay -= ROTARY_TICK;
				}

				if (rotary_input == ROTARY_LEFT)
				{
					if (frame_delay + ROTARY_TICK > 255)
						frame_delay = 255;
					else
						frame_delay += ROTARY_TICK;
				}
			}

			// apply the delay
    		Timer0_Wait(frame_delay);
    	}

    	// game over
    	oled_clearScreen(OLED_COLOR_WHITE);
    	oled_putString(5, 4, (const uint8_t*) "Koniec gry!", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    	oled_putString(5, 14, (const uint8_t*) "SW3 - reset", OLED_COLOR_BLACK, OLED_COLOR_WHITE);

		uint8_t sw3_btn = 1;
    	while (sw3_btn != 0)
    	{
    		sw3_btn = ((GPIO_ReadValue(0) >> 4) & 0x01);
    	} 

    	snake_reset();
    }
}
