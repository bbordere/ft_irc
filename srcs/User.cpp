#include "User.hpp"

User::User(int const fd, struct sockaddr_in addr): _name(""), _nickName(""), _hostName(""), _fullName(""),
					_password(""), _buffer(""), _fd(fd), _mode(0), _isAuth(false), _isLeaving(false),
					_address(addr), _addressSize(sizeof(addr))
{
	_address.sin_family = AF_INET;
	_buffer.reserve(1025);
}

User::User(std::string const &nick): _name(""), _nickName(nick), _hostName(""), _fullName(""),
					_password(""), _buffer(""), _fd(-1), _mode(0), _isAuth(false), _isLeaving(false),
					_address(), _addressSize()
{
	_address.sin_family = AF_INET;
	_buffer.reserve(1025);
}

bool	User::isReadyToAuth(void) const
{
	return (!_name.empty() && !_nickName.empty() && !_fullName.empty() && !_hostName.empty() && !_password.empty());
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

void	User::setUnawayMsg(std::string const &unawayMsg)
{
	_unawayMsg= unawayMsg;
}

void	User::setFd(int const &fd)
{
	_fd = fd;
}

void	User::setId(uint32_t const &id)
{
	_id = id;
}

void	User::setAuth(bool const state)
{
	_isAuth = state;
}

void	User::setLeaving(bool const state)
{
	_isLeaving = state;
}

void	User::setMode(User::MODES const mode)
{
	SET_N_BIT(_mode, mode);
}

void	User::unsetMode(User::MODES const mode)
{
	CLEAR_N_BIT(_mode, mode);
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

std::string const &User::getUnawayMsg(void) const
{
	return (_unawayMsg);
}

std::string  &User::getBuffer(void)
{
	return (_buffer);
}

int		const &User::getFd(void) const
{
	return (_fd);
}

uint32_t const &User::getId(void) const
{
	return (_id);
}

bool	User::getAuthState(void) const
{
	return (_isAuth);
}

bool	User::getLeavingState(void) const
{
	return (_isLeaving);
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
	if (_fd == -1)
		return;
	std::string to_send = msg + "\r\n";
	// std::cout << "Send to client N" <<  _id << ": " << to_send << '\n';
	send(_fd, to_send.c_str(), to_send.length(), MSG_NOSIGNAL);
}

void	User::updateMode(std::string const &str)
{
	std::vector<std::string> splited = split(str, " ");
	if (splited[1].size() != 2)
		sendMsg(numberToString(501) + " :Unknown MODE flag");
}

std::string const User::getAllInfos(void) const
{
	std::string res = ":" + _nickName + "!" + _name + "@" + _hostName;
	return (res);
}

uint8_t User::getMode(void) const
{
	return (_mode);
}

bool	User::checkMode(uint8_t const &mode) const
{
	return  ((_mode & mode) != 0);
}

std::ostream &operator<<(std::ostream &stream, User const &user)
{
	stream << '{';
	stream << "name: " << user.getName() << ", ";
	stream << "nickName: " << user.getNickName() << ", ";
	stream << "hostName: " << user.getHostName() << ", ";
	stream << "fullname: " << user.getFullName() << ", ";
	stream << "password: " << user.getPassword() << ", ";
	stream << "fd: " << user.getFd() << ", ";
	stream << "id: " << user.getId() << "}";
	return (stream);
}
