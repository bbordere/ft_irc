#ifndef RPL_HPP
#define RPL_HPP

namespace RPL
{
	enum CODE
	{
		NONE = 0,
		WELCOME = 001,

		ERR_NOSUCHCHANNEL = 403,
		ERR_PASSWDMISMATCH = 464,
		ERR_NICKCOLLISION = 436,
		ERR_NOTONCHANNEL = 442,
		ERR_ERRONEUSNICKNAME = 432,
		ERR_CHANOPRIVSNEEDED = 482,
		ERR_UMODEUNKNOWNFLAG = 501,
	};
} // namespace RPL


#endif