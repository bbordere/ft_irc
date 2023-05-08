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
		typedef void (Server::*ptrFonction)(vec_str_t const &msg, User &user);
		std::vector<struct pollfd>	_pollingList;

		int	_fd;
		struct sockaddr_in	_address;

		std::vector<User> _users;
		std::map<std::string, ptrFonction> _serverCmd;

		map_chan_t	_channels;

		bool		_isOn;
		std::string	const _password;
		void	_initCmd(void);

		void	_handleConnection(void);
		void	_handlePackets(void);
		void	_authUser(User &user);
		void	_sendWelcomeMsg(User &user);
		std::vector<std::string>			_parseCmd(std::string str);
		bool	_checkMsgLen(vec_str_t const &msg, std::size_t const expected, User const &user) const;

		void	_joinChannel(vec_str_t const &msg, User &user);
		void	_disconnectUser(User const &user, std::size_t const &i);
		void	_leaveChannel(vec_str_t const &name, User &user);

		size_t _getUserIndex(std::string const &nick) const;

		void	_privMsg(vec_str_t const &msg, User &user);
		void	_userPrivMsg(vec_str_t const &msg, User &user);

		void	_updateChannels(void);
		void	_changeChanMode(vec_str_t const &msg, User &user);

		void	_nickCMD(vec_str_t const &msg, User &user);
		bool	_checkNickName(std::string const &nick) const;

		bool	_isChanExist(std::string const &name) const;

		void	_userCMD(vec_str_t const &msg, User &user);
		void	_passCMD(vec_str_t const &msg, User &user);

		void	_killCMD(vec_str_t const &msg, User &user);

		void	_kickCMD(vec_str_t const &msg, User &user);

		void	_leaveAllChan(User const &user);
		void	_quitCMD(vec_str_t const &msg, User &user);

		void	_operCMD(vec_str_t const &msg, User &user);
		void	_dieCMD(vec_str_t const &msg, User &user);

		void	_awayCMD(vec_str_t const &msg, User &user);

		void	_listCMD(vec_str_t const &msg, User &user);

		void	_joinExistingChan(vec_str_t const &msg, std::string const &name, User const &user);

		void	_inviteCMD(vec_str_t const &msg, User &user);
		void	_inviteExistingChan(std::string const &chanName, std::string const &target, User const &user);
		
		void	_printDebug(void) const;

		void	_sendPong(vec_str_t const &msg, User &user);

		void	_checkClientsStatus(void);

		std::string	_cleanMsg(std::string msg) const;
		std::string _getChanUsersList(Channel const &chan) const;

		void	_usrModeHandling(vec_str_t const &msg, User &user);

		void	_noticeCMD(vec_str_t const &msg, User &user);
		void	_modeCMD(vec_str_t const &msg, User &user);

		vec_str_t _parseCmd2(std::string str) const;

		static bool	_isMultiArg(std::string const &str);
		static bool	_isChanRelated(std::string const &str);

		bool	_containsCMD(vec_str_t &msg) const;

		void	_dccParsing(vec_str_t const &msg, User const &user);
		
		void	_topicCMD(vec_str_t const &msg, User &user);

		void	_motdCMD(vec_str_t const &msg, User &user);

		map_chan_t::iterator _searchChannel(std::string const &name, User const &user);

		void	_caseHandling(vec_str_t &msg) const;

		bool	_isAuthNeeded(std::string const &msg) const;
		bool	_authExecHandling(std::string const &cmd, User const &user) const;

	public:

		static std::string getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &reason);
		static std::string getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &arg2, std::string const &reason);
		static bool	userCompFct(std::string const &nick1, std::string const &nick2);
		static std::string const formatMsg(std::string const &msg, User const &sender);
		Server(uint16_t const &port, std::string const &passwd);

		std::vector<User>::const_iterator getUserByNick(std::string const &nick) const;
		std::vector<User>::iterator getUserByNick(std::string const &nick);

		~Server(void);
		void	run(void);
};

#endif
