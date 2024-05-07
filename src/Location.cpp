#include "Location.hpp"

Location::Location(std::string const &location) : _location(location) {}

Location &Location::operator=(Location const &rhs) {
	if (this != &rhs) {
		ServerCore::operator=(rhs);
		_location = rhs._location;
		_nestedLocations = rhs._nestedLocations;
	}
	return (*this);
}

// Purpuse: inheritence from server block
Location &Location::operator=(ServerCore const &rhs)
{
	if (this != &rhs)
	{
		ServerCore::operator=(rhs);
		error_page.clear();
		_cgis.clear();
		_redirections.clear();
	}
	return (*this);
}

Location::~Location() {}

std::string const &Location::getLocation( void ) const {return (this->_location);}
