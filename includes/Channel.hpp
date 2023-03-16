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

		enum CHAN_MODES
		{
			INV_ONLY = 1,
			MODERATED,
			NO_OUT,
			PRIV,
			TOP_LOCK,
			KEY_LOCK,
			USR_LIM
		};

		enum USER_MODES
		{
			CHAN_CREATOR = 1,
			CHAN_OP,
			VOICE
		};

		userMap_t _users;
		void	setName(std::string const &name);
		void	setTopic(std::string const &topic);

		void	addUser(User const &user);
		void	delUser(User const &user);

		void	addInvitedUser(User const &user);
		void	delInvitedUser(User const &user);

		void	setModeUser(User const &user, USER_MODES const mode);
		void	unsetModeUser(User const &user, USER_MODES const mode);

		bool	checkCondition(std::string const &name, User const &user) const;

		void	updateMode(uint8_t const mode);
		std::size_t getNbUsers(void) const;

		bool	isInvited(User const &user) const;
		bool	isInChan(User const &user) const;
		bool	isOp(User const &user) const;
		bool	isEmpty(void) const;

		uint8_t getMode(void) const;

		std::string const &getName(void) const;
		std::string const &getTopic(void) const;
		std::string getModeString(void) const;

		Channel(std::string const &name);
		void	broadcast(std::string const &msg, User const &sender,
						std::vector<User> const &users);

		void	broadcast(std::string const &msg, std::vector<User> const &users) const;
};

std::ostream &operator<<(std::ostream &stream, Channel const &chan);

#endif