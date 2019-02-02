/* 
 * Brian Chrzanowski's Terminal Pong
 * Fri Dec 02, 2016 17:00
 */

#include <stm32f4xx.h>
#include "GLCD.h"
#include "I2C.h"
#include "JOY.h"
//#include <ncurses.h>
//#include <string.h>
//#include <unistd.h>

#define DELAY 30000
#define INITIAL -1
#define DOWN 0
#define UP 1

typedef struct paddle {
	/* paddle variables */
	int x;
	int y;    /* y is the 'top' of the paddle */
	int len;
	int score;
} paddle_t;

typedef struct ball {
	/* ball variables */
	int x;
	int y;
	int next_x;
	int next_y;
	int x_vel;
	int y_vel;
} ball_t;

typedef struct dimensions {
	int x;
	int y;
} dimensions_t;

void draw_ball(ball_t *input);
void draw_paddle(paddle_t *paddle);
void draw_score(paddle_t *inpt_paddle, dimensions_t *wall);
void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle);
void paddle_pos(paddle_t *pddl, dimensions_t *wall, char dir);

int wall_collisions(ball_t *usr_ball, dimensions_t *walls);
int kbdhit();
void game_over(paddle_t *inpt_paddle);

void SysTick_Handler (void) {
	static int x = 26;
	static int y = 28;
	
	GLCD_DisplayString(y,x-1,0,"    ");
	GLCD_DisplayString(y,x,0,"EWHA");
	x++;
}

int getch(void){
			static int joy_up_state = INITIAL;
			static int joy_up_count = 0;
			static int joy_down_state = INITIAL;
			static int joy_down_count = 0;
	
			if (JOY_GetKeys() == JOY_UP) {
      if ((++joy_up_count) == 1000) {
      joy_up_count = 0;
   }
   joy_up_state = DOWN;
   }
 else if (JOY_GetKeys() != JOY_UP) {
 if (joy_up_state==DOWN) {
			return 'k';
 joy_up_count = 0;
 joy_up_state= UP;
 }
 }
 
 
  if (JOY_GetKeys() == JOY_DOWN) {
      if ((++joy_down_count) == 1000) {
      joy_down_count = 0;
   }
   joy_down_state = DOWN;
   }
 else if (JOY_GetKeys() != JOY_DOWN) {
 if (joy_down_state==DOWN) {
			return 'j';
 joy_down_count = 0;
 joy_down_state= UP;
 }
 }
 
	 return 0;
}

int main(int argc, char **argv)
{
	/* initialize curses */
	//initscr();
	//noecho();
	//curs_set(0);

	dimensions_t walls = { 0 };
	
	/* set the paddle variables */
	paddle_t usr_paddle = { 0 };

	/* set the ball */
	ball_t usr_ball = { 0 };	
	int keypress = 0;
	int run = 1;
	int i=0;
	int a = 0;
	//getmaxyx(stdscr, walls.y, walls.x); /* get dimensions */
  walls.x = 40;
	walls.y = 40;
	
	usr_paddle.x = 5;
	usr_paddle.y = 11;
	usr_paddle.len = walls.y / 4; usr_paddle.score = 0; 
	usr_ball.x = walls.x / 2;
	usr_ball.y = walls.y / 2;
	usr_ball.next_x = 0;
	usr_ball.next_y = 0;
	usr_ball.x_vel = 1;
	usr_ball.y_vel = 1;
	/* we actually have to store the user's keypress somewhere... */
	
	
	//nodelay(stdscr, TRUE);
	//scrollok(stdscr, TRUE);
	I2C_Init();
	JOY_Init();
  GLCD_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 20);
	GLCD_Clear(White);
	
	GLCD_SetTextColor(Black);
	GLCD_SetBackColor(White);
	
	while (run) {
				a = kbdhit();
				while (a == 0) {
			//getmaxyx(stdscr, walls.y, walls.x);
			walls.x = 40;
			walls.y = 40;
			//clear(); /* clear screen of all printed chars */
			GLCD_Clear(White);
			
			draw_ball(&usr_ball);
			draw_paddle(&usr_paddle);
			draw_score(&usr_paddle, &walls);
				//refresh(); /* draw to term */
			//usleep(DELAY);
			while(i<DELAY){
				i++;
			}
				
			i = 0;
			/* set next positions */
			usr_ball.next_x = usr_ball.x + usr_ball.x_vel;
			usr_ball.next_y = usr_ball.y + usr_ball.y_vel;
			
			/* check for collisions */
			paddle_collisions(&usr_ball, &usr_paddle);
			if (wall_collisions(&usr_ball, &walls)) {
				run = 0;
				break;
			}
			a = kbdhit();
		}
		/* we fell out, get the key press */
		keypress = a;
		switch (keypress) {
		case 'j':
		case 'k':
			paddle_pos(&usr_paddle, &walls, keypress);
			break;

		case 'p': /* pause functionality, because why not */
			//mvprintw(1, 0, "PAUSE - press any key to resume");
			GLCD_DisplayString(1,0,0,"PAUSE - press any key to resume");
			while (getch() != 0) {
				//usleep(DELAY * 7);
				while(i<DELAY*7){
					i++;
				}
					i=0;
			}
			break;

		case 'q':
			run = 0;
			break;

		}
    

	}

	//endwin();

	//printf("GAME OVER\nFinal Score: %d\n", usr_paddle.score);
	game_over(&usr_paddle);
	

	return 0;
}

/*
 * function : paddle_pos
 * purpose  : have a function that will return a proper 'y' value for the paddle
 * input    : paddle_t *inpt_paddle, dimensions_t *wall, char dir
 * output   : void
 */

void paddle_pos(paddle_t *pddl, dimensions_t *wall, char dir)
{
	if (dir == 'j') { /* moving down */
		if (pddl->y + pddl->len + 1 <= wall->y)
			pddl->y++;
	} else {          /* moving up (must be 'k') */
		if (pddl->y - 1 >= 0)
			pddl->y--;

	}

	return;
}

/*
 * function : wall_collisions
 * purpose  : to check for collisions on the terminal walls
 * input    : ball_t *, dimensions_t *
 * output   : nothing (stored within the structs)
 */
int wall_collisions(ball_t *usr_ball, dimensions_t *walls)
{
	/* check if we're supposed to leave quick */
	if (usr_ball->next_x < 0) {
		return 1;
	}

	/* check for X */
	if (usr_ball->next_x >= walls->x) {
		usr_ball->x_vel *= -1;
	} else {
		usr_ball->x += usr_ball->x_vel;
	}

	/* check for Y */
	if (usr_ball->next_y >= walls->y || usr_ball->next_y < 0) {
		usr_ball->y_vel *= -1;
	} else {
		usr_ball->y += usr_ball->y_vel;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle)
{
	/* 
	* simply check if next_% (because we set the next_x && next_y first) 
	* is within the bounds of the paddle's CURRENT position
	*/

	if (inpt_ball->next_x == inpt_paddle->x) {
		if (inpt_paddle->y <= inpt_ball->y &&
			inpt_ball->y <= 
			inpt_paddle->y + inpt_paddle->len) {

			inpt_paddle->score++;
			inpt_ball->x_vel *= -1;
		}
	}

	return;
}

/* -------------------------------------------------------------------------- */

/*
 * functions : draw_ball && draw_paddle
 * purpose   : condense the drawing functions to functions
 * input     : ball_t * && paddle_t *
 * output    : void
 */
void draw_ball(ball_t *input)
{
	//mvprintw(input->y, input->x, "O");
	GLCD_DisplayString(input->y,input->x,0,"O");
	return;
}

void draw_paddle(paddle_t *paddle)
{
	int i;

	for (i = 0; i < paddle->len; i++)
		//mvprintw(paddle->y + i, paddle->x, "|");
		GLCD_DisplayString(paddle->y + i, paddle->x,0,"|");

	return;
}

void draw_score(paddle_t *inpt_paddle, dimensions_t *wall)
{
	char buf[128];
	//mvprintw(0, wall->x / 2 - 7, "Score: %d", inpt_paddle->score);
	sprintf(buf,"Score: %d",inpt_paddle->score);
	GLCD_DisplayString(0,wall->x/2-7,0,buf);

	return;
}

/* -------------------------------------------------------------------------- */

/*
 * function : kbdhit
 * purpose  : find out if we've got something in the input buffer
 * input    : void
 * output   : 0 on none, 1 on we have a key
 */

int kbdhit()
{
	int key = getch();

	if (key == 0) {
		//ungetch(key);
		return 0;
	} else {
		return key;
	}
}

void game_over(paddle_t *usr_paddle) {
	char buf[128];
	sprintf(buf,"GAME OVER  Final Score: %d", usr_paddle->score);
	GLCD_DisplayString(15,26,0,buf);
}
