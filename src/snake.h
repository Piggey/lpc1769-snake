#ifndef SNAKE_H_
#define SNAKE_H_

#include <lpc_types.h>

// OLED size is 96x64, board is 1/4th of that
#define BOARD_WIDTH 24
#define BOARD_HEIGHT 16

// snake display size
#define SNAKE_SIZE 4

typedef enum
{
	SNAKE_NOT_ASSIGNED,
	SNAKE_LEFT,
	SNAKE_RIGHT,
	SNAKE_UP,
	SNAKE_DOWN
} snake_directions;

// structure of one "node" of snake's body, holds a pointer to the next and previous node
typedef struct SnakeNode SnakeNode;
struct SnakeNode
{
	uint8_t x;
	uint8_t y;
	SnakeNode* next;
};

// main snake structure
typedef struct
{
	SnakeNode* head;
	snake_directions current_direction;

	uint8_t fruit_x;
	uint8_t fruit_y;

} Snake;

void snake_init(void);
void snake_reset(void);
Bool gameover(void);
void snake_move(void);
void snake_chdir(snake_directions new_direction);
uint16_t snake_score(void);

#endif
