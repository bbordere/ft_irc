#ifndef RPL_HPP
#define RPL_HPP

namespace RPL
{
	enum CODE
	{
		NONE = 0,
		WELCOME = 001,

		ERR_PASSWDMISMATCH = 464,
		ERR_NICKCOLLISION = 436,
		ERR_NOTONCHANNEL = 442,
	};
} // namespace RPL


#endif