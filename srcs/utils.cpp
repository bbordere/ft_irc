#include "irc.hpp"
#include "Server.hpp"


std::vector<std::string> split(std::string str, std::string const &delimiter)
{
	std::vector<std::string> res;
	std::size_t pos;

	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		res.push_back(str.substr(0, pos));
		str.erase(0, pos + delimiter.length());
	}
	if (!str.empty())
		res.push_back(str);
	return (res);
}

uint64_t hash(std::string const &str)
{
	uint64_t hash = 5381;

	for (std::size_t i = 0; i < str.length(); ++i)
		hash = ((hash << 5) + hash) + static_cast<int>(str[i]);
	return (hash);
}

void	sigHandler(int sig)
{
	(void)sig;
	throw (std::exception());
}

std::string vecToStr(std::vector<std::string> const &vec, std::size_t len)
{
	std::string res;
	if (len == std::string::npos)
		len = vec.size();
	for (std::size_t i = 0; i < len - 1; ++i)
		res += vec[i] + " ";
	res += vec[len - 1];
	return (res);
}

void	Server::__joinExistingChan(vec_str_t const &msg, std::string const &name, User const &user)
{
	Channel &chan = _channels.at(name);
	if (!chan.checkJoinConditions(user, msg))
		return;
	chan.addUser(user);
	chan.announceJoin(user, _users, __getChanUsersList(chan));
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

bool	Server::userCompFct(std::string const &nick1, std::string const &nick2)
{
	return (nick1 == nick2);
}

std::string Server::__getChanUsersList(Channel const &chan) const
{
	std::string res = "";

	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		if (chan.isInChan(_users[i]))
		{
			if (chan.isOp(_users[i]))
				res += '@';
			else if (chan.isVoiced(_users[i]))
				res += '+';
			res += _users[i].getNickName() + " ";
		}
	}
	return (res);
}

std::string const Server::formatMsg(std::string const &msg, User const &sender)
{
	std::vector<std::string> sp = split(msg, " ");
	std::string res = sender.getAllInfos() + " ";
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
	std::cout << name << '\n';
	if (it == _channels.end())
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, user.getNickName() + " " + name, ":No such channel"));
	return (it);
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

bool	Server::__isChanExist(std::string const &name) const
{
	return (_channels.count(name));
}

std::vector<User>::const_iterator Server::getUserByNick(std::string const &nick) const
{
	return (std::find_if(_users.begin(), _users.end(), nickCompByNick(nick)));
}

std::vector<User>::iterator Server::getUserByNick(std::string const &nick)
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
	if (!__isChanExist(msg[1]))
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NOSUCHCHANNEL, user.getNickName() + " " + msg[1], ":No such channel"));
		return;	
	}
	Channel &chan = _channels.at(msg[1]);

	if (msg.size() == 2)
	{
		user.sendMsg(Server::getRPLString(RPL::RPL_CHANNELMODEIS, user.getNickName(), msg[1], chan.getModeString()));
			return;
	}
	if (msg.size() == 3)
	{
		chan.changeMode(msg, user, _users);
		return;
	}

	if (msg[2].find("+o") != std::string::npos || msg[2].find("-o") != std::string::npos)
		chan.changeUserMode(msg, user, _users);
	else
		chan.changeMode(msg, user, _users);
}

size_t Server::__getUserIndex(std::string const &nick) const
{
	for (std::size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].getNickName() == nick)
			return (i);
	}
	return (0);
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

bool	Server::__checkMsgLen(vec_str_t const &msg, std::size_t const expected, User const &user) const
{
	if (msg.size() < expected)
	{
		user.sendMsg(Server::getRPLString(RPL::ERR_NEEDMOREPARAMS, user.getNickName(), msg[0] + " :Not enough parameters"));
		return (false);
	}
	return (true);
}

void	Server::__leaveAllChan(User const &user)
{
	for (map_chan_t::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if ((*it).second.isInChan(user))
		{
			(*it).second.delUser(user);
			if ((*it).second.isInvited(user))
				(*it).second.delInvitedUser(user);
		}
	}
}
