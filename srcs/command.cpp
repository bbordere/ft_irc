#include "Server.hpp"

void	Server::_initCmd(void)
{
	_serverCmd["PASS"] = &Server::_passCMD;
	_serverCmd["USER"] = &Server::_userCMD;
	_serverCmd["NICK"] = &Server::_nickCMD;
	_serverCmd["JOIN"] = &Server::_joinChannel;
	_serverCmd["PART"] = &Server::_leaveChannel;
	_serverCmd["PING"] = &Server::_sendPong;
	_serverCmd["PRIVMSG"] = &Server::_privMsg;
	_serverCmd["NOTICE"] = &Server::_noticeCMD;
	_serverCmd["MODE"] = &Server::_modeCMD;
	_serverCmd["TOPIC"] = &Server::_topicCMD;
	_serverCmd["MOTD"] = &Server::_motdCMD;
	_serverCmd["INVITE"] = &Server::_inviteCMD;
	_serverCmd["KICK"] = &Server::_kickCMD;
	_serverCmd["QUIT"] = &Server::_quitCMD;
	_serverCmd["KILL"] = &Server::_killCMD;
	_serverCmd["AWAY"] = &Server::_awayCMD;
	_serverCmd["OPER"] = &Server::_operCMD;
	_serverCmd["DIE"] = &Server::_dieCMD;
	_serverCmd["LIST"] = &Server::_listCMD;
}

void	Server::_nickCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
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
	else if (!_checkNickName(*nickIt))
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

void	Server::_privMsg(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
		return;
	std::string chanName = msg[1];
	if (chanName.find('#') == std::string::npos)
	{
		_userPrivMsg(msg, user);
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

void	Server::_noticeCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
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

void	Server::_modeCMD(vec_str_t const &msg, User &user)
{
	if (msg[1].find('#') == std::string::npos)
		_usrModeHandling(msg, user);
	else
		_changeChanMode(msg, user);
}

void	Server::_sendPong(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
		return;
	user.sendMsg("PONG :" + msg[1]);
}

void	Server::_joinChannel(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
		return;
	vec_str_t chans = split(msg[1], ",");
	std::cout << "chans: " << chans << '\n';
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		if (_channels.count(chans[i]))
			_joinExistingChan(msg, chans[i], user);
		else
		{
			std::cout << "New Chan " << '\n';
			Channel &chan = (*_channels.insert(std::make_pair(chans[i], Channel(chans[i]))).first).second;
			chan.addUser(user);
			chan.setModeUser(user, Channel::CHAN_CREATOR);
			chan.announceJoin(user, _users, _getChanUsersList(chan));
		}
	}
}

void	Server::_leaveChannel(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
		return;

	vec_str_t chans = split(msg[1], ",");
	vec_str_t::const_iterator partMsgIt = std::find_if(msg.begin(), msg.end(), _isMultiArg);
	std::cout << chans << '\n';
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		std::string partMsg;
		if (partMsgIt != msg.end())
			partMsg = " " + std::string(&(msg.back()[1]));
		std::cout << "PARTMSG: " << partMsg << '\n';
		map_chan_t::iterator chanIt = _searchChannel(chans[i], user);
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

void	Server::_userPrivMsg(vec_str_t const &msg, User &user)
{
	if (msg[2].find("DCC") != std::string::npos)
	{
		_dccParsing(msg, user);
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

void	Server::_userCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 5, user))
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

void	Server::_passCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
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

void	Server::_inviteCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
		return;
	std::string const targetUser = msg[1];
	std::string const chanName = msg[2];
	std::vector<User>::const_iterator it = getUserByNick(targetUser);

	if (it != _users.end() && it->checkMode(BIT(User::AWAY)))
		user.sendMsg((*it).getNickName() + " is away" + (*it).getUnawayMsg());
	if (!_isChanExist(chanName))
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
		_inviteExistingChan(chanName, targetUser, user);
}

void	Server::_topicCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 2, user))
		return;
	map_chan_t::iterator chanIt = _searchChannel(msg[1], user);
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

void	Server::_kickCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
		return;
	vec_str_t chans = split(msg[1], ",");
	for (std::size_t i = 0; i < chans.size(); ++i)
	{
		if (!_isChanExist(chans[i]))
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

void	Server::_motdCMD(vec_str_t const &msg, User &user)
{
	(void)msg;
	user.sendMsg("375 " + user.getNickName() + " - Message of the day -");
	user.sendMsg("372 " + user.getNickName() + "   _ _     _          ");
	user.sendMsg("372 " + user.getNickName() + "  / _| |   (_)         ");
	user.sendMsg("372 " + user.getNickName() + " | |_| |_   _ _ _ __ ");
	user.sendMsg("372 " + user.getNickName() + " |  _| _| | | '_/ _|");
	user.sendMsg("372 " + user.getNickName() + " | | | |_  | | | | (_ ");
	user.sendMsg("372 " + user.getNickName() + " |_|  \\_| |_|_|  \\__|");
	user.sendMsg("372 " + user.getNickName() + "       ___          ");
	user.sendMsg("372 " + user.getNickName() + "      |___|         ");
	user.sendMsg("376 " + user.getNickName() + " End of the Message the day");
}

void	Server::_operCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
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

void	Server::_quitCMD(vec_str_t const &msg, User &user)
{
	user.setLeaving(true);
	for (std::size_t i = 0; i < _users.size(); ++i)
		_users[i].sendMsg(Server::formatMsg("QUIT " + msg[1], user));
}

void	Server::_killCMD(vec_str_t const &msg, User &user)
{
	if (!_checkMsgLen(msg, 3, user))
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

void	Server::_dieCMD(vec_str_t const &msg, User &user)
{
	(void) msg;
	if (user.checkMode(BIT(User::OPERATOR)))
		_isOn = false;
}

void	Server::_listCMD(vec_str_t const &msg, User &user) {
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

void	Server::_awayCMD(vec_str_t const &msg, User &user) 
{
	user.setMode(User::AWAY);
	user.sendMsg(Server::getRPLString(RPL::RPL_NOWAWAY, "", "", ""));
	if (msg.size() > 1)
		user.setUnawayMsg(msg[1]);
	else
		user.setUnawayMsg("ZzZz");
	user.sendMsg("Away mesage is: " + user.getUnawayMsg());
}
