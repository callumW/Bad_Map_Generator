#include <SDL2/SDL.h>

#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <cstdint>
#include <string>

#include <chrono>
#include <random>

#include "Logger.h"

#define PIXEL_LENGTH 1

struct Pixel {
    SDL_Rect rect;
    int ID;
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

/* States */
bool running = true;
bool reload = true;
bool greyscale_reload = false;
bool load_map = false;

/* Random stuff */
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 random_num{seed};

std::uniform_int_distribution<Uint8> dist(0, 255);


void fill(Pixel* map, int width, int height);
void fill_greyscale(Pixel* map, int width, int height);

void fill_with_map(Pixel* map, int width, int height);
void render(const Pixel* map, int width, int height, SDL_Renderer* rend);
Uint8 get_num();
void clear(SDL_Renderer* r);

void noisify(Pixel* map, int width, int height);

void handle_input();

int main(int argc, char* argv[])
{
    /** Screen Variables **/
    constexpr int screen_width = 1024;
    constexpr int screen_height = 768;
    constexpr int map_width = screen_width / PIXEL_LENGTH;
    constexpr int map_height = screen_height / PIXEL_LENGTH;

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
        LOG("Could not initialise SDL");
        LOG("Error: ");
        LOG(std::string{SDL_GetError()});
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Bad Map Generator!", 40, 40,
        screen_width, screen_height, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        LOG("Could not create SDL_Window");
        LOG("Error: ");
        LOG(std::string{SDL_GetError()});
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL) {
        LOG("Could not create window");
        LOG("Error: ");
        LOG(std::string{SDL_GetError()});
        return 1;
    }

    /*
    Technically we don't need to malloc this but because the array is
    potentially large it is safer to do so. If we don't we run the risk of
    causing a stack overflow.
    */
    Pixel* map = (Pixel*) malloc(sizeof(Pixel) * map_width * map_height);

    if (map == nullptr) {
        LOG("Failed to create screen array!");
        return 1;
    }

    Uint32 current_time = 0;
    Uint32 frame_check_time = 0;   //if 0 calculating the fps may
                                                // trigger on the first loop
    long frames = 0;

    fill(map, map_width, map_height);
    frame_check_time = SDL_GetTicks();
    LOG("Looping...");
    while (running) {
        /** Handle Time stuff **/
        current_time = SDL_GetTicks();

        if (current_time - frame_check_time > 1000) {
            std::string msg{"Bad Map Generator! FPS: "
                + std::to_string(frames * 1000.0 / (current_time -
                frame_check_time))
                + " | Runtime: " + std::to_string(current_time / 1000)
                + "s"};
            SDL_SetWindowTitle(window, msg.c_str());
            frame_check_time = current_time;
            frames = 0;
        }
        /** End of time stuff **/


        handle_input();


        if (reload) {
            //Generate a new map
            reload = false;
            clear(renderer);
            SDL_RenderPresent(renderer);
            fill(map, map_width, map_height);

            clear(renderer);
            render(map, map_width, map_height, renderer);
            SDL_RenderPresent(renderer);
            frames++;
        }
        else if (greyscale_reload) {
            greyscale_reload = false;
            clear(renderer);
            SDL_RenderPresent(renderer);
            fill_greyscale(map, map_width, map_height);

            clear(renderer);
            render(map, map_width, map_height, renderer);
            SDL_RenderPresent(renderer);
        }
        else if (load_map) {
            load_map = false;
            clear(renderer);
            SDL_RenderPresent(renderer);
            fill_with_map(map, map_width, map_height);

            clear(renderer);
            render(map, map_width, map_height, renderer);
            SDL_RenderPresent(renderer);
        }
        else {
            SDL_Delay(50);
        }
        frames++;
    }
    return 0;
}

void fill(Pixel* map, int width, int height)
{
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            SDL_Rect r{j * PIXEL_LENGTH, i * PIXEL_LENGTH, PIXEL_LENGTH,
                PIXEL_LENGTH};
            map[i*width + j] = Pixel{r, 0, get_num(), get_num(), get_num()};
        }
    }
}

void fill_greyscale(Pixel* map, int width, int height)
{
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            SDL_Rect r{j * PIXEL_LENGTH, i * PIXEL_LENGTH, PIXEL_LENGTH,
                PIXEL_LENGTH};
            Uint8 c = get_num();
            map[i*width + j] = Pixel{r, 0, c, c, c};
        }
    }
}

void render(const Pixel* map, int width, int height, SDL_Renderer* rend)
{
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            const Pixel* p = &map[i*width + j];
            if (SDL_SetRenderDrawColor(rend, p->r, p->g, p->b, SDL_ALPHA_OPAQUE)
                < 0 ) {
                    LOG("Failed to set draw colour!");
                    return;
                }

            if (SDL_RenderFillRect(rend, &p->rect) < 0) {
                LOG("Failed to render!");
                return;
            }
        }
    }
}

Uint8 get_num()
{
    return dist(random_num);
}

void handle_input()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
                break;
            }
            else if (e.key.keysym.sym == SDLK_r) {
                reload = true;
            }
            else if (e.key.keysym.sym == SDLK_g) {
                greyscale_reload = true;
            }
            else if (e.key.keysym.sym == SDLK_m) {
                load_map = true;
            }
        }
        else if (e.type == SDL_QUIT) {
            running = false;
            break;
        }
    }

}

void clear(SDL_Renderer* r)
{
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
}

void noisify(Pixel* map, int width, int height)
{

}

void fill_with_map(Pixel* map, int width, int height)
{
    std::uniform_int_distribution<int> standard_dist(0, 262144);

    /** Lay the seeds of land **/
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            if (standard_dist(random_num) < 4) {
                map[i*width + j].ID = 1;
                map[i*width + j].r = 0;
                map[i*width + j].g = 200;
                map[i*width + j].b = 0;
            }
            else {
                map[i*width + j].ID = 0;
                map[i*width + j].r = 0;
                map[i*width + j].g = 0;
                map[i*width + j].b = 200;
            }
        }
    }

    /** Make the land bigger **/
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            if (map[i*width + j])
        }
    }
}
