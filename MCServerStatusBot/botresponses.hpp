#ifndef MCSSB_BOTRESPONSE
#define MCSSB_BOTRESPONSE

#include "network.hpp"

struct bottype {
	dpp::cluster bothandle = dpp::cluster(programinfo.bottoken);
	dpp::snowflake channelid = 0;
	bool autoenabled = false;
	
	std::thread botthread;
};

extern bottype bot;

void sendautomsg();

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