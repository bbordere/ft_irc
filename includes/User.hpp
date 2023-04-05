#ifndef USER_HPP
#define USER_HPP

#include "irc.hpp"
#include "utils.hpp"

class User
{
	public:
		std::string _name;
		std::string _nickName;
		std::string _hostName;
		std::string _fullName;
		std::string _password;
		int			_fd;
		// std::string	_buffer; // A voir
		uint8_t		_mode;
		bool		_isAuth;

		uint32_t		_id;
		// bool		_isConnected;
		struct sockaddr_in	_address;
		socklen_t			_addressSize;


	
	public:

		enum MODES
		{
			AWAY = 1,
			INVISIBLE,
			WALLOPS,
			RESTRICT,
			OPERATOR,
		};

		void	setName(std::string const &name);
		void	setNickName(std::string const &nickName);
		void	setHostName(std::string const &hostName);
		void	setFullName(std::string const &fullName);
		void	setPassword(std::string const &password);
		void	setFd(int const &fd);
		void	setId(uint32_t const &id);

		void	setAuth(bool const state);

		void	setMode(User::MODES const mode);
		void	unsetMode(User::MODES const mode);

		std::string const &getName(void) const;
		std::string const &getNickName(void) const;
		std::string const &getHostName(void) const;
		std::string const &getFullName(void) const;
		std::string const &getPassword(void) const;

		bool	getAuthState(void) const;

		std::string const getAllInfos(void) const;

		int			const &getFd(void) const;

		uint32_t	const &getId(void) const;

		struct sockaddr *getAddress(void);
		socklen_t		*getAddressSize(void);

		void	updateMode(std::string const &str);
		void	sendMsg(std::string const &msg) const;

		bool	isReadyToAuth(void) const;

		bool	checkMode(uint8_t const &mode) const;

		uint8_t getMode(void) const;


		// std::string	&getBuffer(void); // A voir

		// User();
		User(int const fd, struct sockaddr_in);
		User(std::string const &nick);
};

std::ostream &operator<<(std::ostream &stream, User const &user);

#endif