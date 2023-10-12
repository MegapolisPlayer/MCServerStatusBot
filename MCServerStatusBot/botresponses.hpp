#ifndef MCSSB_BOTRESPONSE
#define MCSSB_BOTRESPONSE

#include "network.hpp"

struct bottype {
	static dpp::cluster bothandle;
	
	dpp::snowflake channelid = 0;
	bool autoenabled = false;
	
	static std::thread botthread;
};

extern std::unordered_map<dpp::snowflake, bottype> bots;

void sendautomsg(dpp::snowflake aguildid);

namespace botresponse {
	void serverinfo(const dpp::slashcommand_t& aevent);
	void status(const dpp::slashcommand_t& aevent);
	void players(const dpp::slashcommand_t& aevent);
	void help(const dpp::slashcommand_t& aevent);
	void credits(const dpp::slashcommand_t& aevent);
	
	void home(const dpp::slashcommand_t& aevent);
	void change(const dpp::slashcommand_t& aevent);
	void enableautoupdate(const dpp::slashcommand_t& aevent);
	void disableautoupdate(const dpp::slashcommand_t& aevent);
}

void initbot();

#endif