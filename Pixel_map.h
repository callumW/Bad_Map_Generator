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

    File: Pixel_map.h
    Callum Wilson
    2016-7-13
    Defines a pixel map which can be drawn to the screen. Allows access
    to inidividual pixels, and allows for scaling.
*/
#ifndef PIXEL_MAP_H
#define PIXEL_MAP_H

#include <chrono>
#include <random>
#include <cstdint>
#include <stdexcept>

#include <SDL2/SDL.h>
#include "Biome.h"
#include "Random_color_generator.h"

/**
 * Defines a pixel
 */
struct Pixel {
    SDL_Rect rect;  /**< Location of this pixel */
    BIOME ID;       /**< ID of the current BIOME */
    Uint8 r;        /**< The red value in [0, 255] */
    Uint8 g;        /**< The green value in [0, 255] */
    Uint8 b;        /**< The blue value in [0, 255] */
	double height;  /**< The height of the pixel */
};

class Pixel_map {
public:
    /**
     * Constructor
     * \param r The SDL renderer to use when rendering
     * \param w The desired width of the pixel map (in pixels) > 0.
     * \param h The desired width of the pixel map (in pixels) > 0.
     * \param pl The desired size of each pixel 1 for 1:1 pixel ratio > 0.
     * \param z The zoom factor of the pixel map. must be in [0, inf].
     */
    Pixel_map(SDL_Renderer* r, int w = 100, int h = 100, int pl = 1,
        double z = 1.0f);

    ~Pixel_map();

    /**
     * Fills the pixel map with random noise (look like tv static with color).
     */
    void fill_color_static();

    /**
     * Fills the pixel map with random noise but greyscale (looks like tv
     * static).
     */
    void fill_static();

    /**
     * Fills the pixel map with perlin greyscale perlin noise.
     * \param freq The frequency of perlin noise to use. must be >0.
     */
    void fill_perlin_noise(double freq = 1.0f);

    /**
     * Fills the pixel map with coloured perlin noise.
     * \param freq The frequency of perlin noise to use. Must be >0.
     */
    void fill_color_perlin_noise(double freq = 1.0f);

    /**
     * Fills the pixel map with a map created using perlin noise.
     */
    void fill_perlin_map(double freq = 1.0f);

    /**
     * Render the map to the screen.
     * \return true if successfully rendered to screen, false otherwise.
     */
    bool render();

    /**
     * set the zoom of the pixel map.
     * \param z The new zoom value. Must be >0, else zoom will be set to 1.0f
     */
    void zoom(double z);

    void increment_zoom(double inc=0.05f);

    int x() const {return source_location.x;}
    int y() const {return source_location.y;}

    void set_source_location(int x, int y);

    void show(SDL_Rect* destination);

    int width;    /**< Width of the pixel map */
    int height;   /**< Height of the pixel map */
    int pixel_length; /**< The length of the sides of each pixel */
    SDL_Rect source_location;   /**< The current portion of the pixel map to
                                    draw */
private:
    SDL_Texture* map_image; /**< texture which we draw the pixel map on */

    int zoom_factor;    /**< The zoom factor to draw the map at */
    Pixel* map; /**< Pointer to array of pixels that represents the map */
    SDL_Renderer* renderer;

    Random_color_generator generate_color; /**< Generator of numbers in
                                                [0, 255]*/

    /**
     * Initialise each pixel in the map
     */
    void zero_map_pixels();
};
#endif
