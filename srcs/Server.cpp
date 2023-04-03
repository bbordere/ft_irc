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
	_serverCmd = __initCmd();
}

Server::~Server(void)
{
	close(_fd);
	for (std::size_t i = 0; i < _users.size(); ++i)
		close(_users[i].getFd());
}

bool	Server::userCompFct(std::string const &nick1, std::string const &nick2)
{
	return (nick1 == nick2);
}

std::string	Server::__cleanMsg(std::string msg) const
{
	std::size_t pos;
	while ((pos = msg.find("\r\n")) != std::string::npos)
	{
		msg.erase(msg.begin() + pos, msg.begin() + pos + 2);
		msg.insert(msg.begin() + pos, ' ');
	}
	return (msg);
}

bool	Server::__checkNickName(std::string const &nick) const
{
	std::string const specChar = "[]\\`_^{|}";
	if (nick.length() > 9)
		return (false);
	if (!std::isalpha(nick[0]) && specChar.find(nick[0]) == std::string::npos)
		return (false);
	for (std::size_t i = 1; i < nick.length(); ++i)
	{
		if (!std::isalnum(nick[i]) && (specChar.find(nick[i]) == std::string::npos) && nick[i] != '-')
			return (false);
	}
	return (true);
}

std::string Server::__getChanUsersList(Channel const &chan) const
{
	std::string res;

	for (std::size_t i = 0; i < _users.size() - 1; ++i)
	{
		if (chan.isInChan(_users[i]))
			res += _users[i].getNickName() + " ";
	}
	if (chan.isInChan(_users[_users.size() - 1]))
			res += _users[_users.size() - 1].getNickName();
	return (res);
}


void	Server::__nickCMD(vec_str_t const &msg, User &user) const
{
	std::string firstNickName = user.getNickName();

	vec_str_t::const_iterator nickIt = ++(std::find(msg.begin(), msg.end(), "NICK"));

	user.setNickName(*nickIt);
	bool found = std::find_if(_users.begin(), _users.end(), nickCompByPtr(&user)) != _users.end();
	user.setNickName(firstNickName);
	if (found || *nickIt == user.getNickName())
	{
		std::cout << "Collision found ! " << '\n';
		user.sendMsg(Server::getRPLString(RPL::ERR_NICKNAMEINUSE, "*", *nickIt, ":Nickname is already in use."));
	}
	else if (!__checkNickName(*nickIt))
	{
		std::cout << "Bad nick ! " << '\n';
		user.sendMsg(Server::getRPLString(RPL::ERR_ERRONEUSNICKNAME, "*", *nickIt, ":Erroneous nickname."));
	}
	else
	{
		user.sendMsg(user.getAllInfos() + " NICK " + *nickIt);
		user.setNickName(*nickIt);
	}
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
	newUser.setHostName(hostnameBuff);
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

void	Server::__joinExistingChan(vec_str_t const &msg, User const &user)
{
	Channel &chan = _channels.at(msg[1]);
	if (!chan.checkJoinConditions(user, msg))
		return;
	chan.addUser(user);
	chan.announceJoin(user, _users, __getChanUsersList(chan));
}

void	Server::__joinChannel(vec_str_t const &msg, User const &user)
{
	if (_channels.count(msg[1]))
		__joinExistingChan(msg, user);
	else
	{
		std::cout << "New Chan " << '\n';
		Channel &chan = (*_channels.insert(std::make_pair(msg[1], Channel(msg[1]))).first).second;
		chan.addUser(user);
		chan.setModeUser(user, Channel::CHAN_CREATOR);
		chan.announceJoin(user, _users, __getChanUsersList(chan));
	}
}

void	Server::__disconnectUser(User const &user, std::size_t const &i)
{
	std::cout << "User " << user << " disconected\n";
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		(*it).second.delUser(user);

	close(_pollingList[i].fd);
	_pollingList.erase(_pollingList.begin() + i);
	_users.erase(_users.begin() + (i - 1));
}

std::string const Server::formatMsg(std::string const &msg, User const &sender)
{
	std::vector<std::string> sp = split(msg, " ");
	std::string res = sender.getAllInfos();
	res.append(sp[0]);
	res.append(" ");
	res.append(sp[1]);
	res.append(" ");
	for (std::size_t i = 2; i < sp.size(); ++i)
		res.append(sp[i].append(" "));
	res[res.length() - 1] = '\0';
	return (res);
}

bool	Server::__isMultiArg(std::string const &str)
{
	return (str.find(":") != std::string::npos);
}

bool	Server::__isChanRelated(std::string const &str)
{
	return (str.find("#") != std::string::npos);
}

Server::map_chan_t::iterator Server::__searchChannel(std::string const &name, User const &user)
{
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it == _channels.end())
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, name, "No such channel"));
	return (it);
}


void	Server::__leaveChannel(vec_str_t const &msg, User const &user)
{
	std::string name = msg[1];

	vec_str_t::const_iterator partMsgIt = std::find_if(msg.begin(), msg.end(), __isMultiArg);

	std::string partMsg;
	if (partMsgIt != msg.end())
		partMsg = " " + std::string(&(msg.back()[1]));

	map_chan_t::iterator chanIt = __searchChannel(name, user);
	if (chanIt == _channels.end())
		return;
	Channel &chan = (*chanIt).second;
	if (!chan.isInChan(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOTONCHANNEL, user.getNickName(), name, ":You're not on that channel"));
		return;
	}
	chan.delUser(user);
	chan.delInvitedUser(user);
	user.sendMsg(Server::formatMsg(std::string("PART ") + name + partMsg, user));
	chan.broadcast(std::string("PART ") + name + partMsg, user, _users);
	if (chan.isEmpty())
		_channels.erase(name);
}

void	Server::__userPrivMsg(vec_str_t const &msg, User const &user)
{
	std::cout << "USERPV " << msg << '\n';
	if (msg[2].find("DCC") != std::string::npos)
	{
		__dccParsing(msg, user);
		return;
	}

	std::vector<User>::const_iterator target = getUserByNick(msg[1]);
	if (target == _users.end())
	{
		std::cout << "Not found " << msg[1] << '\n';
		return;
	}
	(*target).sendMsg(user.getAllInfos() + "PRIVMSG " + (*target).getNickName() + " " + msg[2]);
}

void	Server::__privMsg(vec_str_t const &msg, User const &user)
{
	std::string chanName = msg[1];
	if (chanName.find('#') == std::string::npos)
	{
		__userPrivMsg(msg, user);
		return;
	}

	map_chan_t::const_iterator chanIt = _channels.find(chanName);
	if (chanIt == _channels.end())
	{
		//ERROR HANDLING
		return;
	}

	Channel const &chan = (*chanIt).second;
	if (!chan.checkSendConditions(user, msg))
		return;

	std::cout << "Hash of msg: " << hash(vecToStr(msg)) << '\n';
	//TO DO: Changer structure broadcast pour garder le vector
	chan.broadcast(vecToStr(msg), user, _users);	
}

void	Server::__updateChannels(void)
{
	std::vector<std::string> emptyChans;
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (!(*it).second.getNbUsers())
			emptyChans.push_back((*it).first);
	}
	for (std::size_t i = 0; i < emptyChans.size(); ++i)
		_channels.erase(emptyChans[i]);
}

bool	Server::__isChanExist(std::string const &name) const
{
	return (_channels.count(name));
}


std::string Server::getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &reason)
{
	std::string res = "";
	res += numberToString(rpl);
	res += " ";
	res += arg1;
	res += " ";
	res += reason;
	return (res);
}

std::string Server::getRPLString(RPL::CODE const &rpl, std::string const &arg1, std::string const &arg2, std::string const &reason)
{
	std::string res = "";
	res += numberToString(rpl);
	res += " ";
	res += arg1;
	res += " ";
	res += arg2;
	res += " ";
	res += reason;
	return (res);
}

std::vector<User>::const_iterator Server::getUserByNick(std::string const &nick) const
{
	return (std::find_if(_users.begin(), _users.end(), nickCompByNick(nick)));
}

void	Server::__usrModeHandling(vec_str_t const &msg, User &user)
{
	if (msg[1] == user.getNickName() && !user.getMode() && msg[2] == "+i")
		user.setMode(User::INVISIBLE);
}

void	Server::__changeChanMode(vec_str_t const &msg, User &user)
{

	if (std::find_if(msg.begin(), msg.end(), __isChanRelated) == msg.end())
	{
		__usrModeHandling(msg, user);
		return;
	}

	if (!__isChanExist(msg[1]))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, msg[1], ":No such channel"));
		return;	
	}

	Channel &chan = _channels.at(msg[1]);

	if (msg.size() == 2)
	{
		user.sendMsg(Server::getRPLString(RPL::RPL_CHANNELMODEIS, user.getNickName(), msg[1], chan.getModeString()));
		return;	
	}

	if (!chan.checkCondition(msg[1], user))
		return ;
	
	std::string const possibilities = "imnptkl";
	if (msg[2].size() != 2 || possibilities.find(msg[2][1]) == std::string::npos)
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_UMODEUNKNOWNFLAG, msg[1], ":Unknown flag"));
		return;
	}

	uint8_t curMode = chan.getMode();
	if (msg[2][0] == '+')
		chan.updateMode(SET_N_BIT(curMode, (possibilities.find(msg[2][1]) + 1)));
	else
		chan.updateMode(CLEAR_N_BIT(curMode, (possibilities.find(msg[2][1]) + 1)));
	chan.broadcast(user.getAllInfos() + " MODE " + msg[1] + " " + msg[2], _users);

	if (msg[2][1] == 'l')
	{
		if (msg.size() == 4)
			chan.setMaxUser(std::atoll(msg[3].c_str()));
		else
			chan.setMaxUser(0);
	}
	else if (msg[2][1] == 'k')
	{
		if (msg.size() == 4)
			chan.setKey(numberToString(hash(msg[3])));
		else
		{
			chan.updateMode(CLEAR_N_BIT(curMode, (possibilities.find(msg[2][1]) + 1)));
			chan.setKey("");
		}
	}
}

void	Server::__sendPong(std::string const &msg, User const &user) const
{
	std::vector<std::string> sp = split(msg, " ");
	std::string rpl;
	rpl.append("PONG :");
	rpl.append(user.getHostName());
	user.sendMsg(rpl);
}

void	Server::__sendWelcomeMsg(User const &user)
{
	std::string msg2 = "";
	msg2.append(std::string("00") + numberToString(RPL::WELCOME));
	msg2.append(" ");
	msg2.append(user.getNickName());
	msg2.append(" Welcome to OurNetwork, ");
	msg2.append(&user.getAllInfos()[1]);
	user.sendMsg(msg2);
}

void	Server::__checkAuthClients(void)
{
	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].isReadyToAuth() && !_users[i].getAuthState() && _users[i].getPassword() == _password)
		{
			_users[i].setAuth(true);
			__sendWelcomeMsg(_users[i]);
		}
	}
}

void	Server::__userCMD(vec_str_t const &msg, User &user) const
{
	vec_str_t::const_iterator userIt = std::find(msg.begin(), msg.end(), "USER");
	if (user.getAuthState())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_ALREADYREGISTERED, "", ":You may not reregister."));
		return;
	}
	user.setName(*(++userIt));
	user.setFullName(&msg.back()[1]);
}


void	Server::__passCMD(vec_str_t const &msg, User &user) const
{
	vec_str_t::const_iterator passIt = ++(std::find(msg.begin(), msg.end(), "PASS"));
	if (user.getAuthState())
	{
		user.sendMsg(":" + user.getHostName() + " 462 " + user.getNickName() + "  :You may not reregister.");
		return;
	}
	user.setPassword(numberToString(hash(*passIt)));
}

void	Server::__inviteExistingChan(std::string const &chanName, std::string const &target, User const &user)
{
	std::vector<User>::const_iterator it = getUserByNick(target);
	if (it == _users.end())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHNICK, target, target,":No such nick"));
		return;
	}

	Channel &chan = _channels.at(chanName);
	if (!chan.isInChan(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOTONCHANNEL, user.getNickName(), chanName, ":You're not on that channel"));
		return;	
	}

	if (chan.isInvited(*it) || chan.isInChan(*it))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_USERONCHANNEL, target, chanName, ":User already in channel !"));
		return;	
	}

	if (chan.isInMode(BIT(Channel::INV_ONLY) | BIT(Channel::MODERATED) | BIT(Channel::PRIV)) && !chan.isOp(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_CHANOPRIVSNEEDED, user.getNickName(), ":You don't have permision to do this !"));
		return;	
	}
	chan.addInvitedUser(*it);
	user.sendMsg(Server::getRPLString(RPL::RPL_INVITING, user.getNickName(), target, chanName));
	(*it).sendMsg(Server::formatMsg("INVITE " + user.getNickName() + " " + chanName, user));
}

void	Server::__inviteCMD(vec_str_t const &msg, User const &user)
{
	std::string const targetUser = msg[1];
	std::string const chanName = msg[2];

	if (!__isChanExist(chanName))
	{
		std::vector<User>::const_iterator it = getUserByNick(targetUser);
		if (it == _users.end())
			user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHNICK, targetUser, targetUser,":No such nick"));
		else
		{
			user.sendMsg(Server::getRPLString(RPL::RPL_INVITING, targetUser, chanName, targetUser));
			(*it).sendMsg(Server::formatMsg("INVITE " + user.getNickName() + " " + chanName, user));
		}
	}
	else
		__inviteExistingChan(chanName, targetUser, user);
}

Server::vec_str_t Server::__parseCmd2(std::string str) const
{
	std::vector<std::string> res;
	str = __cleanMsg(str);
	std::size_t pos;
	while ((pos = str.find(" ")) != std::string::npos)
	{
		res.push_back(str.substr(0, pos));
		str.erase(0, pos + 1);
		if (res.back().find(":") != std::string::npos)
			break;
	}
	while ((pos = str.find(" ")) != std::string::npos)
	{
		res.back().append(" " + str.substr(0, pos));
		str.erase(0, pos + 1);
	}
	if (!str.empty())
		res.push_back(str);
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
	(*target).sendMsg(user.getAllInfos() + "PRIVMSG " + (*target).getNickName() + " " + msg[2] + "\r\n");
}

void	Server::__topicCMD(vec_str_t const &msg, User const &user)
{
	map_chan_t::iterator chanIt = __searchChannel(msg[1], user);
	if (chanIt == _channels.end())
		return;
	Channel &chan = (*chanIt).second;
	if (chan.isInMode(BIT(Channel::TOP_LOCK)) && !chan.isOp(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_CHANOPRIVSNEEDED, chan.getName(), ":You don't have permision to do this !"));
		return;
	}
	chan.setTopic(&msg[2][1]);
	chan.broadcast(user.getAllInfos() + " TOPIC " + msg[1] + " " + msg[2], _users);
}


void	Server::__handlePackets(void)
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
				std::vector<std::string> vecCmd = __parseCmd2(buffer);

				std::cout << vecCmd << '\n';
				std::map<std::string, ptrFonction>::iterator it = _serverCmd.end();
				if (vecCmd.size() >= 2)
					it = _serverCmd.find(vecCmd[1]);

				if (it != _serverCmd.end()) // A changer notamment pour la partie auth
					(*(it->second))(vecCmd); //exec cmd

				std::string msg(buffer);
				if (msg.size() == 1)
					continue;
				LOG_SEND(i, msg);
				msg.erase(msg.end() - 2, msg.end());

				if (msg.find("STOP") != std::string::npos)
					_isOn = false;

				if (msg.find("PASS") != std::string::npos)
					__passCMD(vecCmd, _users[i - 1]);
				if (msg.find("USER") != std::string::npos)
					__userCMD(vecCmd, _users[i - 1]);
				if (msg.find("NICK") != std::string::npos)
					__nickCMD(vecCmd, _users[i - 1]);
				
				if (msg.find("JOIN") != std::string::npos)
					__joinChannel(vecCmd, _users[i - 1]);
				else if (msg.find("PART") != std::string::npos)
					__leaveChannel(vecCmd, _users[i - 1]);


				else if (msg.find("PING") != std::string::npos)
					__sendPong(msg, _users[i - 1]);
				else if (msg.find("PRIVMSG") != std::string::npos)
					__privMsg(vecCmd, _users[i - 1]);
				else if (msg.find("MODE") != std::string::npos)
					__changeChanMode(vecCmd, _users[i - 1]);
				else if (msg.find("TOPIC") != std::string::npos)
					__topicCMD(vecCmd, _users[i - 1]);
				else if (msg.find("INVITE") != std::string::npos)
					__inviteCMD(vecCmd, _users[i - 1]);
			}
		}
	}
	__updateChannels();
	__checkAuthClients();
	// __printDebug();
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
		__printDebug();
	}
}