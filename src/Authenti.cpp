#include "../inc/Server.hpp"

//---------------//PASS COMMAND
void Server::clientAuthen(int fd, std::string cmd)
{
    Client *cli = getClient(fd);
    cmd = cmd.substr(4);
    size_t pos = cmd.find_first_not_of("\t\v ");
    if(pos < cmd.size())
    {
            cmd = cmd.substr(pos);
            if (cmd[0] == ':')
                    cmd.erase(cmd.begin());
    }
    if(pos == std::string::npos || cmd.empty())
            _sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd);
    else if(!cli->getRegistered())
    {
        std::string pass = cmd;
        if(pass == password)
                cli->setRegistered(true);
        else
                _sendResponse(ERR_INCORPASS(std::string("*")), fd);
    }
    else
            _sendResponse(ERR_ALREADYREGISTERED(getClient(fd)->getNickName()), fd);
}
//---------------//PASS COMMAND

//---------------//NICK COMMAND
bool Server::isValidNickname(std::string& nickname)
{
		
	if(!nickname.empty() && (nickname[0] == '&' || nickname[0] == '#' || nickname[0] == ':'))
		return false;
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if(!std::isalnum(nickname[i]) && nickname[i] != '_')
			return false;
	}
	return true;
}

bool Server::nickNameInUse(std::string& nickname)
{
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].getNickName() == nickname)
			return true;
	}
	return false;
}

void Server::set_nickname(std::string cmd, int fd)
{
        std::string inuse;
        cmd = cmd.substr(4);
        size_t pos = cmd.find_first_not_of("\t\r ");
        if (pos < cmd.size())
        {
                cmd = cmd.substr(pos);
                if (cmd[0] == ':')
                        cmd.erase(cmd.begin());
        }
        Client *cli = getClient(fd);
        if (pos == std::string::npos || cmd.empty())
        {
                _sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd);
                return ;
        }
        if (nickNameInUse(cmd) && cli->getNickName() != cmd)
        {
                inuse = "*";
                if (cli->getNickName().empty())
                        cli->setNickname(inuse);
                _sendResponse(ERR_NICKINUSE(std::string(cmd)), fd);
                return ;
        }
        if (!isValidNickname(cmd))
        {
                _sendResponse(ERR_ERRONEUSNICK(std::string(cmd)), fd);
                return ;
        }
        else
        {
                if (cli && cli->getRegistered())
                {
                        std::string prevnick = cli->getNickName();
                        cli->setNickname(cmd);
                        for (size_t i = 0; i < channels.size(); i++)
                        {
                                Client *cl = channels[i].GetClientInChannel(prevnick);
                                if (cl)
                                        cl->setNickname(cmd);
                        }
                        if (!prevnick.empty() && prevnick != cmd)
                        {
                                if (prevnick == "*" && !cli->getUserName().empty())
                                {
                                        cli->setLogedin(true);
                                        _sendResponse(RPL_CONNECTED(cli->getNickName()), fd);
                                        _sendResponse(RPL_NICKCHANGE(cli->getNickName(), cmd), fd);
                                }
                                else
                                        _sendResponse(RPL_NICKCHANGE(prevnick, cmd), fd);
                                return ;
                        }
                }
                else if (cli && !cli->getRegistered())
                        _sendResponse(ERR_NOTREGISTERED(cmd), fd);
        }
        if (cli && cli->getRegistered() && !cli->getUserName().empty() && !cli->getNickName().empty() && cli->getNickName() != "*" && !cli->getLogedIn())
        {
                cli->setLogedin(true);
                _sendResponse(RPL_CONNECTED(cli->getNickName()), fd);
        }
}
//---------------//NICK COMMAND

//---------------//USER COMMAND
void Server::set_username(std::string& cmd, int fd)
{
        std::vector<std::string> splited_cmd = splitCmd(cmd);

        Client* cli = getClient(fd);
        if (cli && splited_cmd.size() < 5)
        {
                _sendResponse(ERR_NOTENOUGHPARAM(cli->getNickName()), fd);
                return ;
        }
        if (!cli || !cli->getRegistered())
        {
                _sendResponse(ERR_NOTREGISTERED(std::string("*")), fd);
                return ;
        }
        else if (cli && !cli->getUserName().empty())
        {
                _sendResponse(ERR_ALREADYREGISTERED(cli->getNickName()), fd);
                return ;
        }
        else
                cli->setUsername(splited_cmd[1]);
        if (cli && cli->getRegistered() && !cli->getUserName().empty() && !cli->getNickName().empty() && cli->getNickName() != "*" && !cli->getLogedIn())
        {
                cli->setLogedin(true);
                _sendResponse(RPL_CONNECTED(cli->getNickName()), fd);
        }
}
//---------------//USER COMMAND
