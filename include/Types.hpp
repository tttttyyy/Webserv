#ifndef TYPES_HPP
#define TYPES_HPP
#include "Libs.hpp"

class Types
{
    public:
        static Types &GetInstance();
        static std::map<std::string, std::string> MimeTypes;
    private:
        Types( void );
        ~Types();
        static Types singleton_;
};

#endif
