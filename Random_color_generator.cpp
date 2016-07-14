#include "Random_color_generator.h"

Random_color_generator::Random_color_generator()
{
    this = Random_color_generator(
        std::chrono::system_clock::now().time_since_epoch().count());
}

Random_color_generator::Random_color_generator(unsigned s)
    :seed{s}
{
    random_number = std::mt199937{seed};
}

Uint8 Random_color_generator::get_color()
{
    return dist(random_number);
}
