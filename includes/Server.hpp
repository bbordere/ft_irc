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

struct nickCompByNick
{
	User const u1;
	nickCompByNick(std::string const &nick): u1(User(nick)) {}
	bool operator()(User const &user)
	{
		return (user.getNickName() == u1.getNickName());
	}
};

struct nickCompByPtr
{
	User const *u1;
	nickCompByPtr(User const *user): u1(user) {}
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

	public:

		typedef std::map<std::string, Channel> map_chan_t;
		typedef std::vector<std::string> vec_str_t;
		typedef std::vector<User> vec_usr_t;

	private:
		typedef void* (*ptrFonction)(std::vector<std::string>);
		std::vector<struct pollfd>	_pollingList;

		int	_fd;
		struct sockaddr_in	_address;

		std::vector<User> _users;
		std::map<std::string, ptrFonction> _serverCmd;

		map_chan_t	_channels;

		std::set<User, userComp>		_bannedUsers;
		bool		_isOn;
		std::string	const _password;

		void	__handleConnection(void);
		void	__handlePackets(void);
		void	__authUser(User &user);
		void	__sendWelcomeMsg(User const &user);
		std::map<std::string, ptrFonction>	__initCmd();
		std::vector<std::string>			__parseCmd(std::string str);
		bool	__checkMsgLen(vec_str_t const &msg, std::size_t const expected, User const &user) const;

		void	__joinChannel(vec_str_t const &msg, User const &user);
		void	__disconnectUser(User const &user, std::size_t const &i);
		void	__leaveChannel(vec_str_t const &name, User const &user);

		void	__privMsg(vec_str_t const &msg, User const &user);
		void	__userPrivMsg(vec_str_t const &msg, User const &user);

		void	__updateChannels(void);
		void	__changeChanMode(vec_str_t const &msg, User &user);

		void	__nickCMD(vec_str_t const &msg, User &user) const;
		bool	__checkNickName(std::string const &nick) const;

		bool	__isChanExist(std::string const &name) const;

		void	__userCMD(vec_str_t const &msg, User &user) const;
		void	__passCMD(vec_str_t const &msg, User &user) const;

		void	__kickCMD(vec_str_t const &msg, User const &user);

		void	__leaveAllChan(User const &user);
		void	__quitCMD(vec_str_t const &msg, User &user);

		void	__joinExistingChan(vec_str_t const &name, User const &user);

		void	__inviteCMD(vec_str_t const &msg, User const &user);
		void	__inviteExistingChan(std::string const &chanName, std::string const &target, User const &user);
		
		void	__printDebug(void) const;

		void	__sendPong(vec_str_t const &msg, User const &user) const;

		void	__checkAuthClients(void);

		std::string	__cleanMsg(std::string msg) const;
		std::string __getChanUsersList(Channel const &chan) const;

		void	__usrModeHandling(vec_str_t const &msg, User &user);

		void	__noticeCMD(vec_str_t const &msg, User const &user) const;

		vec_str_t __parseCmd2(std::string str) const;

		static bool	__isMultiArg(std::string const &str);
		static bool	__isChanRelated(std::string const &str);

		void	__dccParsing(vec_str_t const &msg, User const &user);
		
		void	__topicCMD(vec_str_t const &msg, User const &user);

		void	__motdCMD(vec_str_t const &msg, User const &user);

		map_chan_t::iterator __searchChannel(std::string const &name, User const &user);

	public:

		static std::string getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &reason);
		static std::string getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &arg2, std::string const &reason);
		static bool	userCompFct(std::string const &nick1, std::string const &nick2);
		static std::string const formatMsg(std::string const &msg, User const &sender);
		Server(uint16_t const &port, std::string const &passwd);

		std::vector<User>::const_iterator getUserByNick(std::string const &nick) const;

		~Server(void);
		void	run(void);
};

#endif
