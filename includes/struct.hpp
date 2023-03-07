/* temp file for structure design */

#include "irc.hpp"

class User
{
	private:
		std::string _name;
		std::string _nickName;
		std::string _hostName;
		std::string _fullName;
		int			_fd;
		std::string	_buffer; // A voir
		std::string	_mode;  // A voir pour changer avec du bit masking
	
	public:
		void	setName(std::string const &name);
		void	setNickName(std::string const &nickName);
		void	setHostName(std::string const &hostName);
		void	setFullName(std::string const &fullName);

		std::string const &getName(void) const;
		std::string const &getNickName(void) const;
		std::string const &getHostName(void) const;
		std::string const &getFullName(void) const;
		int			const &getFd(void) const;

		std::string	&getBuffer(void); // A voir

		User(int fd);
};

class Channel
{
	private:
		std::string	_name;
		std::string	_topic;
		std::string	_mode; // A voir pour changer avec du bit masking
		std::map<User *, std::string> _userModes;
	
	public:
		void	setName(std::string const &name);
		void	setTopic(std::string const &topic);
		void	setMode(std::string const &mode);

		std::string const &getName(void) const;
		std::string const &getTopic(void) const;
		std::string const &getMode(void) const;

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
		std::map<std::string, Channel>	_channels;

		std::set<User, userComp>		_bannedUsers;
		bool		_isOn;
		std::string	_password;
};
