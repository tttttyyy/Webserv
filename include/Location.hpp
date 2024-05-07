#pragma once
#include "Libs.hpp"
#include "ServerCore.hpp"


class Location : public ServerCore
{
    public:
        Location(std::string const &location);
        Location &operator=(Location const &rhs);
        Location &operator=(ServerCore const &rhs);
        ~Location();
    public:
        std::string const &getLocation( void ) const;
    private:
        std::string _location;
        std::map<std::string, Location> _nestedLocations;
};
