#include "../inc/Server.hpp"


void Server::INVITE(std::string &cmd, int &fd)
{
	std::vector<std::string> scmd = splitCmd(cmd);
	if(scmd.size() < 3)// ERR_NEEDMOREPARAMS (461) if there are not enough parameters
		{sendError(461, getClient(fd)->getNickName(), fd, " :Not enough parameters\r\n"); return;}
	std::string channelname = scmd[2].substr(1);
	if(scmd[2][0] != '#' || !getChannel(channelname))// ERR_NOSUCHCHANNEL (403) if the given channel does not exist
	    {sendError(403, channelname, fd, " :No such channel\r\n"); return;}
	if (!(getChannel(channelname)->getClient(fd)) && !(getChannel(channelname)->getAdmin(fd)))// ERR_NOTONCHANNEL (442) if the client is not on the channel
	    {sendError(442, channelname, fd, " :You're not on that channel\r\n"); return;}
	if (getChannel(channelname)->GetClientInChannel(scmd[1]))// ERR_USERONCHANNEL (443) if the given nickname is already on the channel
	    {sendErrorWithChannel(443, getClient(fd)->getNickName(), channelname, fd, " :is already on channel\r\n"); return;}
	Client *clt = getClientNick(scmd[1]);
	if (!clt)// ERR_NOSUCHNICK (401) if the given nickname is not found
		{sendError(401, scmd[1], fd, " :No such nick\r\n");return;}
	if (getChannel(channelname)->GetInvitOnly() && !getChannel(channelname)->getAdmin(fd))// ERR_INVITEONLYCHAN (473) if the channel is invite-only
		{sendErrorWithChannel(482,getChannel(channelname)->getClient(fd)->getNickName(),scmd[1],fd," :You're not channel operator\r\n"); return;}
	if (getChannel(channelname)->GetLimit() && getChannel(channelname)->GetClientsNumber() >= getChannel(channelname)->GetLimit()) // ERR_CHANNELISFULL (471) if the channel is full
		{sendErrorWithChannel(473,getChannel(channelname)->getClient(fd)->getNickName(),channelname,fd," :Cannot invit to channel (+i)\r\n"); return;}
	// RPL_INVITING (341) if the invite was successfully sent
	clt->addChannelInvite(channelname);
	std::string rep1 = ": 341 "+ getClient(fd)->getNickName()+" "+ clt->getNickName()+" "+ scmd[2]+"\r\n";
	_sendResponse(rep1, fd);
	std::string rep2 = ":"+ clt->getHostname() + " INVITE " + clt->getNickName() + " " + scmd[2]+"\r\n";
	_sendResponse(rep2, clt->getFd());
	
}