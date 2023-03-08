#include "Channel.hpp"

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
	_users.insert(std::make_pair(const_cast<User *>(&user), 0));
}

void	Channel::delUser(User const &user)
{
	_users.erase(const_cast<User *>(&user));
}

void	Channel::updateModeUser(User const &user, std::string const &mode)
{
	(void)user;
	(void)mode;
}

void	Channel::broadcast(std::string::const_iterator first, 
							std::string::const_iterator last, 
								User const *sender)
{
	std::string msg = sender->getAllInfos();
	msg.append("PRIVMSG ");
	msg.append(_name);
	msg.append(" ");
	while (first != last)
	{
		msg += *first;
		++first;
	}
	msg.append("\n");
	std::cout << msg;
	for (userMap_t::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if ((*it).first == sender)
			continue;
		((*it).first)->sendMsg(msg);
	}
}

std::ostream &operator<<(std::ostream &stream, Channel const &chan)
{
	stream << "Chan";
	return (stream);
}
