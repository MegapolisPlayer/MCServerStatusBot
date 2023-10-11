#ifndef MCSSB_NETWORK
#define MCSSB_NETWORK

#include "include.hpp"

struct programinfotype {
	std::string serverip;
	std::string serverport;
	
	std::string webrequest;
	
	json oldserver;
	json serverdata;
	
	static bool mainloop;
};

extern std::unordered_map<dpp::snowflake, programinfotype> programinfos;

void DNSresolve(asio::ip::tcp::endpoint* aendpoint, asio::io_context& acontext);

void connect(asio::ssl::stream<asio::ip::tcp::socket>& asocket, asio::ip::tcp::endpoint& aendpoint);

std::string makerequest(dpp::snowflake aguildid);

extern std::vector<char> readbuf;
extern std::vector<char> tempbuf;
extern std::atomic_bool isdone;
extern bool hasreadhttpheader;
extern const char* httpsep;

void readnetdata(asio::ssl::stream<asio::ip::tcp::socket>& asocket) ;

void resetreadnetdata();

#endif