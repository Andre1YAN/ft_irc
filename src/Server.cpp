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
	}
	return *this;
}

//---------------//Getters
int Server::getPort(){return this->port;}
int Server::getFd(){return this->server_fdsocket;}
std::string Server::getPassword(){return this->password;}
Client *Server::getClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].getFd() == fd)
			return &this->clients[i];
	}
	return NULL;
}
Client *Server::getClientNick(std::string nickname){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].getNickName() == nickname)
			return &this->clients[i];
	}
	return NULL;
}

Channel *Server::getChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			return &channels[i];
	}
	return NULL;
}
//---------------//Getters

//---------------//Setters
void Server::setFd(int fd){this->server_fdsocket = fd;}
void Server::setPort(int port){this->port = port;}
void Server::setPassword(std::string password){this->password = password;}
void Server::addClient(Client newClient){this->clients.push_back(newClient);}
void Server::addChannel(Channel newChannel){this->channels.push_back(newChannel);}
void Server::addFds(pollfd newFd){this->fds.push_back(newFd);}
//---------------//Setters

//---------------//Remove Methods
void Server::removeClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].getFd() == fd)
			{this->clients.erase(this->clients.begin() + i); return;}
	}
}
void Server::removeChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			{this->channels.erase(this->channels.begin() + i); return;}
	}
}
void Server::removeFds(int fd){
	for (size_t i = 0; i < this->fds.size(); i++){
		if (this->fds[i].fd == fd)
			{this->fds.erase(this->fds.begin() + i); return;}
	}
}
void	Server::removeChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;
		if (channels[i].getClient(fd))
			{channels[i].removeClient(fd); flag = 1;}
		else if (channels[i].getAdmin(fd))
			{channels[i].removeAdmin(fd); flag = 1;}
		if (channels[i].GetClientsNumber() == 0)
			{channels.erase(channels.begin() + i); i--; continue;}
		if (flag){
			std::string rpl = ":" + getClient(fd)->getNickName() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendToAll(rpl);
		}
	}
}
//---------------//Remove Methods

//---------------//Send Methods
void Server::sendError(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::sendErrorWithChannel(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}
//---------------//Send Methods

//---------------//Close and Signal Methods
bool Server::Signal = false;
void Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

void	Server::closeFds(){
	for(size_t i = 0; i < clients.size(); i++){
		std::cout << RED << "Client <" << clients[i].getFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].getFd());
	}
	if (server_fdsocket != -1){	
		std::cout << RED << "Server <" << server_fdsocket << "> Disconnected" << WHI << std::endl;
		close(server_fdsocket);
	}
}
//---------------//Close and Signal Methods

//---------------//Server Methods
void Server::init(int port, std::string pass)
{
	this->password = pass;
	this->port = port;
	this->setSeverSocket();

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
					this->acceptNewClient();
				else
					this->reciveNewData(fds[i].fd);
			}
		}
	}
	closeFds();
}

void Server::setSeverSocket()
{
	int en = 1;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = INADDR_ANY;
	add.sin_port = htons(port);
	server_fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fdsocket == -1)
		throw(std::runtime_error("faild to create socket"));
	if(setsockopt(server_fdsocket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	 if (fcntl(server_fdsocket, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(server_fdsocket, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(server_fdsocket, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));
	new_cli.fd = server_fdsocket;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	fds.push_back(new_cli);
}

void Server::acceptNewClient()
{
	Client cli;
	memset(&cliadd, 0, sizeof(cliadd));
	socklen_t len = sizeof(cliadd);
	int incofd = accept(server_fdsocket, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
		{std::cout << "fcntl() failed" << std::endl; return;}
	new_cli.fd = incofd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	cli.setFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients.push_back(cli);
	fds.push_back(new_cli);
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::reciveNewData(int fd)
{
	std::vector<std::string> cmd;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *cli = getClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		removeChannels(fd);
		removeClient(fd);
		removeFds(fd);
		close(fd);
	}
	else
	{ 
		cli->setBuffer(buff);
		if(cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		cmd = splitRecivedBuffer(cli->getBuffer());
		for(size_t i = 0; i < cmd.size(); i++)
			this->parseExecCmd(cmd[i], fd);
		if(getClient(fd))
			getClient(fd)->clearBuffer();
	}
}
//---------------//Server Methods

//---------------//Parsing Methods
std::vector<std::string> Server::splitRecivedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}

std::vector<std::string> Server::splitCmd(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;

	while (stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

bool Server::notRegistered(int fd)
{
	if (!getClient(fd) || getClient(fd)->getNickName().empty() || getClient(fd)->getUserName().empty() || getClient(fd)->getNickName() == "*"  || !getClient(fd)->getLogedIn())
		return false;
	return true;
}

void Server::parseExecCmd(std::string &cmd, int fd)
{
	if(cmd.empty())
			return ;
	std::vector<std::string> splited_cmd = splitCmd(cmd);
	size_t found = cmd.find_first_not_of(" \t\v");
	if(found != std::string::npos)
			cmd = cmd.substr(found);
	if(splited_cmd.size() && (splited_cmd[0] == "BONG" || splited_cmd[0] == "bong"))
			return ;
	if(splited_cmd.size() && (splited_cmd[0] == "PASS" || splited_cmd[0] == "pass"))
			clientAuthen(fd, cmd);
		else if (splited_cmd.size() && (splited_cmd[0] == "NICK" || splited_cmd[0] == "nick"))
			set_nickname(cmd,fd);
		else if(splited_cmd.size() && (splited_cmd[0] == "USER" || splited_cmd[0] == "user"))
			set_username(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "QUIT" || splited_cmd[0] == "quit"))
			QUIT(cmd,fd);
		else if(notRegistered(fd))
		{
			if (splited_cmd.size() && (splited_cmd[0] == "KICK" || splited_cmd[0] == "kick"))
				//KICK(cmd, fd);
				return ;
			else if (splited_cmd.size() && (splited_cmd[0] == "JOIN" || splited_cmd[0] == "join"))
				JOIN(cmd, fd);
	//		else if (splited_cmd.size() && (splited_cmd[0] == "TOPIC" || splited_cmd[0] == "topic"))
	//			Topic(cmd, fd);
			//else if (splited_cmd.size() && (splited_cmd[0] == "MODE" || splited_cmd[0] == "mode"))
	//			mode_command(cmd, fd);
		//	else if (splited_cmd.size() && (splited_cmd[0] == "PART" || splited_cmd[0] == "part"))
		//		PART(cmd, fd);
		//	else if (splited_cmd.size() && (splited_cmd[0] == "PRIVMSG" || splited_cmd[0] == "privmsg"))
		//		PRIVMSG(cmd, fd);
		//	else if (splited_cmd.size() && (splited_cmd[0] == "INVITE" || splited_cmd[0] == "invite"))
		//		Invite(cmd,fd);
		//	else if (splited_cmd.size())
		//		_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
		}
		else if (!notRegistered(fd))
			_sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}
//---------------//Parsing Methods