#ifndef SERVER_HPP
#define SERVER_HPP

#include "irc.hpp"
#include "User.hpp"
#include "Channel.hpp"
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

struct nickComp
{
	User const *u1;
	nickComp(User const *user): u1(user) {}
	bool operator()(User const &user)
	{
		if (&user == u1)
			return (false);
		return (user.getNickName() == u1->getNickName());
	}
};


struct userComp
{
	bool operator() (User const &u1, User const &u2)
	{
		return (u1.getId() < u2.getId());
	}
};

class Server
{
	private:
		typedef void* (*ptrFonction)(std::vector<std::string>);
		std::vector<struct pollfd>	_pollingList;

		int	_fd;
		struct sockaddr_in	_address;

		std::vector<User> _users;
		std::map<std::string, ptrFonction> _serverCmd;

		std::map<std::string, Channel>	_channels;

		std::set<User, userComp>		_bannedUsers;
		bool		_isOn;
		std::string	const _password;

		void	__handleConnection(void);
		void	__handlePackets(void);
		void	__authUser(User &user);
		void	__sendWelcomeMsg(User const &user);
		std::map<std::string, ptrFonction>	__initCmd();
		std::vector<std::string>			__parseCmd(std::string str);
		void	__joinChannel(User const &user, std::string const &msg);
		void	__disconnectUser(User const &user, std::size_t const &i);
		void	__leaveChannel(User const &user, std::string const &name);
		void	__privMsg(std::string const &msg, User const &user);
		void	__updateChannels(void);
		void	__changeMode(std::string const &msg, User const &user);

		void	__nickCMD(std::string const &msg, User &user) const;
		bool	__checkNickName(std::string const &nick) const;


		void	__userCMD(std::string const &msg, User &user) const;



		void	__printDebug(void) const;

		void	__sendPong(std::string const &msg, User const &user) const;

		void	__checkAuthClients(void);

		std::string	__cleanMsg(std::string msg) const;


	public:
		static bool	userCompFct(std::string const &nick1, std::string const &nick2);
		static std::string const formatMsg(std::string const &msg, User const &sender);
		Server(uint16_t const &port, std::string const &passwd);
		~Server(void);
		void	run(void);
};

#endif