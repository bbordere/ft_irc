#include "User.hpp"

User::User(int const fd, struct sockaddr_in addr): _name("bastien"), _nickName("bastien"), _hostName("localhost"), _fullName("bastien"),
					_password(""), _fd(fd), _mode(0), _isAuth(false),
					_address(addr), _addressSize(sizeof(addr))
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

void	User::setId(uint32_t const &id)
{
	_id = id;
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

uint32_t const &User::getId(void) const
{
	return (_id);
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

void	User::sendMsg(std::string const &msg) const
{
	// std::string msg3 = ":bastien!bastien@localhost PRIVMSG #test :sq\r\n";
						//    :bastien!bastien@localhost :sqr
	// std::string to_send = ":bastien!bastien@localhost PRIVMSG #test " + msg;
	// std::string to_send = ":" + _name + "!" + _nickName + "@" + _hostName;
	// std::cout << "SEND = " << to_send << '\n';
	// to_send.append("\r\n"); 
	std::string to_send = msg;
	send(_fd, to_send.c_str(), to_send.length(), 0);
}

void	User::updateMode(std::string const &str)
{
	std::vector<std::string> splited = split(str, " ");
	if (splited[1].size() != 2)
		sendMsg(numberToString(501) + " :Unknown MODE flag\r\n");
}

std::string const User::getAllInfos(void) const
{
	std::string res = ":" + _nickName + "!" + _name + "@" + _hostName + " ";
	return (res);
}

std::ostream &operator<<(std::ostream &stream, User const &user)
{
	stream << '{';
	// stream << "name: " << user.getName() << ", ";
	// stream << "nickName: " << user.getNickName() << ", ";
	// stream << "hostName: " << user.getHostName() << ", ";
	// stream << "fullname: " << user.getFullName() << ", ";
	// stream << "password: " << user.getPassword() << ", ";
	stream << "fd: " << user.getFd() << ", ";
	stream << "id: " << user.getId() << ", ";
	stream << "ptr: " << &user << '}';
	return (stream);
}
