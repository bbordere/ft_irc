#include "irc.hpp"

std::vector<std::string> split(std::string str, std::string const &delimiter)
{
	std::vector<std::string> res;
	std::size_t pos;

	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		res.push_back(str.substr(0, pos));
		str.erase(0, pos + delimiter.length());
	}
	res.push_back(str);

	return (res);
}

uint64_t hash(std::string const &str)
{
	uint64_t hash = 5381;

	for (std::size_t i = 0; i < str.length(); ++i)
		hash = ((hash << 5) + hash) + static_cast<int>(str[i]);
	return (hash);
}

std::string vecToStr(std::vector<std::string> const &vec)
{
	std::string res;
	for (std::size_t i = 0; i < vec.size() - 1; ++i)
		res += vec[i] + " ";
	res += vec.back();
	return (res);
}