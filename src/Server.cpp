#include "../inc/Server.hpp"

Server::Server(){this->server_fdsocket = -1;}
Server::~Server(){}
Server::Server(Server const &src){*this = src;}
Server &Server::operator=(Server const &src){
	if (this != &src){
		/*
		struct sockaddr_in add;
		struct sockaddr_in cliadd;
		struct pollfd new_cli;
		*/
		this->port = src.port;
		this->server_fdsocket = src.server_fdsocket;
		this->password = src.password;
		this->clients = src.clients;
		this->channels = src.channels;
		this->fds = src.fds;
		this->isBotfull = src.isBotfull;
	}
	return *this;
}

bool Server::Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

void Server::init(int port, std::string pass)
{
	this->password = pass;
	this->port = port;
	this->set_sever_socket();

	std::cout << GRE << "Server <" << server_fdsocket << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == server_fdsocket)
					this->accept_new_client();
				else
					this->reciveNewData(fds[i].fd);
			}
		}
	}
	close_fds();
}


