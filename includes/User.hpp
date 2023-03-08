#ifndef USER_HPP
#define USER_HPP

#include "irc.hpp"

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
		std::string	_mode;  // A voir pour changer avec du bit masking
		bool		_isAuth;
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

		std::string const &getName(void) const;
		std::string const &getNickName(void) const;
		std::string const &getHostName(void) const;
		std::string const &getFullName(void) const;
		std::string const &getPassword(void) const;
		int			const &getFd(void) const;

		struct sockaddr *getAddress(void);
		socklen_t		*getAddressSize(void);

		// std::string	&getBuffer(void); // A voir

		User();
};

#endif