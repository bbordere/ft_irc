#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(std::string const &name): _name(name), _topic("Default Topic"),
											_key(""), _maxUsers(0), _mode(0) {}

void	Channel::setName(std::string const &name)
{
	_name = name;
}

void	Channel::setTopic(std::string const &topic)
{
	_topic = topic;
}

std::string const &Channel::getName(void) const
{
	return (_name);
}

std::string const &Channel::getTopic(void) const
{
	return (_topic);
}

std::string const &Channel::getKey(void) const
{
	return (_key);
}

void	Channel::addUser(User const &user)
{
	_users.insert(std::make_pair(user.getId(), 0));
}

void	Channel::delUser(User const &user)
{
	_users.erase(user.getId());
}

void	Channel::addInvitedUser(User const &user)
{
	_invitedSet.insert(user.getId());
}

void	Channel::delInvitedUser(User const &user)
{
	std::set<uint32_t>::iterator it = _invitedSet.find(user.getId());
	if (it != _invitedSet.end())
		_invitedSet.erase(it);
}

void	Channel::setModeUser(User const &user, USER_MODES const mode)
{
	if (!isInChan(user))
		return;
	uint16_t cur = _users.at(user.getId());
	_users[user.getId()] = SET_N_BIT(cur, mode);
}

void	Channel::unsetModeUser(User const &user, USER_MODES const mode)
{
	if (!isInChan(user))
		return;
	uint16_t cur = _users.at(user.getId());
	_users[user.getId()] = CLEAR_N_BIT(cur, mode);
}

void	Channel::setMaxUser(uint64_t const &newLimit)
{
	_maxUsers = newLimit;
}

void	Channel::setKey(std::string const &key)
{
	_key = key;
}

bool	Channel::checkCondition(std::string const &name, User const &user) const
{
	if (!isInChan(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOTONCHANNEL, name, ":You are not in this channel !"));
		return (false);
	}
	if (!isOp(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_CHANOPRIVSNEEDED, name, ":You don't have permision to do this !"));
		return (false);
	}
	return (true);
}

void	Channel::updateMode(uint8_t const mode)
{
	_mode = mode;
}

bool	Channel::isInvited(User const &user) const
{
	return (_invitedSet.count(user.getId()));
}

bool	Channel::isInChan(User const &user) const
{
	return (_users.count(user.getId()));
}

bool	Channel::isOp(User const &user) const
{
	return (GET_N_BIT(_users.at(user.getId()), CHAN_CREATOR) || GET_N_BIT(_users.at(user.getId()), CHAN_OP));
}

bool	Channel::isVoiced(User const &user) const
{
	return (isOp(user) || GET_N_BIT(_users.at(user.getId()), VOICE));
}

bool	Channel::isInMode(uint8_t const &mode) const
{
	return	 ((_mode & mode) != 0);
}

bool	Channel::isEmpty(void) const
{
	return (_users.empty());
}

bool	Channel::checkUserLimit(void) const
{
	return (!_maxUsers || _users.size() < _maxUsers);
}

void	Channel::announceJoin(User const &user, std::vector<User> const &users, std::string const &usersList) const
{
	user.sendMsg(Server::formatMsg(std::string("JOIN ") + _name + std::string(" Join Message"), user));
	user.sendMsg(Server::getRPLString(RPL::RPL_NAMREPLY, user.getNickName(), "= " + _name, ":" + usersList));
	user.sendMsg(Server::getRPLString(RPL::RPL_ENDOFNAMES, user.getNickName(), _name, ":End of /NAMES LIST"));
	broadcast(std::string("JOIN ") + _name + std::string(" Join Message"), user, users);
}

bool	Channel::checkJoinConditions(User const &user, vec_str_t const &msg) const
{

	std::pair<RPL::CODE, std::string> rpl[] = {std::make_pair(RPL::ERR_INVITEONLYCHAN, ":Cannot join channel (+i)"),
											std::make_pair(RPL::ERR_BADCHANNELKEY, ":Cannot join channel (+k) - bad key"),
											std::make_pair(RPL::ERR_CHANNELISFULL, ":Chan is full"),
											std::make_pair(RPL::ERR_INVITEONLYCHAN, ":Cannot join private channel"),
											std::make_pair(RPL::ERR_BANNEDFROMCHAN, ":You're banned from this channel")};


	/*
	**Tab pour les check les conditions
	** first -> condition a respecter
	** second -> condition est-elle respectee ?
	*/
	//TO DO ADD BAN
	std::pair<bool, bool> conditions[] = {std::make_pair(isInMode(BIT(Channel::INV_ONLY)), isInvited(user)),
											std::make_pair(isInMode(BIT(Channel::KEY_LOCK)), (msg.size() == 3 && numberToString(hash(msg[2])) == _key)),
											std::make_pair(isInMode(BIT(Channel::USR_LIM)), checkUserLimit()),
											std::make_pair(isInMode(BIT(Channel::PRIV)), isInvited(user))};
	for (std::size_t i = 0; i < 4; ++i)
	{
		if (conditions[i].first && !conditions[i].second)
		{
			user.sendMsg(Server::getRPLString(rpl[i].first, user.getNickName(), _name, rpl[i].second));
			return (false);
		}
	}
	return (true);
}

bool	Channel::checkSendConditions(User const &user, vec_str_t const &msg) const
{
	if (isInMode(BIT(Channel::NO_OUT)) && !isInChan(user))
	{
		user.sendMsg(Server::formatMsg(numberToString(RPL::ERR_CANNOTSENDTOCHAN) + " " + _name + std::string(" :Not in chan"), user));
		return (false);
	}
	if (isInMode(BIT(Channel::MODERATED)) && !isVoiced(user))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_CANNOTSENDTOCHAN, _name, ":You don't have permision to talk here !"));
		return (false);
	}
	return (true);
}

std::string const Channel::__formatMsg(std::string const &msg, User const &sender)
{
	std::vector<std::string> sp = split(msg, " ");
	std::string res = sender.getAllInfos();
	res.append(sp[0]);
	res.append(" ");
	res.append(sp[1]);
	res.append(" ");

	for (std::size_t i = 2; i < sp.size(); ++i)
	{
		res.append(sp[i].append(" "));
	}
	res.append("\n");
	return (res);
}

void	Channel::broadcast(std::string const &msg, std::vector<User> const &users) const
{
	for (std::size_t i = 0; i < users.size(); ++i)
	{
		if (_users.count(users[i].getId()))
			users[i].sendMsg(msg);
	}
}

std::string Channel::getModeString(void) const
{
	std::string res = "+";
	bool	const activeModes[7] = {
						GET_N_BIT(_mode, INV_ONLY), GET_N_BIT(_mode, MODERATED),
						GET_N_BIT(_mode, NO_OUT), GET_N_BIT(_mode, PRIV),
						GET_N_BIT(_mode, TOP_LOCK), GET_N_BIT(_mode, KEY_LOCK),
						GET_N_BIT(_mode, USR_LIM)};
	std::string const possibilities = "imnptkl";
	for (unsigned char i = 0; i < 7; ++i)
	{
		if (activeModes[i])
			res += possibilities[i];
	}
	if (res == "+")
		res = "";
	return (res);
}

void	Channel::broadcast(std::string const &msg, User const &sender, std::vector<User> const &users) const
{
	for (std::size_t i = 0; i < users.size(); ++i)
	{
		if (users[i].getId() != sender.getId() && _users.count(users[i].getId()))
			users[i].sendMsg(Server::formatMsg(msg, sender));
	}
}

std::size_t Channel::getNbUsers(void) const
{
	return (_users.size());
}

uint64_t	Channel::getMaxUsers(void) const
{
	return (_maxUsers);
}

uint8_t Channel::getMode(void) const
{
	return (_mode);
}

std::ostream &operator<<(std::ostream &stream, Channel const &chan)
{
	stream << "{Topic: " << chan.getTopic() << ", key: " << (chan.getKey().empty() ? "\"\"" : chan.getKey());
	stream << ", maxUser: " << chan.getMaxUsers() << ", mode: " << (chan.getModeString().empty() ? "\"\"" : chan.getModeString());
	stream << ", nb User: " << chan.getNbUsers() << '}';
	return (stream);
}
