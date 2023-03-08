#include "User.hpp"

User::User(void): _name("Random"), _nickName("Random"), _hostName("Random"), _fullName("Random"),
					_password(""), _fd(-1), _mode(""), _isAuth(false),
					_address(sockaddr_in()), _addressSize(0)
{
	_address.sin_family = AF_INET;
}

void	User::setName(std::string const &name)
{
	_name = name;
}

void	User::setNickName(std::string const &nickName)
{
	_nickName = nickName;
}

void	User::setHostName(std::string const &hostName)
{
	_hostName = hostName;
}

void	User::setFullName(std::string const &fullName)
{
	_fullName = fullName;
}

void	User::setPassword(std::string const &password)
{
	_password = password;
}

void	User::setFd(int const &fd)
{
	_fd = fd;
}

std::string const &User::getName(void) const
{
	return (_name);
}

std::string const &User::getNickName(void) const
{
	return (_nickName);
}

std::string const &User::getHostName(void) const
{
	return (_hostName);
}

std::string const &User::getFullName(void) const
{
	return (_fullName);
}

std::string const &User::getPassword(void) const
{
	return (_password);
}

int		const &User::getFd(void) const
{
	return (_fd);
}

struct sockaddr *User::getAddress(void)
{
	// return (reinterpret_cast<struct sockaddr *>(&_address));
	return ((struct sockaddr *)(&_address));
}

socklen_t		*User::getAddressSize(void)
{
	// return (reinterpret_cast<socklen_t *>(&_addressSize));
	return ((socklen_t *)(&_addressSize));
}
