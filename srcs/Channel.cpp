#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(std::string const &name): _name(name), _topic("Random Topic"),
											_mode(0) {}

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
	_invitedList.push_back(user.getId());
}

void	Channel::delInvitedUser(User const &user)
{
	std::vector<uint32_t>::const_iterator curUser = std::find(_invitedList.begin(), _invitedList.end(), user.getId());
	if (curUser != _invitedList.end())
		_invitedList.erase(curUser);
}

void	Channel::updateModeUser(User const &user, std::string const &mode)
{
	(void)user;
	(void)mode;
}

void	Channel::updateMode(uint8_t const mode)
{
	_mode = mode;
}

bool	Channel::isInvited(User const &user) const
{
	return (std::find(_invitedList.begin(), _invitedList.end(), user.getId()) != _invitedList.end());
}

bool	Channel::isInChan(User const &user) const
{
	return (_users.count(user.getId()));
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

void	Channel::broadcast(std::string const &msg, User const &sender, std::vector<User> const &users)
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


uint8_t Channel::getMode(void) const
{
	return (_mode);
}

std::ostream &operator<<(std::ostream &stream, Channel const &chan)
{
	(void)chan;
	stream << "Chan";
	return (stream);
}