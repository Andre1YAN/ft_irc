#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include "Client.hpp"
#include "Channel.hpp"
#include "replies.hpp"


#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Client;
class Channel;

class Server
{
private:
	int port;
	int server_fdsocket;
	static bool Signal;
	std::string password;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	std::vector<struct pollfd> fds;
	struct sockaddr_in add;
	struct sockaddr_in cliadd;
	struct pollfd new_cli;
public:
	Server();
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &src);
	//---------------//Getters
	int getFd();
	int getPort();
	std::string getPassword();
	Client *getClient(int fd);
	Client *getClientNick(std::string nickname);
	Channel *getChannel(std::string name);
	//---------------//Setters
	void setFd(int server_fdsocket);
	void setPort(int port);
	void setPassword(std::string password);
	void addClient(Client newClient);
	void addChannel(Channel newChannel);
	void addFds(pollfd newFd);
	void set_username(std::string& username, int fd);
	void set_nickname(std::string cmd, int fd);
	//---------------//Remove Methods
	void removeClient(int fd);
	void removeChannel(std::string name);
	void removeFds(int fd);
	void removeChannels(int fd);
	//---------------//Send Methods
	void sendError(int code, std::string clientname, int fd, std::string msg);
	void sendErrorWithChannel(int code, std::string clientname, std::string channelname, int fd, std::string msg);
	void 		_sendResponse(std::string response, int fd);
	//---------------//Close and Signal Methods
	static void signalHandler(int signum);
	void closeFds();
	//---------------//Server Methods
	void init(int port, std::string pass);
	void acceptNewClient();
	void setSeverSocket();
	void reciveNewData(int fd);
	//---------------//Parsing Methods
	std::vector<std::string> splitRecivedBuffer(std::string str);
	std::vector<std::string> splitCmd(std::string &str);
	void parseExecCmd(std::string &cmd, int fd);
	//---------------//Authentification Methods
	bool notRegistered(int fd);
	bool nickNameInUse(std::string& nickname);
	bool isValidNickname(std::string& nickname);
	void clientAuthen(int fd, std::string pass);
	//---------------------------//JOIN CMD
	void	JOIN(std::string cmd, int fd);
	int		splitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd);
	void	existCh(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
	void	notExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int fd);
	int		searchForClients(std::string nickname);
	//---------------------------//PART CMD
	void	PART(std::string cmd, int fd);
	int		splitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd);
	//---------------------------//KICK CMD
	void	KICK(std::string cmd, int fd);
	std::string splitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd);
	//---------------------------//PRIVMSG CMD
	void	PRIVMSG(std::string cmd, int fd);
	void	checkForChannels_Clients(std::vector<std::string> &tmp, int fd);
	//---------------------------//QUITE CMD
	void	QUIT(std::string cmd, int fd);
	//---------------------------//MODE CMD
	void 		modeCommand(std::string& cmd, int fd);
	std::string inviteOnly(Channel *channel, char opera, std::string chain);
	std::string topicRestriction(Channel *channel ,char opera, std::string chain);
	std::string passwordMode(std::vector<std::string> splited, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments);
	std::string operatorPrivilege(std::vector<std::string> splited, Channel *channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments);
	std::string channelLimit(std::vector<std::string> splited, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments);
	bool		isvalidLimit(std::string& limit);
	std::string modeToAppend(std::string chain, char opera, char mode);
	std::vector<std::string> splitParams(std::string params);
	void getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params);
	//---------------------------//TOPIC CMD
	std::string tTopic();
	void TOPIC(std::string &cmd, int &fd);
	void INVITE(std::string &cmd, int &fd);
	std::string getTopic(std::string& input);
	int getPos(std::string &cmd);
};

#endif