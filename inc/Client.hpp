#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "Channel.hpp"

class Client
{
private:
	int fd;
	bool isOperator;
	bool registered;
	std::string nickname;
	bool logedin;
	std::string username;
	std::string buffer;
	std::string ipadd;
	std::vector<std::string> ChannelsInvite;
public:
	Client();
	Client(std::string nickname, std::string username, int fd);
	~Client();
	Client(Client const &src);
	Client &operator=(Client const &src);
	//---------------//Getters
	int getFd();
	bool getRegistered();
	bool getInviteChannel(std::string &ChName);
	std::string getNickName();
	bool 		getLogedIn();
	std::string getUserName();
	std::string getIpAdd();
	std::string getBuffer();
	std::string getHostname();
	//---------------//Setters
	void setFd(int fd);
	void setNickname(std::string& nickName);
	void setLogedin(bool value);
	void setUsername(std::string& username);
	void setBuffer(std::string recived);
	void setRegistered(bool value);
	void setIpAdd(std::string ipadd);
	//---------------//Methods
	void clearBuffer();
	void addChannelInvite(std::string &chname);
	void removeChannelInvite(std::string &chname);
};

#endif