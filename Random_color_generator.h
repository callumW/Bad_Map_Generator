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

    File: Random_color_generator.h
    Author: Callum Wilson, callum.w@outlook.com
    Date: 2016-7-16
    Description: 
*/
#ifndef RANDOM_COLOR_GENERATOR_H
#define RANDOM_COLOR_GENERATOR_H
#include <cstdint>
#include <random>
#include <chrono>
#include <SDL2/SDL.h>
class Random_color_generator {
public:
    /**
     * Initialise a Random_color_generator using the current time as the seed
     */
    Random_color_generator();

    /**
     * Initiliase a Random_color_generator using the specified seed.
     * \param s The seed to use.
     */
    Random_color_generator(unsigned s);

    /**
     * \return A random color in [0, 255]
     */
    Uint8 get_color();

    Uint8 operator()() {return get_color();}

private:
    unsigned int seed;  /**< The seed for this generator */
    std::mt19937 random_number{};    /**< The generator for this generator */
    std::uniform_int_distribution<Uint8> dist;
};
#endif
