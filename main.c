#include <SDL2/SDL.h>

enum game_state
{
	GAME_STATE_MENU,
	GAME_STATE_PLAY,
	GAME_STATE_GAMEOVER
};

struct v2
{
	int x;
	int y;
};

struct pad
{
	SDL_Rect rect;
};

struct ball
{
	SDL_Rect rect;
	struct v2 speed;
};

struct game
{
	int running;
	char title[4];
	int window_height;
	int window_width;
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct pad player;
	struct pad enemy;
	struct ball ball;
	int move_right;
	int move_left;
	int accept;
	int fps;
	int delay_time;
	enum game_state state;
};

void
init (struct game *g)
{
	int pad_width = 32;
	int pad_height = 16;
	int ball_diameter = 8;
	g->running = 1;
	g->window_width = 640;
	g->window_height = 480;
	struct pad player
		= { { g->window_width / 2 + pad_width / 2,
			  g->window_height - pad_height * 1.5, pad_width, pad_height } };
	struct pad enemy = { { g->window_width / 2 - pad_width / 2,
						   0 + pad_height / 2, pad_width, pad_height } };
	struct ball ball = { { g->window_width / 2 - ball_diameter / 2,
						   g->window_height / 2 - ball_diameter / 2,
						   ball_diameter, ball_diameter },
						 { 2, 3 } };
	g->player = player;
	g->enemy = enemy;
	g->ball = ball;
	SDL_Init (SDL_INIT_VIDEO);
	g->window = SDL_CreateWindow (g->title, SDL_WINDOWPOS_CENTERED,
								  SDL_WINDOWPOS_CENTERED, g->window_width,
								  g->window_height, SDL_WINDOW_SHOWN);
	g->renderer = SDL_CreateRenderer (g->window, -1, 0);
	g->fps = 60;
	g->delay_time = 1000.0f / g->fps;
	g->state = GAME_STATE_MENU;
}

void
input (struct game *g)
{
	SDL_Event e;
	while (SDL_PollEvent (&e))
		{
			if (e.type == SDL_QUIT
				|| (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE))
				g->running = 0;
			if (e.type == SDL_KEYDOWN)
				{
					switch (e.key.keysym.sym)
						{
						case SDLK_LEFT:
							g->move_left = 1;
							break;
						case SDLK_RIGHT:
							g->move_right = 1;
							break;
						case SDLK_RETURN:
							g->accept = 1;
							break;
						}
				}
			if (e.type == SDL_KEYUP)
				{
					switch (e.key.keysym.sym)
						{
						case SDLK_LEFT:
							g->move_left = 0;
							break;
						case SDLK_RIGHT:
							g->move_right = 0;
							break;
						case SDLK_RETURN:
							g->accept = 0;
							break;
						}
				}
		}
}

void
update (struct game *g)
{
	switch (g->state)
		{
		case GAME_STATE_MENU:
			{
				if (g->accept)
					g->state = GAME_STATE_PLAY;
			}
			break;
		case GAME_STATE_PLAY:
			{
				// player
				{
					int speed = 4;
					if (g->move_left && g->player.rect.x > 0)
						{
							g->player.rect.x -= speed;
						}
					if (g->move_right
						&& g->player.rect.x + g->player.rect.w
							   < g->window_width)
						{
							g->player.rect.x += speed;
						}
				}
				// enemy
				{
					int speed = 2;
					g->enemy.rect.x
						+= ((g->ball.rect.x - g->enemy.rect.x) - (-1))
						   / (1 - (-1)) * speed;
					if (g->enemy.rect.x < 0)
						{
							g->enemy.rect.x = 0;
						}
					if (g->enemy.rect.x + g->enemy.rect.w > g->window_width)
						{
							g->enemy.rect.x
								= g->window_width - g->enemy.rect.w;
						}
				}
				// ball
				{
					if (SDL_HasIntersection (&g->ball.rect, &g->player.rect)
						|| SDL_HasIntersection (&g->ball.rect, &g->enemy.rect))
						{
							g->ball.speed.y = -g->ball.speed.y;
							// TODO: Add impulse from player.
						}
					if (g->ball.rect.x + g->ball.rect.w > g->window_width
						|| g->ball.rect.x < 0)
						{
							g->ball.speed.x = -g->ball.speed.x;
						}
					if (g->ball.rect.y + g->ball.rect.h < 0
						|| g->ball.rect.y > g->window_height)
						{
							g->ball.rect.x
								= g->window_width / 2 - g->ball.rect.w / 2;
							g->ball.rect.y
								= g->window_height / 2 - g->ball.rect.h / 2;
							g->ball.speed.x = -g->ball.speed.x;
							g->ball.speed.y = -g->ball.speed.y;
						}
					g->ball.rect.x += g->ball.speed.x;
					g->ball.rect.y += g->ball.speed.y;
				}
			}
			break;
		case GAME_STATE_GAMEOVER:
			break;
		}
}

void
display (struct game *g)
{
	SDL_SetRenderDrawColor (g->renderer, 25, 25, 50, 255);
	SDL_RenderClear (g->renderer);
	SDL_SetRenderDrawColor (g->renderer, 125, 175, 125, 255);
	SDL_RenderDrawRect (g->renderer, &g->player.rect);
	SDL_SetRenderDrawColor (g->renderer, 175, 125, 125, 255);
	SDL_RenderDrawRect (g->renderer, &g->enemy.rect);
	SDL_SetRenderDrawColor (g->renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect (g->renderer, &g->ball.rect);
	SDL_RenderPresent (g->renderer);
}

void
die (struct game *g)
{
	SDL_DestroyRenderer (g->renderer);
	SDL_DestroyWindow (g->window);
	SDL_Quit ();
}

int
main ()
{
	struct game game;
	unsigned int frame_start;
	unsigned int frame_time;
	init (&game);
	do
		{
			frame_start = SDL_GetTicks ();
			input (&game);
			update (&game);
			display (&game);
			frame_time = SDL_GetTicks () - frame_start;
			if (frame_time < game.delay_time)
				SDL_Delay ((int)(game.delay_time - frame_time));
		}
	while (game.running);
	die (&game);
}
