#include "Random_color_generator.h"

Random_color_generator::Random_color_generator()
{
    Random_color_generator(
        std::chrono::system_clock::now().time_since_epoch().count());
}

Random_color_generator::Random_color_generator(unsigned s)
    :seed{s}
{
    dist = std::uniform_int_distribution<Uint8>(0, 255);
    random_number = std::mt19937{seed};
}

Uint8 Random_color_generator::get_color()
{
    return dist(random_number);
}
