#include "Server.hpp"


ServerFailureException::ServerFailureException(char const *msg): _msg(msg) {}

const char *ServerFailureException::what() const throw()
{
	return (_msg);
}


Server::Server(uint16_t const &port, std::string const &passwd): _password(passwd)
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw (ServerFailureException("Socket creation failed"));
	int temp = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &temp, sizeof(temp)) < 0)
		throw (ServerFailureException("Socket creation failed"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw (ServerFailureException("Socket creation failed"));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(port);
    if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
		throw (ServerFailureException("Bind socket to port failed"));
	if (listen(_fd, MAX_CLIENT) < 0)
		throw (ServerFailureException("Start listening failed"));
	_pollingList.push_back(pollfd());
	_pollingList[0].fd = _fd;
	_pollingList[0].events = POLLIN;
	_isOn = true;
}

Server::~Server(void)
{
	close(_fd);
}

void	Server::__sendWelcomeMsg(User const &user)
{
	std::string msg = ":localhost 001 ";
	msg.append(user.getNickName());
	msg.append(" Welcome to OurNetwork, ");
	msg.append(user.getNickName());
	msg.append("!");
	msg.append(user.getName());
	msg.append("@");
	msg.append(user.getHostName());
	msg.append("\r\n");
	std::cout << msg << '\n';
	send(user.getFd(), msg.c_str(), msg.length(), 0);

	// std::string content = ":localhost 001 bastien Welcome\r\n";
}

void	Server::__authUser(User const &user)
{
	if (user.getPassword() != _password)
	{
		std::string content = numberToString(464) + "\r\n";
		std::cout << "WRONG PASSWORD FOR " << user.getFd() << '\n';
		send(user.getFd(), content.c_str(), content.length(), 0);
		return;
	}
	__sendWelcomeMsg(user);
	_users.push_back(user);
}

void	Server::__handleConnection(void)
{
	User newUser;
	newUser.setFd(accept(_fd, newUser.getAddress(), newUser.getAddressSize()));
	if (newUser.getFd() == -1)
	{
		std::cerr << "Failed to accept incoming connection.\n";
		return;
	}
	if (fcntl(newUser.getFd(), F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Failed to accept incoming connection.\n";
		close(newUser.getFd());
		return;
	}
	char hostnameBuff[NI_MAXHOST];
	if (getnameinfo(newUser.getAddress(), *newUser.getAddressSize(), hostnameBuff,
		NI_MAXHOST, NULL, 0, NI_NUMERICSERV) < 0)
	{
		std::cerr << "Failed to get info from incoming connection.\n";
		close(newUser.getFd());
		return;
	}
	newUser.setHostName(hostnameBuff);

	struct pollfd newPFd;
	newPFd.events = POLLIN;
	newPFd.fd = newUser.getFd();
	_pollingList.push_back(newPFd);

	LOG_CO(newUser.getHostName(), newUser.getFd());
	__authUser(newUser);
}

void	Server::__handlePackets()
{
	for (std::size_t i = 1; i < _pollingList.size(); ++i)
	{
		if (_pollingList[i].revents & POLLIN)
		{
			char buffer[1024] = {};
			ssize_t bytes = recv(_pollingList[i].fd, &buffer, sizeof(buffer), 0);
			if (bytes < 0)
				std::cerr << "recv() failed\n";
			else if (!bytes)
			{
				std::cout << "Client " << i << " disconected\n";
				close(_pollingList[i].fd);
				_pollingList.erase(_pollingList.begin() + i);
			}
			else
			{
				std::string msg(buffer);
				msg.erase(--msg.end());
				LOG_SEND(i, msg);
			}
		}
	}
}

void	Server::run(void)
{
	while (_isOn)
	{
		if (poll(&_pollingList[0], _pollingList.size(), -1) < 0)
			break; // ERROR HANDLING
		if (_pollingList[0].revents & POLLIN)
			__handleConnection();
		else
			__handlePackets();
	}
}