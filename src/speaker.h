#ifndef SPEAKER_H_
#define SPEAKER_H_

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

void speaker_init(void);
void speaker_play_note(uint32_t note, uint32_t durationMs);
void speaker_play_song(uint8_t *song);

#endif
