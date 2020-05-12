#include "Types.hpp"

#include <string>



std::istream & operator>>(std::istream & iStream, VehicleType & vType)
{
    std::string value;
    iStream >> value;
    vType = VehicleType(std::stoi(value));

    return iStream;
}

std::ostream & operator<<(std::ostream & oStream, const VehicleType & vType)
{
    oStream << static_cast<int>(vType);

    return oStream;
}
