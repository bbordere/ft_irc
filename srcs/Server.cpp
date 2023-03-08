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
	_serverCmd = __initCmd();
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

void	Server::__authUser(User &user)
{
	if (user.getPassword() != _password)
	{
		std::string content = numberToString(464) + "\r\n";
		std::cout << "WRONG PASSWORD FOR " << user.getFd() << '\n';
		send(user.getFd(), content.c_str(), content.length(), 0);
		return;
	}
	__sendWelcomeMsg(user);
	if (_users.size() != 0)
		user.setId(_users.back().getId() + 1);
	else
		user.setId(0);
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

void	Server::__joinChannel(User const &user, std::string const &msg)
{
	std::vector<std::string> splited = split(msg, " ");
	splited[1].erase(splited[1].end() - 1, splited[1].end());
	if (_channels.count(splited[1]))
		_channels.at(splited[1]).addUser(user);
	else
	{
		std::cout << "New Chan " << '\n';
		_channels.insert(std::make_pair(splited[1], Channel(splited[1])));
		_channels.at(splited[1]).addUser(user);
	}
	std::string joinMsg = ":" + user.getName() + "!" + user.getNickName() + "@localhost";

//:bastien!bastien@localhost JOIN #test
//:bastien!bastien@localhost JOIN #test

	
	joinMsg.append(" JOIN ");
	joinMsg.append(splited[1]);
	joinMsg.append("\r\n");
	// joinMsg = ":bastien!bastien@localhost JOIN #test\r\n";
	// _channels.at(splited[1]).broadcast(joinMsg);
	std::cout << joinMsg;
	// _channels.at(splited[1]).broadcast("localhost 353 Random = #test");
	// _channels.at(splited[1]).broadcast("localhost 366 Random #test");
}

void	Server::__disconnectUser(User const &user, std::size_t const &i)
{
	std::cout << "User " << user << " disconected\n";
	close(_pollingList[i].fd);
	_pollingList.erase(_pollingList.begin() + i);
	_users.erase(_users.begin() + (i - 1));
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		(*it).second.delUser(user);
}

void	Server::__leaveChannel(User const &user, std::string const &name)
{
	_channels.at(name).delUser(user);
	user.sendMsg(std::string("PART ").append(name));
	// user.sendMsg(":bastien!bastien@localhost PART #test :Part Message");
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
				__disconnectUser(_users[i - 1], i);
			else
			{
				std::vector<std::string> vecCmd = __parseCmd(buffer);
				std::map<std::string, ptrFonction>::iterator it = _serverCmd.find(vecCmd[1]);
				if (it != _serverCmd.end())
					(*(it->second))(vecCmd); //exec cmd
				std::string msg(buffer);
				msg.erase(--msg.end());

				if (msg.find("JOIN") != std::string::npos)
					__joinChannel(_users[i - 1], msg);
				if (msg.find("PART") != std::string::npos)
					__leaveChannel(_users[i - 1], split(msg, " ")[1]);
				if (msg.find("PRIVMSG") != std::string::npos)
				{
					std::size_t pos = msg.find(' ', msg.find(' ') + 1) + 1;
					std::size_t pos2 = msg.find(' ');
					std::vector<std::string> sp = split(msg, " ");
					_channels.at(msg.substr(0, pos2 + 1)).broadcast(msg.begin() + pos, msg.end(), &_users[i - 1]);
				}
				// LOG_SEND(i, msg);
			}
		}
	}
}

std::map<std::string, ptrFonction> Server::__initCmd() {
	std::map<std::string, ptrFonction> serverCmd;
	
	// serverCmd["CAP"] = &nickCmd;
	serverCmd["NICK"] = &nickCmd;
	serverCmd["USER"] = &userCmd;
	serverCmd["OPER"] = &operCmd;
	serverCmd["MODE"] = &modeCmd;
	serverCmd["QUIT"] = &quitCmd;
	serverCmd["SQUIT"] = &squitCmd;
	serverCmd["JOIN"] = &joinCmd;
	serverCmd["KILL"] = &killCmd;
	serverCmd["KICK"] = &kickCmd;
	serverCmd["ERROR"] = &errorCmd;
	serverCmd["PART"] = &partCmd;
	serverCmd["PRIVMSG"] = &privmsgCmd;
	serverCmd["NOTICE"] = &noticeCmd;
	serverCmd["AWAY"] = &awayCmd;
	serverCmd["DIE"] = &dieCmd;
	serverCmd["RESTART"] = &restartCmd;

	return (serverCmd);
}

std::vector<std::string> Server::__parseCmd(std::string str) {
    std::vector<std::string> cmd;

    if (str[0] != ':')
        cmd.push_back("");        
    int size = str.find(' '); 
    while (size != -1) {
        cmd.push_back(str.substr(0, size));
        str.erase(str.begin(), str.begin() + size + 1);
        size = str.find(' ');
    }
    str.erase(str.end() - 2, str.end());
    cmd.push_back(str.substr(0, size));

    return (cmd);
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

		for (std::map<std::string, Channel>::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			std::cout << "Chan: " << (*it).first << '\n';
			std::cout << "\t" << (*it).second._users << '\n';
		}
		std::cout << '\n';
	}
}