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

/*
Side length of each pixel (value of 1 means that there is a 1:1 mapping from
screen pixels to data pixels).
*/
#define PIXEL_LENGTH 1

enum class BIOME {
    empty, deep_sea, shore, beach, grassland, woodland, mountain, snow
};

/** Distributions for calculating pixel colors base on BIOME **/
std::uniform_int_distribution<Uint8> deep_sea_blue(100, 160);
std::uniform_int_distribution<Uint8> shore_blue(200, 255);
std::uniform_int_distribution<Uint8> grassland_green(150, 200);
std::uniform_int_distribution<Uint8> woodland_green(130, 150);
std::uniform_int_distribution<Uint8> mountain_all(85, 196);
std::uniform_int_distribution<Uint8> snow_all(240, 255);

/*
A single pixel on screen
*/
struct Pixel {
    SDL_Rect rect;
    BIOME ID;
    Uint8 r;
    Uint8 g;
    Uint8 b;
	double height;
};

/** Screen Variables **/
constexpr bool fullscreen = false;
constexpr int screen_width = 512;
constexpr int screen_height = 512;
constexpr int map_width = screen_width / PIXEL_LENGTH;
constexpr int map_height = screen_height / PIXEL_LENGTH;

/* States */
bool running = true;
bool reload = true;
bool greyscale_reload = false;
bool load_map = false;
bool noisify_map = false;
bool write_map = false;
/* Random stuff */
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 random_num{seed};  //generator

/** Noise Variables **/
noise::module::Perlin gen;	//noise generator
constexpr double freq_increment = 0.001;
double frequency = 0.004;


/*
A distribution allows us to specify a range for our random numbers. Here we use
[0, 255] so we can get a color value for red, green, or blue.
*/
std::uniform_int_distribution<Uint8> dist(0, 255);

/*
Get a random number from 0 to 255
*/
Uint8 get_num()
{
    return dist(random_num);
}

double noisify(double nx, double ny) {
  // Rescale from -1.0:+1.0 to 0.0:1.0
  return gen.GetValue(nx, ny, 0.5) / 2.0 + 0.5;
}
/*
Fill the map with random color values
*/
void fill(std::vector<Pixel>& map, int width, int height)
{
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            SDL_Rect r{j * PIXEL_LENGTH, i * PIXEL_LENGTH, PIXEL_LENGTH,
                PIXEL_LENGTH};
            map[i*width + j] = Pixel{r, BIOME::empty, get_num(), get_num(),
                get_num(), 0};
        }
    }
}

/*
Fill the map with greyscale color values
*/
void fill_greyscale(std::vector<Pixel>& map, int width, int height)
{
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            SDL_Rect r{j * PIXEL_LENGTH, i * PIXEL_LENGTH, PIXEL_LENGTH,
                PIXEL_LENGTH};
            Uint8 c = get_num();
            map[i*width + j] = Pixel{r, BIOME::empty, c, c, c};
        }
    }
}

/*
Fill the map with an actual map!
*/
void fill_with_map(std::vector<Pixel>& map, int width, int height)
{
    std::uniform_int_distribution<int> standard_dist(0, 262144);

    /** Lay the seeds of land **/
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            if (standard_dist(random_num) < 4) { //~50% chance
                map[i*width + j].ID = BIOME::grassland;
            }
            else {
                map[i*width + j].ID = BIOME::empty;
            }
        }
    }

    /** Make the land bigger **/
	for (int k=0; k<150; k++) {
	    for (int i=0; i<height; i++) {
	        for (int j=0; j<width; j++) {

	            if (map[i*width + j].ID == BIOME::grassland) {

					if (i > 0 && i < height-1) {
						if (j > 0 && j < width-1) {
							if (standard_dist(random_num) < 26214) {
								map[(i-1)*width + j].ID=BIOME::grassland;
								map[(i-1)*width + (j-1)].ID=BIOME::grassland;
								map[(i-1)*width + (j+1)].ID=BIOME::grassland;
								map[i*width + j].ID=BIOME::grassland;
								map[i*width + (j-1)].ID=BIOME::grassland;
								map[i*width + (j+1)].ID=BIOME::grassland;
								map[(i+1)*width + j].ID=BIOME::grassland;
								map[(i+1)*width + (j-1)].ID=BIOME::grassland;
								map[(i+1)*width + (j+1)].ID=BIOME::grassland;
							}
						}
					}
				}
	        }
	    }
	}

	/** Color the map */
	for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            switch (map[i*width + j].ID) {
			case BIOME::grassland:
				map[i*width + j].r = 0;
				map[i*width + j].g = 200;
				map[i*width + j].b = 0;
				break;
			case BIOME::empty:
				map[i*width + j].r = 0;
				map[i*width + j].g = 0;
				map[i*width + j].b = 200;
				break;
			default:
				map[i*width + j].r = 0;
				map[i*width + j].g = 0;
				map[i*width + j].b = 0;
			}
        }
    }

}

void fill_noise(std::vector<Pixel>& map, int width, int height)
{
    long start_time = SDL_GetTicks();
	for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            map[i*width + j].height = noisify(frequency*j, frequency*i);

            double height = map[i*width + j].height;
            if (height < 0.55) {    //Deep sea
                map[i*width + j].ID = BIOME::deep_sea;
                map[i*width + j].r = 0;
    			map[i*width + j].g = 0;
    			map[i*width + j].b = deep_sea_blue(random_num);
            }
            else if (height < 0.57) {     //If under water table
                map[i*width + j].ID = BIOME::shore;
                map[i*width + j].r = 0;
    			map[i*width + j].g = 0;
    			map[i*width + j].b = shore_blue(random_num);
            }
            else if (height < 0.59) {     //sand beaches
                map[i*width + j].ID = BIOME::beach;
                map[i*width + j].r = 240;
    			map[i*width + j].g = 255;
    			map[i*width + j].b = 140;
            }
            else if (height < 0.8) { //grass
                map[i*width + j].ID = BIOME::grassland;
                map[i*width + j].r = 0;
    			map[i*width + j].g = grassland_green(random_num);
    			map[i*width + j].b = 0;
            }
            else if (height < 0.9) {    //darker grass
                map[i*width + j].ID = BIOME::woodland;
                map[i*width + j].r = 0;
    			map[i*width + j].g = woodland_green(random_num);
    			map[i*width + j].b = 0;
            }
            else if (height < 0.99999) {    //rock
                map[i*width + j].ID = BIOME::mountain;
                Uint8 c_val = mountain_all(random_num);
                map[i*width + j].r = c_val;
    			map[i*width + j].g = c_val;
    			map[i*width + j].b = c_val;
            }
            else {      //snow caps
                map[i*width + j].ID = BIOME::snow;
                Uint8 c_val = snow_all(random_num);
                map[i*width + j].r = c_val;
    			map[i*width + j].g = c_val;
    			map[i*width + j].b = c_val;
            }
        }
    }
	/*
	for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {

        }
    }*/

    long end_time = SDL_GetTicks();
    LOG("Time taken to fill map with noise: " +
        std::to_string(end_time-start_time) + "ms");
}

/*
Render the map on screen_width
*/
void render(const std::vector<Pixel>& map, int width, int height,
    SDL_Renderer* rend)
{
    long start_time = SDL_GetTicks();
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
    long end_time = SDL_GetTicks();
    LOG("Rendering time: " + std::to_string(end_time-start_time) + "ms");
}


/*
Clear the screen
*/
void clear(SDL_Renderer* r)
{
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
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
			else if (e.key.keysym.sym == SDLK_n) {
                gen.SetSeed(random_num());
				noisify_map = true;
			}
            else if (e.key.keysym.sym == SDLK_UP) {
                noisify_map = true;
                frequency += freq_increment;
                LOG("Frequency: " + std::to_string(frequency));
            }
            else if (e.key.keysym.sym == SDLK_DOWN) {
                noisify_map = true;
                frequency -= freq_increment;
                LOG("Frequency: " + std::to_string(frequency));
            }
            else if (e.key.keysym.sym == SDLK_w) {
                write_map = true;
                LOG("Writing to file");
            }
        }
        else if (e.type == SDL_QUIT) {
            running = false;
            break;
        }
    }
}

void write_to_file(const std::vector<Pixel>& map)
{
    LOG("Writing file");
    BMP image, color_image;
    image.SetSize(screen_width, screen_height);
    color_image.SetSize(screen_width, screen_height);
    color_image.SetBitDepth(32);
    image.SetBitDepth(32);
    for (int y=0; y<screen_height; y++) {
        for (int x=0; x<screen_width; x++) {
            image(x, y)->Red = map[y*screen_width + x].height * 255;
            image(x, y)->Green = map[y*screen_width + x].height * 255;
            image(x, y)->Blue = map[y*screen_width + x].height * 255;
            image(x, y)->Alpha = 255;

            color_image(x, y)->Red = map[y*screen_width + x].r;
            color_image(x, y)->Green = map[y*screen_width + x].g;
            color_image(x, y)->Blue = map[y*screen_width + x].b;
            color_image(x, y)->Alpha = 255;
        }
    }
    image.WriteToFile("Map.bmp");
    color_image.WriteToFile("Color_Map.bmp");
    LOG("Finished writing to file");
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

    //Create an array of on screen pixels
    std::vector<Pixel> map{};
    try {
        map.reserve(map_width * map_height);
    }
    catch (const std::length_error& e) {
        LOG("Failed to reserve enough space in the map vector");
        LOG("Error: " + std::string{e.what()});
        return 1;
    }

    Uint32 current_time = 0;
    Uint32 frame_check_time = 0;
    long frames = 0;

    fill(map, map_width, map_height);   //fill the map with random values

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
                + "s" + " Frequency: " + std::to_string(frequency)};

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
		else if (noisify_map) {
			noisify_map = false;
            clear(renderer);
            SDL_RenderPresent(renderer);
            fill_noise(map, map_width, map_height);

            clear(renderer);
            render(map, map_width, map_height, renderer);
            SDL_RenderPresent(renderer);
		}
        else if (write_map) {
            SDL_SetWindowTitle(window, "Writing to file");
            write_map = false;
            write_to_file(map);
            SDL_SetWindowTitle(window, "Finished writing to file");
        }
        else {
            SDL_Delay(50);
        }

        frames++;
    }
    LOG("Program exiting successfully");
    return 0;
}
