/*
COPYRIGHT (c) 2016 Callum Wilson

MIT License

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <SDL2/SDL.h>
#include <libnoise/module/perlin.h>

#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <random>

#include "Logger.h"
#include "EasyBMP.h"
#include "Pixel_map.h"

/** Screen Variables **/
constexpr bool fullscreen = false;
constexpr int screen_width = 512;
constexpr int screen_height = 512;
constexpr int map_width = 2000;
constexpr int map_height = 2000;

SDL_Rect screen_rect{0, 0, screen_width, screen_height};
Pixel_map* map;

/* States */
bool running = true;
bool reload = false;
bool greyscale_reload = false;
bool perlin = false;
bool perlin_color = false;
bool perlin_map = true;
bool screen_changed = true;

int prev_mouse_x;
int prev_mouse_y;

void zoom(int y)
{
    int new_width = map->source_location.w;
    int new_height = map->source_location.h;
    if (y < 0) {    //Scrolling down, zooming out
        map->increment_zoom(-0.05);
    }
    else if (y > 0) {   //Scrolling up, zooming in
        map->increment_zoom();
    }
}

void update_screen_location(int mouse_x, int mouse_y)
{
    int mouse_dx = prev_mouse_x - mouse_x;
    int mouse_dy = prev_mouse_y - mouse_y;

    int new_x = map->x() + mouse_dx;
    int new_y = map->y() + mouse_dy;

    /** Ensure screen_location is still in the map **/
    if (new_x < 0)
        new_x = 0;
    else if (new_x + map->source_location.w > map_width)
        new_x = map_width - map->source_location.w;

    if (new_y < 0)
        new_y = 0;
    else if (new_y + map->source_location.h > map_height)
        new_y = map_height - map->source_location.h;

    map->set_source_location(new_x, new_y);

    prev_mouse_x = mouse_x;
    prev_mouse_y = mouse_y;
}


/*
Clear the screen
*/
void clear(SDL_Renderer* r)
{
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderClear(r);
}

/*
Handle input from the user
*/
void handle_input()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_KEYDOWN) {
            screen_changed = true;
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
                perlin_map = true;
            }
			else if (e.key.keysym.sym == SDLK_n) {
				perlin = true;
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            if ((SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) ==
                SDL_BUTTON_LMASK) {
                prev_mouse_x = x;
                prev_mouse_y = y;
            }
        }
        else if (e.type == SDL_MOUSEMOTION) {
            int x, y;
            if ((SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) ==
                SDL_BUTTON_LMASK) {
                    update_screen_location(x, y);
            }
            else {
                prev_mouse_x = x;
                prev_mouse_y = y;
            }
        }
        else if (e.type == SDL_MOUSEWHEEL) {
            zoom(e.wheel.y);
        }
    }
    else if (e.type == SDL_QUIT) {
        running = false;
        break;
    }
}
/*
void write_to_file(Pixel* map, int width, int height)
{
    LOG("Writing file");
    BMP image, color_image;
    image.SetSize(width, height);
    color_image.SetSize(width*RESOLUTION_FACTOR/2, height*RESOLUTION_FACTOR/2);
    color_image.SetBitDepth(32);
    image.SetBitDepth(32);
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            image(x, y)->Red = map[y*width + x].height * 255;
            image(x, y)->Green = map[y*width + x].height * 255;
            image(x, y)->Blue = map[y*width + x].height * 255;
            image(x, y)->Alpha = 255;

            if (y<height-1 && x<width-1) {
                if (x == 0 && y == 0) {
                    color_image(x, y)->Red = map[y*width + x].r;
                    color_image(x+1, y)->Red = map[y*width + x].r;
                    color_image(x, y+1)->Red = map[y*width + x].r;
                    color_image(x+1, y+1)->Red = map[y*width + x].r;

                    color_image(x, y)->Green = map[y*width + x].g;
                    color_image(x+1, y)->Green = map[y*width + x].g;
                    color_image(x, y+1)->Green = map[y*width + x].g;
                    color_image(x+1, y+1)->Green = map[y*width + x].g;

                    color_image(x, y)->Blue = map[y*width + x].b;
                    color_image(x+1, y)->Blue = map[y*width + x].b;
                    color_image(x, y+1)->Blue = map[y*width + x].b;
                    color_image(x+1, y+1)->Blue = map[y*width + x].b;

                    color_image(x, y)->Alpha = 255;
                    color_image(x+1, y)->Alpha = 255;
                    color_image(x, y+1)->Alpha = 255;
                    color_image(x+1, y+1)->Alpha = 255;
                }
                else {
                    int off_x = x + (RESOLUTION_FACTOR/4);
                    int off_y = y + (RESOLUTION_FACTOR/4);

                    color_image(off_x, off_y)->Red = map[y*width + x].r;
                    color_image(off_x+1, off_y)->Red = map[y*width + x].r;
                    color_image(off_x, off_y+1)->Red = map[y*width + x].r;
                    color_image(off_x+1, off_y+1)->Red = map[y*width + x].r;

                    color_image(off_x, off_y)->Green = map[y*width + x].g;
                    color_image(off_x+1, off_y)->Green = map[y*width + x].g;
                    color_image(off_x, off_y+1)->Green = map[y*width + x].g;
                    color_image(off_x+1, off_y+1)->Green = map[y*width + x].g;

                    color_image(off_x, off_y)->Blue = map[y*width + x].b;
                    color_image(off_x+1, off_y)->Blue = map[y*width + x].b;
                    color_image(off_x, off_y+1)->Blue = map[y*width + x].b;
                    color_image(off_x+1, off_y+1)->Blue = map[y*width + x].b;

                    color_image(off_x, off_y)->Alpha = 255;
                    color_image(off_x+1, off_y)->Alpha = 255;
                    color_image(x, off_y+1)->Alpha = 255;
                    color_image(off_x+1, off_y+1)->Alpha = 255;

                }

            }
        }
    }

    image.WriteToFile("Map.bmp");
    color_image.WriteToFile("Color_Map.bmp");
    LOG("Finished writing to file");*/
}

int main(int argc, char* argv[])
{
    /* Initliase SDL subsystems */
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
        LOG("Could not initialise SDL");
        LOG("Error: ");
        LOG(std::string{SDL_GetError()});
        return 1;
    }
    SDL_Window* window;
    if (fullscreen) {
        window = SDL_CreateWindow("Bad Map Generator!", 40, 40,
            screen_width, screen_height,
            SDL_WINDOW_SHOWN|SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        window = SDL_CreateWindow("Bad Map Generator!", 40, 40,
            screen_width, screen_height,SDL_WINDOW_SHOWN);
    }

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

    map = new Pixel_map(renderer, map_width, map_height);

    Uint32 current_time = 0;
    Uint32 frame_check_time = 0;
    long frames = 0;

    frame_check_time = SDL_GetTicks();
    LOG("Entering main loop");
    while (running) {
        /** Handle Time stuff **/
        current_time = SDL_GetTicks();

        //Update FPS counter every second
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
            map->fill_color_static();
            map->render();
        }
        else if (greyscale_reload) {
            greyscale_reload = false;
            map->fill_static();
            map->render();
        }
		else if (perlin) {
			perlin = false;
            map->fill_perlin_noise();
            map->render();
		}
        //else if () {}
        else {
            SDL_Delay(50);
        }

        if (screen_changed) {
            screen_changed = false;
            if (!map->show(&screen_rect)) {
                LOG("Failed to render!");
                LOG(SDL_GetError());
            }
            else {
                SDL_RenderPresent(renderer);
            }
        }

        frames++;
    }
    LOG("Program exiting successfully");
    return 0;
}
