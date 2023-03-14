#include "irc.hpp"
#include "Server.hpp"

#include <bitset>

int main(void)
{
	try
	{
		Server server(6667, "1234");
		server.run();
	}
	catch(const ServerFailureException& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}
