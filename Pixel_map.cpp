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

    File: Pixel_map.cpp
    Callum Wilson
    2016-7-13
    Defines a pixel map, see Pixel_map.h
*/
#include "Pixel_map.h"
#include "Perlin_noise_generator.h"
#include <libnoise/module/perlin.h>
#include <climits>

Pixel_map::Pixel_map(SDL_Renderer* r, int w, int h, int pl, double z)
    :renderer{r}
{
    width = (w<0 ? 100 : w);
    height = (h<0 ? 100 : h);
    pixel_length = (pl<0 ? 1 : pl);
    zoom_factor = (z<0 ? 1.0f : z);

    map = new Pixel[width * height];
    if (map == NULL) {
        throw std::runtime_error("Failed to allocate pixel map array");
    }

    map_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, width, height);

    if (map_image == NULL)
        throw std::runtime_error("Failed to create map_image: " +
            std::string{SDL_GetError()});

    zero_map_pixels();

    double new_width = width*zoom_factor;
    double new_height = height*zoom_factor;

    /* Check if values can fit in an int */
    if (new_width > INT_MAX)
        new_width = INT_MAX;
    if (new_height > INT_MAX)
        new_width = INT_MAX;

    source_location = SDL_Rect{0, 0, (int) new_width, (int) new_height};

    generate_color = Random_color_generator();
}

Pixel_map::~Pixel_map()
{
    delete[] map;
    SDL_DestroyTexture(map_image);
    renderer = NULL;    //Note: this class does not own the renderer. Is this
                        // still nedded? Likely not.
}

void Pixel_map::zero_map_pixels()
{
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            map[y*width + x] = Pixel{SDL_Rect{x, y, pixel_length, pixel_length},
                BIOME::empty, 0, 0, 0, 0.0f};
        }
    }
}

void Pixel_map::fill_color_static()
{
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            map[y*width + x].ID = BIOME::empty;
            map[y*width + x].r = generate_color();
            map[y*width + x].g = generate_color();
            map[y*width + x].b = generate_color();
            map[y*width + x].height = 0.0f;
        }
    }
}

void Pixel_map::fill_static()
{
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            Uint8 color = generate_color();
            map[y*width + x].ID = BIOME::empty;
            map[y*width + x].r = color;
            map[y*width + x].g = color;
            map[y*width + x].b = color;
            map[y*width + x].height = 0.0f;
        }
    }
}

void Pixel_map::fill_perlin_noise(double freq)
{
    Perlin_noise_generator generator{};
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            map[y*width + x].height = generator.get_num(freq*x, freq*y);

            Uint8 color = map[y*width + x].height * 255;
            map[y*width + x].r = color;
            map[y*width + x].g = color;
            map[y*width + x].b = color;
            map[y*width + x].ID = BIOME::empty;
        }
    }
}

void Pixel_map::fill_color_perlin_noise(double freq)
{
}

void Pixel_map::fill_perlin_map(double freq)
{
    Perlin_noise_generator generator{};
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            double temp_height = generator.get_num(freq*x, freq*y);

            if (temp_height < 0.55) {    //Deep sea
                map[y*width + x].ID = BIOME::deep_sea;
                map[y*width + x].r = 0;
    			map[y*width + x].g = 0;
    			map[y*width + x].b = 130;
            }
            else if (temp_height < 0.57) {     //If under water table
                map[y*width + x].ID = BIOME::shore;
                map[y*width + x].r = 0;
    			map[y*width + x].g = 0;
    			map[y*width + x].b = 227;
            }
            else if (temp_height < 0.59) {     //sand beaches
                map[y*width + x].ID = BIOME::beach;
                map[y*width + x].r = 240;
    			map[y*width + x].g = 255;
    			map[y*width + x].b = 140;
            }
            else if (temp_height < 0.8) { //grass
                map[y*width + x].ID = BIOME::grassland;
                map[y*width + x].r = 0;
    			map[y*width + x].g = 175;
    			map[y*width + x].b = 0;
            }
            else if (temp_height < 0.9) {    //darker grass
                map[y*width + x].ID = BIOME::woodland;
                map[y*width + x].r = 0;
    			map[y*width + x].g = 145;
    			map[y*width + x].b = 0;
            }
            else if (temp_height < 0.99999) {    //rock
                map[y*width + x].ID = BIOME::mountain;
                map[y*width + x].r = 140;
    			map[y*width + x].g = 140;
    			map[y*width + x].b = 140;
            }
            else {      //snow caps
                map[y*width + x].ID = BIOME::snow;
                map[y*width + x].r = 240;
    			map[y*width + x].g = 240;
    			map[y*width + x].b = 240;
            }
            map[y*width + x].height = temp_height;
        }
    }
}

bool Pixel_map::render()
{
    SDL_SetRenderTarget(renderer, map_image);

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            const Pixel* p = &map[y*width + x];
            SDL_SetRenderDrawColor(renderer, p->r, p->g, p->b,
                SDL_ALPHA_OPAQUE);

            SDL_RenderFillRect(renderer, &p->rect);
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
}

void Pixel_map::zoom(double z)
{
    if (z > 0) {
        zoom_factor = z;
        double new_width = width*zoom_factor;
        double new_height = height*zoom_factor;

        /* Check if values can fit in an int */
        if (new_width > INT_MAX)
            new_width = INT_MAX;
        if (new_height > INT_MAX)
            new_width = INT_MAX;

        source_location.w = (int) new_width;
        source_location.h = (int) new_height;
    }
}

void Pixel_map::set_source_location(int x, int y)
{
    if (x >= 0 && y >= 0) {
        source_location.x = x;
        source_location.y = y;
    }
}

void Pixel_map::increment_zoom(double inc)
{
    if (zoom_factor + inc < 0)
        return;
    else
        zoom(zoom_factor + inc);
}

void Pixel_map::show(SDL_Rect* destination)
{
    SDL_RenderCopy(renderer, map_image, &source_location, destination);
}
