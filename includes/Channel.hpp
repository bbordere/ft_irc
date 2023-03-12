#ifndef CHANNELL_HPP
#define CHANNELL_HPP

#include "irc.hpp"
#include "User.hpp"
#include "utils.hpp"

class Channel
{

	private:
		typedef std::map<uint32_t, uint16_t> userMap_t;

		std::vector<uint32_t> _invitedList;
		std::string	_name;
		std::string	_topic;

		uint8_t	_mode;

		std::string const __formatMsg(std::string const &msg, User const &sender);
	
	public:

		enum
		{
			INV_ONLY = 1,
			MODERATED,
			NO_OUT,
			PRIV,
			TOP_LOCK,
			KEY_LOCK,
			USR_LIM
		};

		userMap_t _users;
		void	setName(std::string const &name);
		void	setTopic(std::string const &topic);

		void	addUser(User const &user);
		void	delUser(User const &user);

		void	addInvitedUser(User const &user);
		void	delInvitedUser(User const &user);

		void	updateModeUser(User const &user, std::string const &mode);
		void	updateMode(uint8_t const mode);
		std::size_t getNbUsers(void) const;

		bool	isInvited(User const &user) const;
		bool	isInChan(User const &user) const;

		uint8_t getMode(void) const;

		std::string const &getName(void) const;
		std::string const &getTopic(void) const;

		Channel(std::string const &name);
		void	broadcast(std::string const &msg, User const &sender,
						std::vector<User> const &users);
};

std::ostream &operator<<(std::ostream &stream, Channel const &chan);

#endif