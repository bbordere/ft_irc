#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

#define SET_N_BIT(x, n) (x |= 1 << n)
#define CLEAR_N_BIT(x, n) (x &= ~(1 << n))
#define TOG_N_BIT(x, n) (x ^= 1 << n)
#define GET_N_BIT(x, n) ((x >> n) & 1)

template <class T, class U>
std::ostream &operator<<(std::ostream &stream, std::pair<T, U> const &pair)
{
	stream << '(' << pair.first << ", " << pair.second << ')';
	return (stream);
}

template <class Key, class T, class Compare, class Allocator>
std::ostream &operator<<(std::ostream &stream, std::map<Key, T, Compare, Allocator> const &map)
{
	stream << '{';
	if (map.empty())
	{
		stream << '}';
		return (stream);
	}
	typename std::map<Key, T, Compare, Allocator>::const_iterator pair = map.begin();
	for (std::size_t i = 0; i < map.size() - 1; i++)
	{
		stream << '\'' << (*pair).first << "': " << (*pair).second << ", ";
		++pair;
	}
	stream << '\'' << (*pair).first << "': " << (*pair).second << '}';
	return (stream);
}

template <typename T, class Alloc>
std::ostream &operator<<(std::ostream &stream, std::vector<T, Alloc> const &vec)
{
	if (vec.empty())
	{
		stream << "[]";
		return (stream);
	}
	stream << '[';
	for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end() - 1; ++it)
		stream << *it << ", ";
	stream << *(vec.end() - 1) << ']';
	stream << ", size: " << vec.size() << ", capacity: " << vec.capacity();
	return (stream);	
}

namespace userMode
{
	enum
	{
		a,
		i,
		w,
		r,
		o,
		O,
		s
	};
}

template <typename T>
std::string numberToString(T n)
{
	std::ostringstream oOStrStream;
	oOStrStream << n;
	return oOStrStream.str();
}

std::vector<std::string> split(std::string str, std::string const &delimiter);

#endif