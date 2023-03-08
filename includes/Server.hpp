#ifndef SERVER_HPP
#define SERVER_HPP

#include "irc.hpp"
#include "User.hpp"
#include <exception>

#define MAX_CLIENT 10

#define LOG_CO(host, client) (std::cout << "New connection from " << host << " on socket " << client << '\n')
#define LOG_SEND(client, msg) (std::cout << "client N" << client << " sent: " << msg << '\n')

class ServerFailureException: public std::exception
{
	private:
		const char *_msg;
	
	public:
		ServerFailureException(char const *msg);
		virtual const char* what() const throw();
};

struct userComp
{
	bool operator() (User const &u1, User const &u2)
	{
		return (u1.getFullName() < u2.getFullName());
	}
};

class Server
{
	private:
		typedef void (Server::*fctPtr)(void); // typedef pointeur sur fonction membre

		std::vector<struct pollfd>	_pollingList;

		int	_fd;
		struct sockaddr_in _address;

		std::vector<User> _users;
		std::map<std::string, fctPtr>	_commands;

		// std::map<std::string, Channel>	_channels;

		std::set<User, userComp>		_bannedUsers;
		bool		_isOn;
		std::string	const _password;

		void	__handleConnection(void);
		void	__handlePackets(void);
		void	__authUser(User const &user);
		void	__sendWelcomeMsg(User const &user);

	public:
		Server(uint16_t const &port, std::string const &passwd);
		~Server(void);
		void	run(void);
};

#endif