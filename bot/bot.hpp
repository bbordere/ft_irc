#ifndef BOT_HPP
#define BOT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

class BotFailureException: public std::exception
{
	private:
		const char *_msg;
	
	public:
		BotFailureException(char const *msg);
		virtual const char* what() const throw();
};

BotFailureException::BotFailureException(char const *msg): _msg(msg) {}

const char *BotFailureException::what() const throw()
{
	return (_msg);
}

class Bot
{
	private:
		std::string _botName;
		std::string	_password;
		std::string	_chanName;
		std::string	_buffer;
		int			_clientSocket;
		int			_apiSocket;
		bool		_isConnected;
		struct sockaddr_in _serverAddr;

	public:
		Bot(std::size_t port, std::string const &serverIp, std::string const &password);
		~Bot();
		std::string recvMsg(int sock) const;

		std::string getReqString(std::string const &host, std::string const &req) const;
		void		apiHandling(std::string const &host, std::string const &req);
		void		parseApiContent(std::string const &host, std::string const &content) const;

		void		helpHandling(void) const;

		void		resetApiSocket(std::string const &name);
		void		sendMsg(int sock, std::string const &msg) const;
		void		sendMsgChan(std::string const &msg) const;
		void		rickRollHandling(void) const;
		void		auth(void);
		void		run(void);
};

#endif