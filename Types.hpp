#pragma once

#include <iostream>

enum class VehicleType
{
    undefined = -1,
    CAR = 0,
    BUS = 1,
    TRUCK = 2,
    MOTORCYCLE = 3,
    BICYCLE = 4
};

std::istream & operator>>(std::istream & iStream, VehicleType & vType);
std::ostream & operator<<(std::ostream & oStream, const VehicleType & vType);
