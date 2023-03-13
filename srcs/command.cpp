#include "irc.hpp"
#include "struct.hpp"

void*	nickCmd(std::vector<std::string> cmdLine) {
	// if (cmdLine.size < 3)
	// 	send ERR_NONICKNAMEGIVEN 431;
	// if (cmdLine[2] == _user._name) //Nickname is already in use
	//  send ERR_NICKNAMEINUSE
	// if (cmdLine[2].size > 9)// Nickname not in set charactere rule
	//  send ERR_ERRONEUSNICKNAME 432;
	// if (_users.find(cmdLine[3]))
	//  send ERR_NICKCOLLISION 436;
	// if (_user._mode == -r)
	//  send ERR_RESTRICTED 484;

	//       437    ERR_UNAVAILRESOURCE//timer invalid

	//user.nickName = cmdLine[3];

	return (0);
}

void*	userCmd(std::vector<std::string> cmdLine) {
	// if (cmdLine.size < 6)
	// 	send ERR_NEEDMOREPARAMS 431;
	// if (_users.find(cmdLine[3]))
	// 	send ERR_ALREADYREGISTRED 462;

	// add new user, je sais pas comment tu veux l'implémenter

	return (0);
}

void*	operCmd(std::vector<std::string> cmdLine) {
	//User* user;
	
	// if (cmdLine.size < 4)
	// 	send ERR_NEEDMOREPARAMS 431;
	// if (user = _user.find(cmdLine[2] == _users.end())
	//	return (1):
	// if (cmdLine[3] != pasword)
	//	send ERR_PASSWDMISMATCH 464;
    //    491    ERR_NOOPERHOST
    //           ":No O-lines for your host"

    //      - If a client sends an OPER message and the server has
    //        not been configured to allow connections from the
    //        client's host as an operator, this error MUST be
    //        returned.

	//	user mode add opp
	//	send RPL_YOUREOPER 381;

	return (0);
}

void*	modeCmd(std::vector<std::string> cmdLine) {
	
	//a voir avec bastien
	
	return (0);
}

void*	quitCmd(std::vector<std::string> cmdLine) {
	
	// voir avec bastien
	
	return (0);
}

void*	squitCmd(std::vector<std::string> cmdLine) {
	// if (_user.mode != -O)
	//  send ERR_NOPRIVILEGES 481; 
	// if (cmdLine.size < 3)
	// 	send ERR_NEEDMOREPARAMS 461;
	// if (cmdLine[2] != )
	// 	send ERR_NOSUCHSERVER  402;
	return (0);
}

void*	joinCmd(std::vector<std::string> cmdLine) {

	//channel part, voir avec bastien

	return (0);
}

void*	killCmd(std::vector<std::string> cmdLine) {



	return (0);
}

void*	kickCmd(std::vector<std::string> cmdLine) {

	//kick user from channel, voir avec bastien

	return (0);
}

void*	errorCmd(std::vector<std::string> cmdLine) {

	// remove connection from serveur, voir avec bastien

	return (0);
}

void*	partCmd(std::vector<std::string> cmdLine) {

	//channel part, voir avec bastien

	return (0);
}

void*	privmsgCmd(std::vector<std::string> cmdLine) {

	//prv part

	return (0);
}

void*	noticeCmd(std::vector<std::string> cmdLine) {

	//prv part

	return (0);
}

void*	awayCmd(std::vector<std::string> cmdLine) {

	//prv part

	return (0);
}

void*	dieCmd(std::vector<std::string> cmdLine) {

	// if (_user.mode != -O)
	//  send ERR_NOPRIVILEGES 481; 

	// server.isOn = FALLS;

	return (0);
}

void*	restartCmd(std::vector<std::string> cmdLine) {

	//voir avec bastien

	return (0);
}