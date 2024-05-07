#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

std::string fileToString(std::string const &fileName);
bool readFromFd(int fd, std::string &str);
int writeInFd(int fd, std::string &str);
long double	get_current_time(void);
std::string my_to_string(size_t num);