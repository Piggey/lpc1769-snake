#include "snake.h"
#include "speaker.h"
#include "led7seg.h"
#include "pca9532.h"
#include "random.h"
#include "oled.h"

#include <stdlib.h>

static Snake* snake;
static uint16_t snake_length;
static Bool first_run;
static const char* sound_fruit_eaten = "E2,A2,A2";
static const char* sound_gameover = "B1,A1,F1,F1";

// forward declarations
void move_to_front(const int8_t dx, const int8_t dy);
void snake_eat(void);
void snake_draw(oled_color_t color);
void snake_draw_node(SnakeNode* node, oled_color_t color);
void snake_draw_food(void);
void set_7seg_display(void);
void set_red_led(void);
void draw_oled_boundaries(void);
void clear_red_leds(void);

/*!
 *  @brief    inicjalizacja gry
 *  @returns  void
 *  @side effects:
 *            brak
 */
void snake_init(void)
{
	snake = (Snake*) malloc(sizeof(Snake));
	first_run = TRUE;
	snake_reset();
}

/*!
 *  @brief    przywraca wszystkie wartosci gry do stanu poczatkowego
 *  @returns  void
 *  @side effects:
 *            brak
 */
void snake_reset(void)
{
	// free the memory from the previous game
	if (first_run == FALSE)
	{
		SnakeNode* node = snake->head;
		SnakeNode* tmp;
		while (node != NULL)
		{
			tmp = node;
			node = node->next;
			free(tmp);
		}
	}

	// init the snake and fruit positions
	snake_length = 2;
	SnakeNode* head = (SnakeNode*) malloc(sizeof(SnakeNode));
	SnakeNode* nxt = (SnakeNode*) malloc(sizeof(SnakeNode));

	head->x = 4;
	head->y = BOARD_HEIGHT / 2;
	head->next = nxt;

	nxt->x = 3;
	nxt->y = BOARD_HEIGHT / 2;
	nxt->next = NULL;

	snake->head = head;
	snake->current_direction = SNAKE_NOT_ASSIGNED;
	snake->fruit_x = BOARD_WIDTH  - 4;
	snake->fruit_y = BOARD_HEIGHT / 2;

	// draw snake
	oled_clearScreen(OLED_COLOR_BLACK);
	draw_oled_boundaries();
	snake_draw(OLED_COLOR_WHITE);

	// draw food
	snake_draw_food();

	// reset 7 segment display and LEDs
	set_7seg_display();
	clear_red_leds();
	set_red_led();

	first_run = FALSE;
}

/*!
 *  @brief    sprawdzenie, czy gracz nie przegral
 *  @returns  TRUE, jezeli koniec gry, w przeciwnym wypadku FALSE
 *  @side effects:
 *            brak
 */
Bool gameover(void)
{
	// returns TRUE if gameover

	uint8_t head_x = snake->head->x;
	uint8_t head_y = snake->head->y;

	// check if the snake's head is hitting the wall
	if (head_x == 0 || head_x >= BOARD_WIDTH || head_y == 0 || head_y >= BOARD_HEIGHT)
	{
		speaker_play_song((uint8_t*) sound_gameover);
		return TRUE;
	}


	// check if snake's head is hitting any other snake node.
	SnakeNode* node = snake->head->next;
	while (node->next != NULL)
	{
		uint8_t node_x = node->x;
		uint8_t node_y = node->y;

		if (head_x == node_x && head_y == node_y)
		{
			speaker_play_song((uint8_t*) sound_gameover);
			return TRUE;
		}

		node = node->next;
	}

	return FALSE;
}

/*!
 *  @brief    poruszenie oraz narysowanie na ekranie weza
 *  @returns  void
 *  @side effects:
 *            dodatkowe wywolanie funkcji snake_eat
 */
void snake_move(void)
{
	switch (snake->current_direction)
	{
		case SNAKE_NOT_ASSIGNED:
			break;

		case SNAKE_LEFT:
			move_to_front(-1, 0);
			break;

		case SNAKE_RIGHT:
			move_to_front(1, 0);
			break;

		case SNAKE_UP:
			move_to_front(0, -1);
			break;

		case SNAKE_DOWN:
			move_to_front(0, 1);
			break;
	}

	// eat the fruit if possible
	snake_eat();
}

/*!
 *  @brief    ustawia nowy kierunek poruszania sie weza
 *  @param new_direction
 *             nowy kierunek, w ktorym chcemy sie poruszac, 
 *
 *  @returns  void
 *  @side effects:
 *            w przypadku, gdy nowy ruch jest przeciwny do obecnego ruchu
 *            ruch nie zostanie zmieniony
 */
void snake_chdir(snake_directions new_direction)
{
	switch (snake->current_direction)
	{
		case SNAKE_NOT_ASSIGNED:
			snake->current_direction = new_direction;
			break;

		case SNAKE_LEFT:
			if (new_direction == SNAKE_RIGHT)
				break;

			snake->current_direction = new_direction;
			break;

		case SNAKE_RIGHT:
			if (new_direction == SNAKE_LEFT)
				break;

			snake->current_direction = new_direction;
			break;

		case SNAKE_UP:
			if (new_direction == SNAKE_DOWN)
				break;

			snake->current_direction = new_direction;
			break;

		case SNAKE_DOWN:
			if (new_direction == SNAKE_UP)
				break;

			snake->current_direction = new_direction;
			break;
	}
}

/*!
 *  @brief    obecny wynik gracza (liczbe zjedzonych owocow)
 *  @returns  liczba zjedzonych owocow w tej grze
 *  @side effects:
 *            brak
 */
uint16_t snake_score(void)
{
	return snake_length - 2;
}

/*!
 *  @brief    zamienia ostatniego SnakeNode na glowe weza z przeniesiem
 *  @param dx
 *             o ile ma zostac przeniesiona glowa weza w plaszczyznie x
 *  @param dy 
 *             o ile ma zostac przeniesiona glowa weza w plaszczyznie y
 *  @returns  void
 *  @side effects:
 *            brak
 */
void move_to_front(const int8_t dx, const int8_t dy)
{
	SnakeNode* sec_last = snake->head;
	SnakeNode* last = snake->head;
	while (last->next != NULL)
	{
		sec_last = last;
		last = last->next;
	}

	// fade out old node
	snake_draw_node(last, OLED_COLOR_BLACK);

	last->x = snake->head->x + dx;
	last->y = snake->head->y + dy;

	sec_last->next = NULL;
	last->next = snake->head;
	snake->head = last;

	// draw the new head node
	snake_draw_node(snake->head, OLED_COLOR_WHITE);
}

/*!
 *  @brief    proba zjedzenia owocu
 *  @returns  void
 *  @side effects:
 *            funkcja sprawdzana co kazde poruszenie sie weza
 */
void snake_eat(void)
{
	if (snake->head->x == snake->fruit_x && snake->head->y == snake->fruit_y)
	{
		snake_length++;

		// play cool song (very cool)
		speaker_play_song((uint8_t*) sound_fruit_eaten);

		// flash green LEDs
		pca9532_setLeds(0xff00, 0x0000);
		Timer0_Wait(200);
		pca9532_setLeds(0x0000, 0xff00);

		set_7seg_display();
		set_red_led();

		// get the last node before eating
		SnakeNode* last = snake->head;
		while (last->next != NULL)
			last = last->next;

		// add new node
		SnakeNode* new_node = (SnakeNode*) malloc(sizeof(SnakeNode));
		new_node->x = last->x;
		new_node->y = last->y;
		new_node->next = NULL;

		last->next = new_node;
		last = new_node;

		// generate and draw new fruit
		snake->fruit_x = random_u8(2, BOARD_WIDTH - 2);
		snake->fruit_y = random_u8(2, BOARD_HEIGHT - 2);
		snake_draw_food();
	}
}

/*!
 *  @brief    rysuje na ekranie OLED pojedynczy SnakeNode
 *  @param node
 *             SnakeNode do narysowania
 *  @param color
 *             kolor, w ktorym ma byc narysowany SnakeNode
 *  @returns  void
 *  @side effects:
 *            brak
 */
void snake_draw_node(SnakeNode* node, oled_color_t color)
{
	uint8_t draw_x = node->x * SNAKE_SIZE;
	uint8_t draw_y = node->y * SNAKE_SIZE;
	oled_fillRect(draw_x, draw_y, draw_x + SNAKE_SIZE, draw_y + SNAKE_SIZE, color);
}

/*!
 *  @brief    rysuje na ekranie OLED wszystkie obecne SnakeNode 
 *  @param color
 *             kolor, w ktorym ma byc narysowany waz
 *  @returns  void
 *  @side effects:
 *            brak
 */
void snake_draw(oled_color_t color)
{
	SnakeNode* node = snake->head;
	while(node != NULL)
	{
		snake_draw_node(node, color);
		node = node->next;
	}
}

/*!
 *  @brief    rysuje owoc na wyswietlaczu OLED
 *  @returns  void
 *  @side effects:
 *            owoc rysowany jest na wyswietlaczu tylko raz,
 * 			  mozliwe jest wiec narysowanie owocu "pod wezem"
 * 			  przez co pozniej staje sie on niewidoczny
 */
void snake_draw_food(void)
{
	uint8_t draw_x = snake->fruit_x * SNAKE_SIZE;
	uint8_t draw_y = snake->fruit_y * SNAKE_SIZE;
	oled_fillRect(draw_x, draw_y, draw_x + SNAKE_SIZE, draw_y + SNAKE_SIZE, OLED_COLOR_WHITE);
}

/*!
 *  @brief    wyswietla ostatnia cyfre wyniku na 7-segmentowym wyswietlaczu
 *  @returns  void
 *  @side effects:
 *            brak
 */
void set_7seg_display(void)
{
	uint8_t last_digit = ((snake_length - 2) % 10) + '0';
	led7seg_setChar(last_digit, FALSE);
}

/*!
 *  @brief    zapala czerwone LEDy, zgodnie z wynikiem gry
 *  @returns  void
 *  @side effects:
 *            zgodnie z zalozeniem po zapaleniu wszystkich LEDow
 * 			  powinny sie one odpowiednio wylaczac
 * 		   	  aby to sprawdzic jednak trzeba byc dobrym w te gre
 * 			  i tutaj pojawia sie problem
 */
void set_red_led(void)
{
	// sets red pca9532 LEDs depending on the snake_length
	// enable LED each time snake_length wraps around
	uint8_t led = ((snake_length - 2) / 10) % 16;
	
	if (led < 8)
	{
		// skip if the LED is already turned on
		if ((pca9532_getLedState(FALSE) & (1 << led)) > 0)
			return;

		// turn on LED
		pca9532_setLeds((1 << led), 0x0000);
	}
	else
	{
		led -= 8;

		// skip if the LED is already turned off
		if ((pca9532_getLedState(FALSE) & (1 << led)) == 0)
			return;

		// turn off LED
		pca9532_setLeds(0x0000, (1 << led));
	}
}

/*!
 *  @brief    rysuje bariery na rogach wyswietlacza OLED
 *  @returns  void
 *  @side effects:
 *            bariery rysowane sa tylko raz
 */
void draw_oled_boundaries(void)
{
	oled_rect(0, 0, OLED_DISPLAY_WIDTH - 1, OLED_DISPLAY_HEIGHT - 1, OLED_COLOR_WHITE);
}

/*!
 *  @brief    wylacz wszystkie czerwone LEDy
 *  @returns  void
 *  @side effects:
 *            brak
 */
void clear_red_leds(void)
{
	pca9532_setLeds(0x0000, 0x00ff);
}
