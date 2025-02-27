#include "../inc/Server.hpp"

void FindQ(std::string cmd, std::string tofind, std::string &str)
{
    size_t i = 0;
	for (; i < cmd.size(); i++)
    {
		if (cmd[i] != ' ')
        {
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind)
				break;
			else tmp.clear();
		}
	}
	if (i < cmd.size())
		str = cmd.substr(i);
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

std::string SplitQuit(std::string cmd)
{
    std::istringstream stm(cmd);
    std::string reason, str;
    stm >> str;
    FindQ(cmd, str, reason);
    if (reason.empty())
        return std::string("Quit");
    if (reason[0] != ':')
    {
        for (size_t i = 0; i < reason.size(); i++)
        {
            if (reason[i] == ' ')
            {
                reason.erase(reason.begin() + i, reason.end());
                break;
            }
        }
        reason.insert(reason.begin(), ':');
    }
    return reason;
}

void Server::QUIT(std::string cmd, int fd)
{
    std::string reason;
    reason = SplitQuit(cmd);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].getClient(fd))
        {
            channels[i].removeClient(fd);
            if (channels[i].GetClientsNumber() == 0)
                channels.erase(channels.begin() + i);
            else
            {
                std::string rpl = ":" + getClient(fd)->getNickName() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT " + reason + "\r\n";
                channels[i].sendToAll(rpl);
            }
        }
        else if (channels[i].getAdmin(fd)){
			channels[i].removeAdmin(fd);
			if (channels[i].GetClientsNumber() == 0)
				channels.erase(channels.begin() + i);
			else{
				std::string rpl = ":" + getClient(fd)->getNickName() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT " + reason + "\r\n";
				channels[i].sendToAll(rpl);
			}
		}
    }
    std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
    removeChannels(fd);
    removeClient(fd);
    removeFds(fd);
    close(fd);
}