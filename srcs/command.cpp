#include "Server.hpp"

void	Server::__initCmd(void)
{
	_serverCmd["PASS"] = &Server::__passCMD;
	_serverCmd["USER"] = &Server::__userCMD;
	_serverCmd["NICK"] = &Server::__nickCMD;
	_serverCmd["JOIN"] = &Server::__joinChannel;
	_serverCmd["PART"] = &Server::__leaveChannel;
	_serverCmd["PING"] = &Server::__sendPong;
	_serverCmd["PRIVMSG"] = &Server::__privMsg;
	_serverCmd["NOTICE"] = &Server::__noticeCMD;
	_serverCmd["MODE"] = &Server::__modeCMD;
	_serverCmd["TOPIC"] = &Server::__topicCMD;
	_serverCmd["MOTD"] = &Server::__motdCMD;
	_serverCmd["INVITE"] = &Server::__inviteCMD;
	_serverCmd["KICK"] = &Server::__kickCMD;
	_serverCmd["QUIT"] = &Server::__quitCMD;
	_serverCmd["KILL"] = &Server::__killCMD;
	_serverCmd["AWAY"] = &Server::__awayCMD;
	_serverCmd["OPER"] = &Server::__operCMD;
	_serverCmd["DIE"] = &Server::__dieCMD;
	_serverCmd["LIST"] = &Server::__listCMD;
}

void	Server::__nickCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;
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
		if (user.getAuthState())
			user.sendMsg(user.getAllInfos() + " NICK " + *nickIt);
		user.setNickName(*nickIt);
	}
}

void	Server::__privMsg(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	std::string chanName = msg[1];
	if (chanName.find('#') == std::string::npos)
	{
		__userPrivMsg(msg, user);
		return;
	}

	map_chan_t::const_iterator chanIt = _channels.find(chanName);
	if (chanIt == _channels.end())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, user.getNickName() + " " + chanName, ":No such channel"));
		return;
	}

	Channel const &chan = (*chanIt).second;
	if (!chan.checkSendConditions(user))
		return;
	chan.broadcast(vecToStr(msg, 3), user, _users);
}

void	Server::__noticeCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	std::string chanName = msg[1];
	if (chanName.find('#') == std::string::npos)
	{
		std::vector<User>::const_iterator target = getUserByNick(msg[1]);
		if (target == _users.end())
			return;
		(*target).sendMsg(user.getAllInfos() + " NOTICE " + (*target).getNickName() + " " + msg[2]);
		return;
	}
	map_chan_t::const_iterator chanIt = _channels.find(chanName);
	std::cout << _channels.count(chanName) << '\n';
	if (chanIt == _channels.end())
		return;
	Channel const &chan = (*chanIt).second;
	if (chan.isInMode(BIT(Channel::NO_OUT)) && !chan.isInChan(user))
		return;
	if (chan.isInMode(BIT(Channel::MODERATED)) && !chan.isVoiced(user))
		return;
	chan.broadcast(vecToStr(msg, 3), user, _users);
}

void	Server::__modeCMD(vec_str_t const &msg, User &user)
{
	if (msg[1].find('#') == std::string::npos)
		__usrModeHandling(msg, user);
	else
		__changeChanMode(msg, user);
}

void	Server::__sendPong(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;
	user.sendMsg("PONG :" + msg[1]);
}

void	Server::__joinChannel(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;
	vec_str_t chans = split(msg[1], ",");
	std::cout << "chans: " << chans << '\n';
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		if (_channels.count(chans[i]))
			__joinExistingChan(chans, chans[i], user);
		else
		{
			std::cout << "New Chan " << '\n';
			Channel &chan = (*_channels.insert(std::make_pair(chans[i], Channel(chans[i]))).first).second;
			chan.addUser(user);
			chan.setModeUser(user, Channel::CHAN_CREATOR);
			chan.announceJoin(user, _users, __getChanUsersList(chan));
		}
	}
}

void	Server::__leaveChannel(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;

	vec_str_t chans = split(msg[1], ",");
	vec_str_t::const_iterator partMsgIt = std::find_if(msg.begin(), msg.end(), __isMultiArg);
	std::cout << chans << '\n';
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		std::string partMsg;
		if (partMsgIt != msg.end())
			partMsg = " " + std::string(&(msg.back()[1]));
		std::cout << "PARTMSG: " << partMsg << '\n';
		map_chan_t::iterator chanIt = __searchChannel(chans[i], user);
		if (chanIt == _channels.end())
			continue;
		Channel &chan = (*chanIt).second;
		if (!chan.isInChan(user))
		{
			user.sendMsg(Server::getRPLString(RPL::ERR_NOTONCHANNEL, user.getNickName(), chans[i], ":You're not on that channel"));
			continue;
		}
		chan.delUser(user);
		chan.delInvitedUser(user);
		user.sendMsg(Server::formatMsg(std::string("PART ") + chans[i] + partMsg, user));
		chan.broadcast(std::string("PART ") + chans[i] + partMsg, user, _users);
		if (chan.isEmpty())
			_channels.erase(chans[i]);
	}
}

void	Server::__userPrivMsg(vec_str_t const &msg, User &user)
{
	if (msg[2].find("DCC") != std::string::npos)
	{
		__dccParsing(msg, user);
		return;
	}

	std::vector<User>::const_iterator target = getUserByNick(msg[1]);
	if (target == _users.end())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHNICK, user.getNickName(), msg[1],":No such nick"));
		return;
	}
    if ((*target).checkMode(BIT(User::AWAY))) {
		user.sendMsg((*target).getNickName() + " is away" + (*target).getUnawayMsg());
		user.sendMsg((*target).getAllInfos() + " PRIVMSG " + (*target).getNickName() + " away message" + (*target).getUnawayMsg());
	}
	(*target).sendMsg(user.getAllInfos() + " PRIVMSG " + (*target).getNickName() + " " + msg[2]);
}

void	Server::__userCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 5, user))
		return;
	vec_str_t::const_iterator userIt = std::find(msg.begin(), msg.end(), "USER");
	if (user.getAuthState())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_ALREADYREGISTERED, "", ":You may not reregister."));
		return;
	}
	user.setName(*(++userIt));
	msg.back()[0] == ':' ? user.setFullName(&msg.back()[1]) : user.setFullName(msg.back());
}

void	Server::__passCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;
	vec_str_t::const_iterator passIt = ++(std::find(msg.begin(), msg.end(), "PASS"));

	std::string const pass = numberToString(hash(*passIt));

	if (pass != _password)
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_PASSWDMISMATCH, "PASS", ":Incorect Password !"));
		user.sendMsg("ERROR :Incorect Password !");
		user.setLeaving(true);
	}

	if (user.getAuthState())
	{
		user.sendMsg(":" + user.getHostName() + " 462 " + user.getNickName() + "  :You may not reregister.");
		return;
	}
	user.setPassword(pass);
}

void	Server::__inviteCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	std::string const targetUser = msg[1];
	std::string const chanName = msg[2];
	std::vector<User>::const_iterator it = getUserByNick(targetUser);

	if (it != _users.end() && it->checkMode(BIT(User::AWAY)))
		user.sendMsg((*it).getNickName() + " is away" + (*it).getUnawayMsg());
	if (!__isChanExist(chanName))
	{
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

void	Server::__topicCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 2, user))
		return;
	map_chan_t::iterator chanIt = __searchChannel(msg[1], user);
	if (chanIt == _channels.end())
		return;
	Channel &chan = (*chanIt).second;
	if (chan.isInMode(BIT(Channel::TOP_LOCK)) && !chan.isOp(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_CHANOPRIVSNEEDED, chan.getName(), ":You don't have permision to do this !"));
		return;
	}
	if (msg.size() == 2)
	{
		if (chan.getTopic().empty())
			user.sendMsg(Server::getRPLString(RPL::RPL_NOTOPIC, user.getNickName(), chan.getName() +  " :No topic is set"));
		else
			user.sendMsg(Server::getRPLString(RPL::RPL_TOPIC, user.getNickName(), chan.getName() +  " :" + chan.getTopic()));
		return;
	}
	chan.setTopic(&msg[2][1]);
	chan.broadcast(user.getAllInfos() + " TOPIC " + msg[1] + " " + msg[2], _users);
}

void	Server::__kickCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	vec_str_t chans = split(msg[1], ",");
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		if (!__isChanExist(chans[i]))
		{
			user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, user.getNickName() + " " + chans[i], ":No such channel"));
			continue;;
		}
		Channel &chan = _channels.at(chans[i]);

		vec_usr_t::const_iterator target = getUserByNick(msg[2]);
		if (target == _users.end() || !chan.isInChan(*target))
		{
			user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHNICK, msg[2], ":No such nick"));
			continue;
		}

		if (!chan.checkModifCondition(user))
			continue;
		std::string reason = "";
		if (msg.back() != ":")
			reason = msg[3];
		chan.broadcast(user.getAllInfos() + " KICK " + chans[i] + " " + msg[2] + " " + reason, _users);
		chan.delUser(*target);
		chan.delInvitedUser(*target);
	}
}

void	Server::__motdCMD(vec_str_t const &msg, User &user)
{
	(void)msg;
	user.sendMsg("375 " + user.getNickName() + " - Message of the day -");
	user.sendMsg("372 " + user.getNickName() + "   __ _     _          ");
	user.sendMsg("372 " + user.getNickName() + "  / _| |   (_)         ");
	user.sendMsg("372 " + user.getNickName() + " | |_| |_   _ _ __ ___ ");
	user.sendMsg("372 " + user.getNickName() + " |  _| __| | | '__/ __|");
	user.sendMsg("372 " + user.getNickName() + " | | | |_  | | | | (__ ");
	user.sendMsg("372 " + user.getNickName() + " |_|  \\__| |_|_|  \\___|");
	user.sendMsg("372 " + user.getNickName() + "       ______          ");
	user.sendMsg("372 " + user.getNickName() + "      |______|         ");
	user.sendMsg("376 " + user.getNickName() + " End of the Message the day");
}

void	Server::__operCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	std::vector<User>::iterator target = getUserByNick(msg[1]);
	if (msg[2] != PWD_OPERATOR)
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_PASSWDMISMATCH, "PASSWORD", msg[2],":invalid password"));
		return;
	}
	if (target == _users.end())
		return ;
	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		_users[i].sendMsg(msg[1] + " is the new operator of the server");
	}
	if ((*target).checkMode(BIT(User::OPERATOR)))
		user.sendMsg(getRPLString(RPL::RPL_YOUREOPER, "", ""));
	(*target).setMode(User::OPERATOR);
}

void	Server::__quitCMD(vec_str_t const &msg, User &user)
{
	user.setLeaving(true);
	for (std::size_t i = 0; i < _users.size(); ++i)
		_users[i].sendMsg(Server::formatMsg("QUIT " + msg[1], user));
}

void	Server::__killCMD(vec_str_t const &msg, User &user)
{
	if (!__checkMsgLen(msg, 3, user))
		return;
	if (!user.checkMode(BIT(User::OPERATOR)))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOPRIVILEGES, user.getNickName(), ":You don't have permision to do this !"));
		return;
	}
	vec_usr_t::iterator target = std::find_if(_users.begin(), _users.end(), nickCompByNick(msg[1]));
	if (target == _users.end())
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHNICK, user.getNickName(), msg[1],":No such nick"));
		return;
	}
	(*target).setLeaving(true);
	(*target).sendMsg(user.getAllInfos() + " KILL " + (*target).getNickName() + " " + msg[2]);
	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].getNickName() == (*target).getNickName())
			continue;
		_users[i].sendMsg(Server::getRPLString(RPL::RPL_KILLDONE, user.getNickName(), (*target).getNickName(), ":was killed by " + user.getNickName()));
	}
}

void	Server::__dieCMD(vec_str_t const &msg, User &user)
{
	(void) msg;
	if (user.checkMode(BIT(User::OPERATOR)))
		_isOn = false;
}

void	Server::__listCMD(vec_str_t const &msg, User &user) {
	if (msg.size() == 1) {
		for (map_chan_t::const_iterator chanIt = _channels.begin(); chanIt != _channels.end(); chanIt++) {
			std::stringstream ss;
			ss << chanIt->second.getNbUsers();
			std::string nbUser = ss.str();
			user.sendMsg("[" + chanIt->second.getName() + "] " + "nb user: " + nbUser + " topic: " + chanIt->second.getTopic());
		}
	} else {
		map_chan_t::const_iterator chanIt = _channels.find(msg[1]);
		if (chanIt == _channels.end())
		{
			user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, user.getNickName() + " " + msg[1], ":No such channel"));
			return ;
		}
		std::stringstream ss;
		ss << chanIt->second.getNbUsers();
		std::string nbUser = ss.str();
		user.sendMsg("[" + chanIt->second.getName() + "] " + "nb user: " + nbUser + " topic: " + chanIt->second.getTopic());
	}
}

void	Server::__awayCMD(vec_str_t const &msg, User &user) 
{
	user.setMode(User::AWAY);
	user.sendMsg(Server::getRPLString(RPL::RPL_NOWAWAY, "", "", ""));
	if (msg.size() > 1)
		user.setUnawayMsg(msg[1]);
	else
		user.setUnawayMsg("ZzZz");
	user.sendMsg("Away mesage is: " + user.getUnawayMsg());
}
