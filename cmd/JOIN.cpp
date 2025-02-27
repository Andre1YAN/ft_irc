#include "../inc/Server.hpp"

void	Server::JOIN(std::string cmd, int fd)
{
    std::vector<std::pair<std::string, std::string> > token;

    if (!splitJoin(token, cmd, fd))
    {
        sendError(461, getClient(fd)->getNickName(), getClient(fd)->getFd(), " :Not enough parameters\r\n");
        return ;
    }
    for (size_t i = 0; i < token.size(); i++)
    {
        bool exist = false;
        for (size_t j = 0; j < this->channels.size(); j++)
        {
            if (this->channels[j].GetName() == token[i].first)
            {
                existCh(token, i, j, fd);
                exist = true;
                break;
            }
        }
        if (!exist)
            notExistCh(token, i, fd);
    }
}

int		Server::splitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd)
{
    std::istringstream iss(cmd);
    std::string command, channels_str, keys_str;

    iss >> command;
    if (!(iss >> channels_str))
    {
        token.clear();
        return 0;
    }
    iss >> keys_str;

    std::vector<std::string> channels;
    std::vector<std::string> keys;

    std::stringstream ss_ch(channels_str);
    std::string ch;
    size_t channel_count = 0;
    while (std::getline(ss_ch, ch, ',')) 
    {
        if (channel_count > 10)
        {
            sendError(407, getClient(fd)->getNickName(), getClient(fd)->getFd(), " :Too many channels\r\n");
            return 0;
        }
        if (!ch.empty() && ch[0] == '#') 
        {
            channels.push_back(ch.substr(1)); 
            channel_count++;
        }
        else
            sendErrorWithChannel(403, getClient(fd)->getNickName(), ch, getClient(fd)->getFd(), " :No such channel\r\n");

    }

    std::stringstream ss_key(keys_str);
    std::string key;
    while (std::getline(ss_key, key, ','))
        keys.push_back(key);

    for (size_t i = 0; i < channels.size(); i++)
    {
        std::string password;
        if (i < keys.size())
            password = keys[i];
        else
            password = "";
        token.push_back(std::make_pair(channels[i], password));
    }
    return 1;
}

void	Server::existCh(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd)
{
    Client* cli = getClient(fd);
    if (this->channels[j].GetClientInChannel(cli->getNickName()))
        return ;
    if (searchForClients(cli->getNickName()) >= 10)
    {
        sendError(405, cli->getNickName(), cli->getFd(), " :You have joined too many channels\r\n");
        return ;
    }
    if (!this->channels[j].GetPassword().empty() && this->channels[j].GetPassword() != token[i].second)
    {
        sendErrorWithChannel(475, cli->getNickName(), "#" + token[i].first, cli->getFd(), " :Cannot join channel (+k) - bad key\r\n"); 
        return;
    }
    if (this->channels[j].GetInvitOnly() && !cli->getInviteChannel(token[i].first))
    {
        sendErrorWithChannel(475, cli->getNickName(), "#" + token[i].first, cli->getFd(), " : :Cannot join channel (+i)\r\n");
        return ;
    }
    if (this->channels[j].GetLimit() && this->channels[j].GetClientsNumber() >= this->channels[j].GetLimit())
    {
        sendErrorWithChannel(471, cli->getNickName(), "#" + token[i].first, cli->getFd(), " :Cannot join channel (+l)\r\n");
        return ;
    }

    this->channels[j].addClient(*cli);
    if(channels[j].GetTopicName().empty())
		_sendResponse(RPL_JOINMSG(cli->getHostname(),cli->getIpAdd(),token[i].first) + \
			RPL_NAMREPLY(cli->getNickName(),channels[j].GetName(),channels[j].clientChannelList()) + \
			RPL_ENDOFNAMES(cli->getNickName(),channels[j].GetName()), fd);
	else
		_sendResponse(RPL_JOINMSG(cli->getHostname(),cli->getIpAdd(),token[i].first) + \
			RPL_TOPICIS(cli->getNickName(),channels[j].GetName(),channels[j].GetTopicName()) + \
			RPL_NAMREPLY(cli->getNickName(),channels[j].GetName(),channels[j].clientChannelList()) + \
			RPL_ENDOFNAMES(cli->getNickName(),channels[j].GetName()),fd);
    channels[j].sendToAllExceptMe(RPL_JOINMSG(cli->getHostname(),cli->getIpAdd(),token[i].first), fd);
}

void	Server::notExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int fd)
{
    Client* cli = getClient(fd);
    if (searchForClients(cli->getNickName()) >= 10)
    {
        sendError(405, cli->getNickName(), cli->getFd(), " :You have joined too many channels\r\n");
        return ;
    }
    Channel newChannel;
    newChannel.SetName(token[i].first);
    newChannel.addAdmin(*cli);
    newChannel.setCreateiontime();
    this->channels.push_back(newChannel);
    _sendResponse(RPL_JOINMSG(cli->getHostname(),cli->getIpAdd(),newChannel.GetName()) + \
        RPL_NAMREPLY(cli->getNickName(),newChannel.GetName(),newChannel.clientChannelList()) + \
        RPL_ENDOFNAMES(cli->getNickName(),newChannel.GetName()),fd);
}

int		Server::searchForClients(std::string nickname)
{
    int count = 0;
    for (size_t i = 0; i < this->channels.size(); i++)
    {
        if (this->channels[i].GetClientInChannel(nickname))
            count++;
    }
    return count;
}