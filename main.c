#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14

enum game_state
{
	GAME_STATE_MENU,
	GAME_STATE_PLAY,
	GAME_STATE_GAMEOVER
};

struct v2
{
	float x;
	float y;
};

struct particle
{
	SDL_Rect rect;
	SDL_Color color;
	struct v2 speed;
	struct v2 position;
	int active;
	int lifespan;
};

struct pad
{
	SDL_Rect rect;
	struct v2 speed;
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
	struct particle particles[100];
	int move_right;
	int move_left;
	int accept;
	unsigned int fps;
	unsigned int delay_time;
	enum game_state state;
	TTF_Font *font;
	unsigned int score_player;
	unsigned int score_enemy;
};

void
move_ball_to_center (struct game *g)
{
	g->ball.rect.x = g->window_width / 2 - g->ball.rect.w / 2;
	g->ball.rect.y = g->window_height / 2 - g->ball.rect.h / 2;
	g->ball.speed.x = -g->ball.speed.x;
	g->ball.speed.y = -g->ball.speed.y;
}

void
init (struct game *g)
{
	srand(time(NULL));
	int pad_width = 64;
	int pad_height = 16;
	int ball_diameter = 8;
	g->running = 1;
	g->window_width = 1280;
	g->window_height = 720;
	struct pad player
		= { { g->window_width / 2 + pad_width / 2,
			  g->window_height - pad_height * 1.5, pad_width, pad_height },
			{ 0, 0 } };
	struct pad enemy = { { g->window_width / 2 - pad_width / 2,
						   0 + pad_height / 2, pad_width, pad_height },
						 { 0, 0 } };
	struct ball ball = { { g->window_width / 2 - ball_diameter / 2,
						   g->window_height / 2 - ball_diameter / 2,
						   ball_diameter, ball_diameter },
						 { 2, 3 } };
	g->player = player;
	g->enemy = enemy;
	g->ball = ball;
	SDL_Init (SDL_INIT_VIDEO);
	TTF_Init ();
	g->window = SDL_CreateWindow (g->title, SDL_WINDOWPOS_CENTERED,
								  SDL_WINDOWPOS_CENTERED, g->window_width,
								  g->window_height, SDL_WINDOW_SHOWN);
	g->renderer = SDL_CreateRenderer (g->window, -1, 0);
	g->fps = 60;
	g->delay_time = 1000.0f / g->fps;
	g->state = GAME_STATE_MENU;
	g->font = TTF_OpenFont ("Sans.ttf", 24);
	g->score_player = 0;
	g->score_enemy = 0;
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
							g->player.speed.x = -speed;
						}
					else if (g->move_right
							 && g->player.rect.x + g->player.rect.w
									< g->window_width)
						{
							g->player.speed.x = speed;
						}
					else
						{
							g->player.speed.x = 0;
						}
					g->player.rect.x += g->player.speed.x;
				}
				// enemy
				{
					int speed = 2;
					int diff_between_positions
						= g->ball.rect.x - g->enemy.rect.x;
					g->enemy.rect.x += diff_between_positions
									   / abs (diff_between_positions) * speed;
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
							g->ball.speed.x += g->player.speed.x;
							int number_of_requested_particles = 9;
							for (int i = 0; i < 100; i++)
								{
									if (g->particles[i].active)
										continue;
									g->particles[i].active = 1;
									g->particles[i].lifespan = 25;
									g->particles[i].color.r = 255;
									g->particles[i].color.g = 255;
									g->particles[i].color.b = 255;
									g->particles[i].color.a = 255;
									g->particles[i].rect.w = 2;
									g->particles[i].rect.h = 2;
									g->particles[i].position.x = g->ball.rect.x;
									g->particles[i].position.y = g->ball.rect.y;
									float angle
										= (i + 1) * 360
										  / number_of_requested_particles * PI
										  / 180;
									int speed = rand() % 5;
									g->particles[i].speed.x = cos (angle) * speed;
									g->particles[i].speed.y = sin (angle) * speed;
									number_of_requested_particles--;
									if (number_of_requested_particles <= 0)
										break;
								}
						}
					if (g->ball.rect.x + g->ball.rect.w > g->window_width
						|| g->ball.rect.x < 0)
						{
							g->ball.speed.x = -g->ball.speed.x;
						}
					if (g->ball.rect.y + g->ball.rect.h < 0)
						{
							g->score_player++;
							move_ball_to_center (g);
						}
					if (g->ball.rect.y > g->window_height)
						{
							g->score_enemy++;
							move_ball_to_center (g);
						}
					g->ball.speed.x
						= g->ball.speed.x > 4 ? 4 : g->ball.speed.x;
					g->ball.speed.x
						= g->ball.speed.x < -4 ? -4 : g->ball.speed.x;
					g->ball.speed.y
						= g->ball.speed.y > 4 ? 4 : g->ball.speed.y;
					g->ball.speed.y
						= g->ball.speed.y < -4 ? -4 : g->ball.speed.y;
					g->ball.rect.x += g->ball.speed.x;
					g->ball.rect.y += g->ball.speed.y;
				}
				// particles
				{
					for (int i = 0; i < 100; i++)
						{
							if (g->particles[i].active)
								{
									g->particles[i].lifespan--;
									g->particles[i].active
										= g->particles[i].lifespan <= 0 ? 0
																		: 1;
									g->particles[i].position.x
										+= g->particles[i].speed.x;
									g->particles[i].position.y
										+= g->particles[i].speed.y;
									g->particles[i].rect.x = (int) g->particles[i].position.x;
									g->particles[i].rect.y = (int) g->particles[i].position.y;
								}
						}
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
	SDL_Color White = { 255, 255, 255, 255 };
	char score_string[50];
	sprintf (score_string, "%d:%d", g->score_player, g->score_enemy);
	SDL_Surface *surfaceMessage
		= TTF_RenderText_Solid (g->font, score_string, White);
	SDL_Texture *Message
		= SDL_CreateTextureFromSurface (g->renderer, surfaceMessage);
	SDL_Rect Message_rect = { 0, 0, 100, 50 };
	SDL_RenderCopy (g->renderer, Message, NULL, &Message_rect);
	SDL_FreeSurface (surfaceMessage);
	SDL_DestroyTexture (Message);
	for (int i = 0; i < 100; i++)
		{
			if (g->particles[i].active)
				{
					SDL_SetRenderDrawColor (
						g->renderer, g->particles[i].color.r,
						g->particles[i].color.g, g->particles[i].color.b,
						g->particles[i].color.a);
					SDL_RenderDrawRect (g->renderer, &g->particles[i].rect);
				}
		}
	SDL_RenderPresent (g->renderer);
}

void
die (struct game *g)
{
	SDL_DestroyRenderer (g->renderer);
	SDL_DestroyWindow (g->window);
	TTF_Quit ();
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
