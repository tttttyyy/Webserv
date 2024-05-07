#ifndef CORE_HPP
#define CORE_HPP
#include "Libs.hpp"

struct IListener
{
	protected:
		virtual void setIp(std::string const &ipv) = 0;
		virtual void setPort(std::string const &port) = 0;
		virtual const char* getIp( void ) const = 0;
		virtual const char* getPort( void ) const = 0;
        // virtual uint32_t getNIp( void ) const = 0;
		// virtual uint16_t getNPort( void ) const = 0;
		virtual ~IListener() {};
	protected:
        std::string ip;
        std::string port;
    protected:
        uint16_t n_port;
        uint32_t n_ip;

};

#endif