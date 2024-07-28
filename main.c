#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 800

#define GRID_SIZE 25
#define INIT_LENGTH 4

#define CELL_SIZE (min(window_width, window_height)/GRID_SIZE)
#define INPUT_LEN 256

int window_width, window_height;
int input[256];

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct cell {
    unsigned int x,y;
    struct cell *next, *prev;
} Cell;

typedef struct { 
    Cell *head, *tail;
    Direction dir;
    Direction prev_dir;
} Snake;

typedef struct {
    unsigned int x,y;
} Apple;

int min(int a, int b) {
    return a<b?a:b;
}

void get_window_offset(int *x, int *y) {
    if(window_height > window_width) {
        *y = (window_height-window_width)/2;
        *x = 0;
    } else {
        *x = (window_width-window_height)/2;
        *y = 0;
    }
}

bool is_colliding(unsigned int x, unsigned int y, Cell *snake) 
{
    if(snake == NULL) return false;
    if(snake->x == x && snake->y == y) return true;
    return is_colliding(x, y, snake->next);
}

Apple generate_apple(Snake snake) 
{
    Apple apple;
    do {
        apple.x = rand() % GRID_SIZE;
        apple.y = rand() % GRID_SIZE;
    } while(is_colliding(apple.x, apple.y, snake.head));
    return apple;
}

Snake generate_snake() 
{
    Snake snake;
    Cell* cells = (Cell*)malloc(sizeof(Cell)*INIT_LENGTH);
    for(size_t i=0; i<INIT_LENGTH; i++) {
        Cell* current_cell = cells+i;
        if(i<INIT_LENGTH-1) current_cell->next = current_cell+1;
        else current_cell->next = NULL;
        if(i>0) current_cell->prev = current_cell-1;
        else current_cell->prev = NULL;
        current_cell->x = GRID_SIZE/2-i;
        current_cell->y = GRID_SIZE/2; 
    }
    snake.head = cells;
    snake.tail = cells+INIT_LENGTH-1;
    snake.dir  = RIGHT;
    snake.prev_dir = RIGHT;
    return snake;
}

void draw_snake(SDL_Renderer *renderer, Snake snake)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0XFF, 0x00, 0xFF);
    Cell* current = snake.head;
    while(current) {
        int offset_x, offset_y;
        get_window_offset(&offset_x, &offset_y);
        SDL_Rect cell_square = {.w=CELL_SIZE,.h=CELL_SIZE,.x=offset_x+(current->x)*CELL_SIZE,.y=offset_y+(current->y)*CELL_SIZE};
        SDL_RenderFillRect(renderer, &cell_square);
        current = current->next;
    }
}

void draw_apple(SDL_Renderer *renderer, Apple apple) 
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    int offset_x, offset_y;
    get_window_offset(&offset_x, &offset_y);
    SDL_Rect cell_square = {.w=CELL_SIZE,.h=CELL_SIZE,.x=offset_x+(apple.x)*CELL_SIZE,.y=offset_y+(apple.y)*CELL_SIZE};
    SDL_RenderFillRect(renderer, &cell_square);
}

void move_snake(Snake *snake) {
    Cell* new_tail = snake->tail->prev;
    Cell* new_head = snake->tail;
    new_tail->next = NULL;
    snake->tail = new_tail;
    new_head->next = snake->head;
    snake->head->prev = new_head;
    snake->head = new_head;
    switch (snake->dir) {
        case UP:
            snake->head->x = snake->head->next->x;
            snake->head->y = snake->head->next->y -1;
        break;
        case DOWN:
            snake->head->x = snake->head->next->x;
            snake->head->y = snake->head->next->y +1;
        break;
        case LEFT:
            snake->head->x = snake->head->next->x -1;
            snake->head->y = snake->head->next->y;
        break;
        case RIGHT:
            snake->head->x = snake->head->next->x +1;
            snake->head->y = snake->head->next->y;
        break;
    }
}

void handle_key_press(Snake *snake) 
{
    if(input[SDL_SCANCODE_UP] && snake->prev_dir != DOWN) {
        snake->dir = UP;
        input[SDL_SCANCODE_UP] = false;
    } else if (input[SDL_SCANCODE_DOWN] && snake->prev_dir != UP) {
        snake->dir = DOWN;
        input[SDL_SCANCODE_DOWN] = false;
    } else if (input[SDL_SCANCODE_RIGHT] && snake->prev_dir != LEFT) {
        snake->dir = RIGHT;
        input[SDL_SCANCODE_RIGHT] = false;
    } else if (input[SDL_SCANCODE_LEFT] && snake->prev_dir != RIGHT) {
        snake->dir = LEFT;
        input[SDL_SCANCODE_LEFT] = false;
    }
}

double delta_time(clock_t t1, clock_t t2) 
{
    return ((double)t1-t2)/CLOCKS_PER_SEC;
}

int main() 
{
    srand(time(0));
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL\n");
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SNAKE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_HEIGHT, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE); 
    if(!window) {
        printf("Couldn't create the window\n");
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        printf("Couldn't create the renderer\n");
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    Snake snake = generate_snake();
    Apple apple = generate_apple(snake);

    bool stay_open = true;
    clock_t current_tick = clock();
    while(stay_open) {
        SDL_Event e;
        SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xFF);
        SDL_RenderClear(renderer);
        SDL_GetWindowSize(window, &window_width, &window_height);
        draw_snake(renderer, snake);
        draw_apple(renderer,apple);
        handle_key_press(&snake);
        while(SDL_PollEvent(&e) > 0) {
            switch(e.type) {
                case SDL_QUIT:
                    stay_open = false;
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    break;
                case SDL_KEYDOWN:
                    input[e.key.keysym.scancode] = true;
                break;
            }
        }
        SDL_RenderPresent(renderer);
        clock_t current_time = clock();
        if(delta_time(current_time,current_tick) > 0.1) {
            move_snake(&snake);
            current_tick = current_time;
            snake.prev_dir = snake.dir;
        }
    }

    SDL_Quit();
    return 0;
}
