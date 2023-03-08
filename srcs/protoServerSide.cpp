#include "irc.hpp"
#include "const.hpp"

struct server_t
{
	int fd;
 	struct sockaddr_in address; 
};

template <typename T>
std::string NumberToString(T pNumber)
{
	std::ostringstream oOStrStream;
	oOStrStream << pNumber;
	return oOStrStream.str();
}

int main(void)
{
	server_t server;

    // Create server socket
	server.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.fd == -1)
	{
        std::cerr << "Failed to bind server socket to port.\n";
        return 1;	
	}

    // Set server socket to non-blocking mode
    fcntl(server.fd, F_SETFL, O_NONBLOCK);

    // Bind server socket to port
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(PORT);
    if (bind(server.fd, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
	{
        std::cerr << "Failed to bind server socket to port.\n";
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server.fd, MAX_CLIENT) < 0) {
        std::cerr << "Failed to start listening for incoming connections.\n";
        return 1;
    }

	std::vector<struct pollfd> pollingList; // socket to poll

    // Add server socket to the list of sockets to poll
	pollingList.push_back(pollfd());
    pollingList[0].fd = server.fd;
    pollingList[0].events = POLLIN;
	bool isOn = true;
	while (isOn)
	{
		// Poll all sockets for events
		if (poll(&pollingList[0], pollingList.size() + 1, -1) < 0)
		{
			std::cerr << "Polling failed.\n";
			break;
		}
		// New connection incoming 
		if (pollingList[0].revents == POLLIN)
		{
			struct sockaddr_in	clientAdress;
			socklen_t			clientSize = sizeof(clientAdress);

			// Accept new connection
			int clientFd = accept(server.fd, (struct sockaddr *)&clientAdress, &clientSize);
			if (clientFd == -1)
			{
				std::cerr << "Failed to accept incoming connection.\n";
				continue;
			}

			// Set client fd to non-blocking mode
			if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
			{
				std::cerr << "Failed to accept incoming connection.\n";
				continue;
			}

			//Get Hostname client
			char hostname[NI_MAXHOST];
			int i = getnameinfo((struct sockaddr *)&clientAdress, clientSize, 
								hostname, NI_MAXHOST, NULL, 0,  NI_NUMERICSERV);
			std::cout << i << '\n';
			struct pollfd newFD;
			newFD.events = POLLIN;
			newFD.fd = clientFd;
			pollingList.push_back(newFD);


			std::cout  << "new connection from " << hostname << " on socket " << clientFd << '\n';

			//Send RPL 1 for IRSSI
			std::string content = ":localhost 001 bastien Welcome\r\n";
			send(clientFd, content.c_str(), content.length(), 0);
		}
		// Handle packets
		else
		{
			for (size_t i = 1; i < pollingList.size(); i++) 
			{
				// If data to read
				if (pollingList[i].revents & POLLIN)
				{
					char buffer[1024] = {0};
					int bytes_read = recv(pollingList[i].fd, &buffer, sizeof(buffer), 0);
					if (bytes_read == -1)
						std::cerr << "recv() failed" << std::endl;
					else if (bytes_read == 0)
					{
						// Client has disconnected
						std::cout << "Client " << i << " disconected\n";
						int fd = pollingList[i].fd;
						pollingList.erase(pollingList.begin() + i);
						close(fd);
					}
					else
					{
						// Echo received data back to client
						//Remove '\n' at the end of buffer
						std::string msg(buffer);
						msg.erase(--msg.end());

						std::cout << "client N" << i << " sent: " << msg << '\n';

						// Send message content to other client

						// for (int j = 1; j < fds.size(); j++) 
						// {
						// 	if (i == j)
						// 		continue;
						// 	std::string content = "Client N";
						// 	content.append(NumberToString(i));
						// 	content.append(" :");
						// 	content.append(buffer);
						// 	int s = send(fds[j].fd, content.c_str(), content.length(), MSG_DONTWAIT);
						// }
					}
				}
			}
		}
	}
	return 0;
}