#ifndef USER_HPP
#define USER_HPP

#include "irc.hpp"
#include "utils.hpp"

class User
{
	private:
		std::string _name;
		std::string _nickName;
		std::string _hostName;
		std::string _fullName;
		std::string _password;
		int			_fd;
		// std::string	_buffer; // A voir
		uint8_t		_mode;
		bool		_isAuth;

		unsigned int		_id;
		// bool		_isConnected;
		struct sockaddr_in	_address;
		socklen_t			_addressSize;


	
	public:
		void	setName(std::string const &name);
		void	setNickName(std::string const &nickName);
		void	setHostName(std::string const &hostName);
		void	setFullName(std::string const &fullName);
		void	setPassword(std::string const &password);
		void	setFd(int const &fd);
		void	setId(unsigned int const &id);

		std::string const &getName(void) const;
		std::string const &getNickName(void) const;
		std::string const &getHostName(void) const;
		std::string const &getFullName(void) const;
		std::string const &getPassword(void) const;

		std::string const getAllInfos(void) const;

		int			const &getFd(void) const;

		unsigned int	const &getId(void) const;

		struct sockaddr *getAddress(void);
		socklen_t		*getAddressSize(void);

		void	updateMode(std::string const &str);
		void	sendMsg(std::string const &msg) const;

		// std::string	&getBuffer(void); // A voir

		User();
};

std::ostream &operator<<(std::ostream &stream, User const &user);

#endif