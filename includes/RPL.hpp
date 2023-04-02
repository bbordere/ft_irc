#ifndef RPL_HPP
#define RPL_HPP

namespace RPL
{
	enum CODE
	{
		NONE = 0,
		WELCOME = 1,
		RPL_CHANNELMODEIS = 324,
		RPL_INVITING = 341,
		RPL_NAMREPLY = 353,
		RPL_ENDOFNAMES = 366,
		ERR_NOSUCHNICK = 401,
		ERR_NOSUCHCHANNEL = 403,
		ERR_CANNOTSENDTOCHAN = 404,
		ERR_ERRONEUSNICKNAME = 432,
		ERR_NICKNAMEINUSE = 433,
		ERR_NICKCOLLISION = 436,
		ERR_NOTONCHANNEL = 442,
		ERR_USERONCHANNEL = 443,
		ERR_ALREADYREGISTERED = 462,
		ERR_PASSWDMISMATCH = 464,
		ERR_CHANNELISFULL = 471,
		ERR_INVITEONLYCHAN = 473,
		ERR_BANNEDFROMCHAN = 474,
		ERR_BADCHANNELKEY = 475,
		ERR_CHANOPRIVSNEEDED = 482,
		ERR_UMODEUNKNOWNFLAG = 501,
	};
} // namespace RPL


#endif