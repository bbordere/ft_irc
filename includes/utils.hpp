#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

template <typename T>
std::string numberToString(T n)
{
	std::ostringstream oOStrStream;
	oOStrStream << n;
	return oOStrStream.str();
}

#endif