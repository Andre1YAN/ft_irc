#include "../inc/Server.hpp"

void FindKReason(std::string cmd, std::string tofind, std::string &str)
{
    size_t i = 0;
	for (; i < cmd.size(); i++)
    {
		if (cmd[i] != ' ')
        {
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind) break;
			else tmp.clear();
		}
	}
	if (i < cmd.size()) str = cmd.substr(i);
	    i = 0;
	for (; i < str.size() && str[i] == ' '; i++)
	    str = str.substr(i);
}

std::string SplitCmdK(std::string &cmd, std::vector<std::string> &tmp)
{
    std::stringstream ss(cmd);
	std::string str, reason;

	int count = 3;
	while (ss >> str && count--)
		tmp.push_back(str);
	if(tmp.size() != 3) 
        return std::string("");
	FindKReason(cmd, tmp[1], reason);
	return reason;
}

std::string Server::splitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd)
{
    std::string reason = SplitCmdK(cmd, tmp);
	if(tmp.size() < 3) 
    {
        tmp.clear(); 
        return 0;
    }
	tmp.erase(tmp.begin());
	std::string str = tmp[0]; 
    std::string str1; 
    user = tmp[1];
    tmp.clear();
	for (size_t i = 0; i < str.size(); i++){//split the first string by ',' to get the channels names
		if (str[i] == ',')
		{	
			tmp.push_back(str1); 
			str1.clear();
		}
		else 
			str1 += str[i];
	}
	tmp.push_back(str1);
	for (size_t i = 0; i < tmp.size(); i++)//erase the empty strings
	{
        if (tmp[i].empty())
            tmp.erase(tmp.begin() + i--);
    }
	if (reason[0] == ':')
        reason.erase(reason.begin());
	else //shrink to the first space
	{
        for (size_t i = 0; i < reason.size(); i++)
        {
            if (reason[i] == ' ')
            {
                reason = reason.substr(0, i);
                break;
            }
        }
    }
	for (size_t i = 0; i < tmp.size(); i++){// erase the '#' from the channel name and check if the channel valid
			if (!tmp[i].empty() && *(tmp[i].begin()) == '#')
				tmp[i].erase(tmp[i].begin());
			else
				{
                    sendErrorWithChannel(403, getClient(fd)->getNickName(), tmp[i], getClient(fd)->getFd(), " :No such channel\r\n"); 
                    tmp.erase(tmp.begin() + i--);
                }
		}
	return reason;
}

void Server::KICK(std::string cmd, int fd)
{
    std::vector<std::string> tmp;
    std::string reason, user;
    reason = splitCmdKick(cmd, tmp, user, fd);
    if (user.empty())
    {
        sendError(461, getClient(fd)->getNickName(), getClient(fd)->getFd(), " :Not enough parameters\r\n");
        return ;
    }
    for (size_t i = 0; i < tmp.size(); i++)
    {
        if (getChannel(tmp[i])) // check if channel exist
        {
            Channel* ch = getChannel(tmp[i]);
            if (!ch->getClient(fd) && !ch->getAdmin(fd))
            {
                sendErrorWithChannel(442, getClient(fd)->getNickName(), "#" + tmp[i], getClient(fd)->getFd(), " :You're not on that channel\r\n");
                return ;
            }
            if (ch->getAdmin(fd)) // check if client is admin
            {
                Client* c = ch->GetClientInChannel(user);
                if (c) // check if client in channel
                {
                    std::stringstream ss;
                    ss << ":" << getClient(fd)->getNickName() << "!~" << getClient(fd)->getUserName() << "@" << "localhost" << " KICK #" << tmp[i] << " " << user;
                    if (!reason.empty())
                        ss << " :" << reason << "\r\n";
                    else
                        ss << "\r\n";
                    ch->sendToAll(ss.str());
                    if (ch->getAdmin(c->getFd()))
                        ch->removeAdmin(c->getFd());
                    else   
                        ch->removeClient(c->getFd());
                    if (ch->GetClientsNumber() == 0)
                        channels.erase(channels.begin() + i);
                }
                else // if client not in channel
                {
                    sendErrorWithChannel(441, getClient(fd)->getNickName(), "#" + tmp[i], getClient(fd)->getFd(), " :They aren't on that channel\r\n");
                    continue ;
                }
            }
            else // if client is not admin
            {
                sendErrorWithChannel(482, getClient(fd)->getNickName(), "#" + tmp[i], getClient(fd)->getFd(), " :You're not channel operator\r\n");
                continue ;
            }
        }
        else // if channel doesn't exist
            sendErrorWithChannel(403, getClient(fd)->getNickName(), "#" + tmp[i], getClient(fd)->getFd(), " :No such channel\r\n");
    }
}