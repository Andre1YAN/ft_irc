#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"

class Client;
class Channel
{
private:

	int invit_only;
	int topic;
	int key;
	int limit;
	bool topic_restriction;
	std::string name;
	std::string time_creation;
	std::string password;
	std::string created_at;
	std::string topic_name;
	std::vector<Client> clients;
	std::vector<Client> admins;
	std::vector<std::pair<char, bool> > modes;
public:
	Channel();
	~Channel();
	Channel(Channel const &src);
	Channel &operator=(Channel const &src);
	//---------------//Setters
	void SetInvitOnly(int invit_only);
	void SetTopic(int topic);
	void SetKey(int key);
	void SetLimit(int limit);
	void SetTopicName(std::string topic_name);
	void SetPassword(std::string password);
	void SetName(std::string name);
	void SetTime(std::string time);
	void setTopicRestriction(bool value);
	void setModeAtindex(size_t index, bool mode);
	void setCreateiontime();
	//---------------//Getters
	int GetInvitOnly();
	int GetTopic();
	int GetKey();
	int GetLimit();
	int GetClientsNumber();
	bool Gettopic_restriction() const;
	bool getModeAtindex(size_t index);
	bool clientInChannel(std::string &nick);
	std::string GetTopicName();
	std::string GetPassword();
	std::string GetName();
	std::string GetTime();
	std::string getCreationTime();
	std::string getModes();
	std::string clientChannelList();
	Client *getClient(int fd);
	Client *getAdmin(int fd);
	Client *GetClientInChannel(std::string name);
	//---------------//Methods
	void addClient(Client newClient);
	void addAdmin(Client newClient);
	void removeClient(int fd);
	void removeAdmin(int fd);
	bool changeClientToAdmin(std::string& nick);
	bool changeAdminToClient(std::string& nick);
	//---------------//SendToAll
	void sendToAll(std::string rpl1);
	void sendToAllLeave(std::string rpl1, int fd);
};

#endif