#ifndef CHANNELL_HPP
#define CHANNELL_HPP

#include "irc.hpp"
#include "User.hpp"
#include "utils.hpp"

class Channel
{
	private:
		typedef std::map<User *, uint8_t> userMap_t;
		std::string	_name;
		std::string	_topic;
		uint8_t	_mode; // A voir pour changer avec du bit masking
	
	public:
		userMap_t _users;
		void	setName(std::string const &name);
		void	setTopic(std::string const &topic);

		void	addUser(User const &user);
		void	delUser(User const &user);

		void	updateModeUser(User const &user, std::string const &mode);

		std::string const &getName(void) const;
		std::string const &getTopic(void) const;

		Channel(std::string const &name);
		void	broadcast(std::string::const_iterator first, 
							std::string::const_iterator last, 
							User const *sender);
};

std::ostream &operator<<(std::ostream &stream, Channel const &chan);

#endif