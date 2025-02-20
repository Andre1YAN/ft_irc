#include "../inc/Server.hpp"

//---------------//PASS COMMAND
void Server::client_authen(int fd, std::string cmd)
{
    Client *cli = GetClient(fd);
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
            _sendResponse(ERR_ALREADYREGISTERED(GetClient(fd)->GetNickName()), fd);
}

