#include "irc.hpp"
#include "Server.hpp"

#include <bitset>

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Usage: ./ircserv [port] [password]" << '\n';
		return (1);
	}
	try
	{
		Server server(std::atoi(av[1]), av[2]);
		std::cout << "Server running on port " << std::atoi(av[1]) << "!\n";
		server.run();
	}
	catch(const ServerFailureException& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
	catch(const std::exception& e){}
	return (0);
}
