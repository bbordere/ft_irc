#include "Server.hpp"

ServerFailureException::ServerFailureException(char const *msg): _msg(msg) {}

const char *ServerFailureException::what() const throw()
{
	return (_msg);
}

Server::Server(uint16_t const &port, std::string const &passwd): _password(numberToString(hash(passwd)))
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
	__initCmd();
}

Server::~Server(void)
{
	close(_fd);
	for (std::size_t i = 0; i < _users.size(); ++i)
		close(_users[i].getFd());
}

void	Server::__handleConnection(void)
{
	struct sockaddr_in usrAddr;
	socklen_t		   sin_size = sizeof(usrAddr);

	int userFd = accept(_fd, (struct sockaddr *)(&usrAddr), &sin_size);

	if (userFd == -1)
	{
		std::cerr << "Failed to accept incoming connection.\n";
		return;
	}

	if (fcntl(userFd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Failed to accept incoming connection.\n";
		close(userFd);
		return;
	}

	char hostnameBuff[NI_MAXHOST];
	if (getnameinfo((struct sockaddr *)(&usrAddr), sin_size, hostnameBuff,
			NI_MAXHOST, NULL, 0, NI_NUMERICSERV) < 0)
	{
		std::cerr << "Failed to get info from incoming connection.\n";
		close(userFd);
		return;
	}

	User newUser(userFd, usrAddr);
	newUser.setHostName(std::string(hostnameBuff));
	struct pollfd newPFd;
	newPFd.events = POLLIN;
	newPFd.fd = newUser.getFd();
	_pollingList.push_back(newPFd);

	LOG_CO(newUser.getHostName(), newUser.getFd());
	if (_users.size() != 0)
		newUser.setId(_users.back().getId() + 1);
	else
		newUser.setId(1);
	_users.push_back(newUser);
}

void	Server::__disconnectUser(User const &user, std::size_t const &i)
{
	std::cout << "User " << user << " disconected\n";
	__leaveAllChan(user);
	close(_pollingList[i].fd);
	_users[i - 1].setFd(-1);
	_pollingList.erase(_pollingList.begin() + i);
	_users.erase(_users.begin() + (i - 1));
}

void	Server::__sendWelcomeMsg(User &user)
{
	std::string msg2 = "";
	msg2.append(std::string("00") + numberToString(RPL::WELCOME));
	msg2.append(" ");
	msg2.append(user.getNickName());
	msg2.append(" Welcome to OurNetwork, ");
	msg2.append(&user.getAllInfos()[1]);
	user.sendMsg(msg2);
	user.sendMsg(user.getAllInfos() + " NICK " + user.getNickName());
	__motdCMD(vec_str_t(), user);
}

void	Server::__checkClientsStatus(void)
{
	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].isReadyToAuth() && !_users[i].getAuthState() && _users[i].getPassword() == _password)
		{
			_users[i].setAuth(true);
			__sendWelcomeMsg(_users[i]);
		}
		else if (_users[i].getLeavingState())
		{
			std::cout << "User " << _users[i] << " disconected\n";
			__leaveAllChan(_users[i]);
			close(_pollingList[i + 1].fd);
			_users[i].setFd(-1);
			_pollingList.erase(_pollingList.begin() + 1 + i);
			_users.erase(_users.begin() + i);
		}
	}
}

Server::vec_str_t Server::__parseCmd2(std::string str) const
{
	vec_str_t res;
	vec_str_t cmds = split(str, "\r\n");

	for (std::size_t i = 0; i < cmds.size(); ++i)
	{
		cmds[i] = __cleanMsg(cmds[i]);
		std::size_t pos;
		while ((pos = cmds[i].find(" ")) != std::string::npos)
		{
			res.push_back(cmds[i].substr(0, pos));
			cmds[i].erase(0, pos + 1);
			if (res.back().find(":") != std::string::npos)
			{
				while ((pos = cmds[i].find(" ")) != std::string::npos)
				{
					res.back().append(" " + cmds[i].substr(0, pos));
					cmds[i].erase(0, pos + 1);
				}
				res.back().append(" " + cmds[i]);
				cmds[i].clear();
			}
		}
		if (!cmds[i].empty())
			res.push_back(cmds[i]);
	}
	return (res);
}

void	Server::__dccParsing(vec_str_t const &msg, User const &user)
{
	std::string toParse = msg.back();
	vec_str_t sp = split(&toParse[1], " ");
	std::vector<User>::const_iterator target = getUserByNick(msg[1]);
	if (target == _users.end())
	{
		std::cout << "Not found " << msg[1] << '\n';
		return;
	}
	(*target).sendMsg(user.getAllInfos() + " PRIVMSG " + (*target).getNickName() + " " + msg[2] + "\r\n");
}

bool	Server::__containsCMD(vec_str_t &msg) const
{
	std::string const cmdHandled[] = {"CAP", "PASS", "USER", "NICK", "JOIN", "PART", "PING", "PRIVMSG", 
									"NOTICE", "MODE", "TOPIC", "MOTD", "INVITE", "KICK", "QUIT",
									"KILL", "DIE"};
	for (std::size_t i = 1; i < msg.size(); ++i)
	{
		std::string const *cmd = std::find(cmdHandled, cmdHandled + 15, msg[i]);
		if (cmd != cmdHandled + 15)
		{
			vec_str_t::iterator cmdPos = std::find(++msg.begin(), msg.end(), *cmd);
			msg.erase(msg.begin(), cmdPos);
			return (true);
		}
	}
	msg.clear();
	return (false);
}

void	Server::__caseHandling(vec_str_t &msg) const
{
	std::transform(msg[0].begin(), msg[0].end(), msg[0].begin(), ::toupper);
	if (msg.size() >= 2 && msg[1].find("#") != std::string::npos)
		std::transform(msg[1].begin(), msg[1].end(), msg[1].begin(), ::toupper);
}

void	Server::__handlePackets(void)
{
	for (std::size_t i = 1; i < _pollingList.size(); ++i)
	{
		std::vector<std::string> vecCmd;
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
				_users[i - 1].getBuffer() += buffer;
				if (_users[i - 1].getBuffer().find("\r\n") == std::string::npos)
					continue;
				if (_users[i - 1].checkMode(BIT(User::AWAY))
				                && _users[i - 1].getBuffer().find("PING") == std::string::npos 
				                && _users[i - 1].getBuffer().find("PONG") == std::string::npos
				                && _users[i - 1].getBuffer().find("AWAY") == std::string::npos){
				        _users[i - 1].unsetMode(User::AWAY);
				        _users[i - 1].sendMsg(Server::getRPLString(RPL::RPL_UNAWAY, "", "", ""));
				}

				vecCmd = __parseCmd2(_users[i - 1].getBuffer());
				if (vecCmd.empty())
					continue;
				bool isCommand = true;
				while (isCommand)
				{
					__caseHandling(vecCmd);
					if (!__authExecHandling(vecCmd[0], _users[i - 1]))
						break;
					std::map<std::string, ptrFonction>::iterator it = _serverCmd.find(vecCmd[0]);
					if (it != _serverCmd.end())
						(this->*(it->second))(vecCmd, _users[i - 1]);
					isCommand = __containsCMD(vecCmd);
				}
				_users[i - 1].getBuffer().clear();
			}
		}
	}
	__checkClientsStatus();
	__updateChannels();
}

void	Server::__printDebug(void) const
{
	std::cout << "\033[2J" << std::flush;
	std::cout << "------CHANNEL------" << '\n';
	std::cout << _channels.size() << " ACTIVE: " << '\n';
	for (std::map<std::string, Channel>::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		std::cout << (*it).first << '\t';
		std::cout << (*it).second << '\n';
	}
	std::cout << "------USERS------" << '\n';
	std::cout << _users.size() << " CONNECTED: " << '\n';
	std::cout << _users << '\n';
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
	signal(SIGINT, sigHandler);
	while (_isOn)
	{
		if (poll(&_pollingList[0], _pollingList.size(), -1) < 0)
			throw (ServerFailureException("poll"));
		if (_pollingList[0].revents & POLLIN)
			__handleConnection();
		else
			__handlePackets();
		__printDebug();
	}
}

bool	Server::__authExecHandling(std::string const &cmd, User const &user) const
{
	if (__isAuthNeeded(cmd) && !user.getAuthState())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOTREGISTERED, user.getNickName(), ":You have not registered !"));
		return (false);
	}
	return (true);
}


bool	Server::__isAuthNeeded(std::string const &msg) const
{
	std::string authCmds[] = {"CAP", "PASS", "NICK", "USER"};
	return (std::find(authCmds, authCmds + 4, msg) == authCmds + 4);
}
