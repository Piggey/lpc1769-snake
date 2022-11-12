#include "speaker.h"

// forward declarations
uint32_t speaker_get_note(uint8_t ch);
uint32_t speaker_get_duration(uint8_t ch);
uint32_t speaker_get_pause(uint8_t ch);

static uint32_t notes[] = {
	2272, // A - 440 Hz
	2024, // B - 494 Hz
	3816, // C - 262 Hz
	3401, // D - 294 Hz
	3030, // E - 330 Hz
	2865, // F - 349 Hz
	2551, // G - 392 Hz
	1136, // a - 880 Hz
	1012, // b - 988 Hz
	1912, // c - 523 Hz
	1703, // d - 587 Hz
	1517, // e - 659 Hz
	1432, // f - 698 Hz
	1275, // g - 784 Hz
};

void speaker_init()
{
	GPIO_SetDir(2, 1<<0, 1);
	GPIO_SetDir(2, 1<<1, 1);

	GPIO_SetDir(0, 1<<27, 1);
	GPIO_SetDir(0, 1<<28, 1);
	GPIO_SetDir(2, 1<<13, 1);
	GPIO_SetDir(0, 1<<26, 1);

	GPIO_ClearValue(0, 1<<27); //LM4811-clk
	GPIO_ClearValue(0, 1<<28); //LM4811-up/dn
	GPIO_ClearValue(2, 1<<13); //LM4811-shutdn
}

void speaker_play_note(uint32_t note, uint32_t durationMs)
{
	uint32_t t = 0;

	if (note > 0)
	{
		while (t < (durationMs*1000))
		{
			NOTE_PIN_HIGH();
			Timer0_us_Wait(note / 2);
			//delay32Us(0, note / 2);

			NOTE_PIN_LOW();
			Timer0_us_Wait(note / 2);
			//delay32Us(0, note / 2);

			t += note;
		}
	}
	else
		Timer0_Wait(durationMs);
}

void speaker_play_song(uint8_t *song)
{
	/*
	 * A song is a collection of tones where each tone is
	 * a note, duration and pause, e.g.
	 *
	 * "E2,F4,"
	 */

	uint32_t note = 0;
	uint32_t dur  = 0;
	uint32_t pause = 0;

	while(*song != '\0')
	{
		note = speaker_get_note(*song++);
		if (*song == '\0')
			break;

		dur  = speaker_get_duration(*song++);
		if (*song == '\0')
			break;

		pause = speaker_get_pause(*song++);

		speaker_play_note(note, dur);
		Timer0_Wait(pause);
	}
}

uint32_t speaker_get_note(uint8_t ch)
{
    if (ch >= 'A' && ch <= 'G')
        return notes[ch - 'A'];

    if (ch >= 'a' && ch <= 'g')
        return notes[ch - 'a' + 7];

    return 0;
}

uint32_t speaker_get_duration(uint8_t ch)
{
    if (ch < '0' || ch > '9')
        return 400;

    /* number of ms */

    return (ch - '0') * 200;
}

uint32_t speaker_get_pause(uint8_t ch)
{
    switch (ch)
    {
		case '+': return 0;
		case ',': return 5;
		case '.': return 20;
		case '_': return 30;
		default:  return 5;
    }
}


