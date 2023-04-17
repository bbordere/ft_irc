#include "bot.hpp"

Bot::Bot(std::size_t port, std::string const &serverIp, std::string const &password):
	_botName("beBot"), _password(password), _chanName(""), _buffer(""), _clientSocket(-1), _apiSocket(-1),
	_isConnected(false), _serverAddr()
{
	_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_clientSocket < 0)
		throw BotFailureException("Socket Failure");
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(port);
	struct addrinfo hints = {};
	struct addrinfo *servInfo;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(serverIp.c_str() , 0 , &hints , &servInfo);
	_serverAddr.sin_addr = ((struct sockaddr_in *) servInfo->ai_addr)->sin_addr;
	freeaddrinfo(servInfo);
}

Bot::~Bot()
{
	sendMsg(_clientSocket, "QUIT :Leaving\r\n");
	if (_clientSocket != -1)
		close(_clientSocket);
}

void		Bot::sendMsg(int sock, std::string const &msg) const
{
	if (send(sock, msg.c_str(), msg.length(), MSG_NOSIGNAL) < 0)
		throw BotFailureException("send Failure");
	std::cout << "Bot send: " << msg << '\n';
}

void		Bot::sendMsgChan(std::string const &msg) const
{
	std::string toSend = "NOTICE " + _chanName + " :" + msg + "\r\n";
	sendMsg(_clientSocket, toSend);
}

std::string Bot::getReqString(std::string const &host, std::string const &req) const
{
	std::string res = "GET " + req + " HTTP/1.1\r\n";
	res.append("Host: " + host + "\r\n");
	res.append("Connection: close\r\n\r\n");
	return (res);
}


std::string Bot::recvMsg(int sock) const
{
	static struct pollfd pfd;
	pfd.fd = sock;
	pfd.events = POLLIN;
	poll(&pfd, 1, -1);
	char buff[1024] = {};
	std::size_t bytes;
	if ((bytes = recv(sock, buff, 1024, 0) < 0))
		throw BotFailureException("recv Failure");
	std::cout << "Bot receive: " << buff << '\n';
	return (buff);
}

void	Bot::resetApiSocket(std::string const &name)
{
	struct addrinfo hints = {};
	struct addrinfo *servInfo;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (_apiSocket != -1)
		close(_apiSocket);
	_apiSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_apiSocket < 0)
		throw BotFailureException("Socket Failure");
	getaddrinfo(name.c_str(), "80", &hints, &servInfo);
	connect(_apiSocket, servInfo->ai_addr, servInfo->ai_addrlen);
	freeaddrinfo(servInfo);
}

void	Bot::parseApiContent(std::string const &host, std::string const &content) const
{
	if (host.find("numbersapi") != std::string::npos)
		sendMsgChan(content.substr(content.find_last_of('\n', content.size() - 2) + 1));
	else if (host.find("notify") != std::string ::npos)
	{
		if (content.find("success") == std::string::npos)
			sendMsgChan("Unable to find ISS position !");
		else
		{
			std::string msg = "ISS current position: Latitude: ";
			std::size_t i = content.find("latitude") + 12;
			std::size_t j = content.find('\"', i + 1);
			msg.append(content.substr(i, j - i));
			msg.append(", Longitude: ");
			i = content.find("longitude", i) + 13;
			j = content.find('\"', i + 1);
			msg.append(content.substr(i, j - i));
			sendMsgChan(msg);
		}
	}
}

void	Bot::apiHandling(std::string const &host, std::string const &req)
{
	resetApiSocket(host);
	std::string const request = getReqString(host, req);
	if (send(_apiSocket, request.c_str(), request.length(), MSG_NOSIGNAL) < 0)
		throw BotFailureException("send Failure");
	char buff[1024] = {};
	std::string content = "";
	std::size_t bytes = 0;
	while ((bytes = recv(_apiSocket, buff, 1024, MSG_NOSIGNAL)) > 0)
		content.append(buff);
	if (bytes < 0)
		throw BotFailureException("recv Failure");
	parseApiContent(host, content);
}

void		Bot::rickRollHandling(void) const
{
	sendMsgChan("🕺 Never gonna give you up 🕺");
	sleep(1);
	sendMsgChan("🕺 Never gonna let you down 🕺");
	sleep(1);
	sendMsgChan("🕺 Never gonna run around and desert you 🕺");
	sleep(1);
	sendMsgChan("🕺 Never gonna make you cry 🕺");
	sleep(1);
	sendMsgChan("🕺 Never gonna say goodbye 🕺");
	sleep(1);
	sendMsgChan("🕺 Never gonna tell a lie and hurt you 🕺");
}

void	Bot::helpHandling(void) const
{
	sendMsgChan("BeBot Commands\r\n?math -> Get random fact about Maths, ?iss -> Get current postion of ISS, ?rick -> Surpise !");
}

void	Bot::auth(void)
{
	std::string authMsg = "PASS " + _password + "\r\n"
						  "NICK " + _botName + "\r\n"
						  "USER " + _botName + " " + _botName + " localhost :" + _botName + "\r\n";
	sendMsg(_clientSocket, authMsg);
	_buffer = recvMsg(_clientSocket);
	while (_buffer.find("433") != std::string::npos)
	{
		_botName += "_";
		sendMsg(_clientSocket, "NICK " + _botName + "\r\n");
		_buffer = recvMsg(_clientSocket);
	}
	_isConnected = true;
}

void	Bot::run(void)
{
	while (connect(_clientSocket, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		sleep(1);
	auth();
	while (_buffer.find("376") == std::string::npos)
		_buffer = recvMsg(_clientSocket);

	std::string message = "JOIN #bot\r\n"; //TODO COLISION CHAN HANDLING
	_chanName = "#bot";

	sendMsg(_clientSocket, message);

	while (1)
	{
		_buffer = recvMsg(_clientSocket);
		std::string cmd = _buffer.substr(_buffer.find(":?") + 2);
		// while (std::isspace(cmd.back())) // TO DO FAIRE CLEAING EN CAS DE WHITE SPACE
		// 	cmd.pop_back();
		std::cout << cmd << '\n';
		if (cmd == "help")
			helpHandling();
		if (cmd == "math")
			apiHandling("numbersapi.com", "/random/math");
		if (cmd == "iss")
			apiHandling("api.open-notify.org", "/iss-now.json");
		if (cmd == "rick")
			rickRollHandling();

		if (cmd == "stop")
			break;
	}
}

int main()
{
	Bot bot(6667, "localhost", "1234");
	bot.run();
	return 0;
}