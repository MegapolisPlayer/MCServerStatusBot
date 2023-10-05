#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "json.hpp"
using namespace nlohmann;

#include "dpp/dpp.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ssl.hpp"

static struct {
	std::string serverip = "witch.hostify.cz";
	std::string serverport = "38170";
	std::string bottoken = "";
} programinfo;

void DNSresolve(asio::ip::tcp::endpoint* aendpoint, asio::io_context& acontext) {
	asio::error_code ec;
	
	asio::ip::tcp::resolver dnsresolver(acontext);
	
	asio::ip::tcp::resolver::iterator result = dnsresolver.resolve("mcapi.us", "443", {}, ec); //if HTTPS use 443, if HTTP use 80
	
	if(!ec) { std::cout << "DNS resolved for domain " << result->host_name() << "\n"; } 
	else { std::cout << "DNS resolver failure: " + ec.message() + "\n"; exit(-1); }
	
	(*aendpoint) = result->endpoint();
}

void connect(asio::ssl::stream<asio::ip::tcp::socket>& asocket, asio::ip::tcp::endpoint& aendpoint) {
	asio::error_code ec;
	
	asocket.lowest_layer().connect(aendpoint, ec);
	if(!ec) { std::cout << "Connection successful.\n"; }
	else { std::cout << "Connection failure: " + ec.message() + "\n"; exit(-1); }
	
	asocket.handshake(asio::ssl::stream_base::handshake_type::client, ec);
	if(!ec) { std::cout << "Handshake successful.\n"; }
	else { std::cout << "Handshake failure: " + ec.message() + "\n"; exit(-1); }
}

std::vector<char> readbuf;
std::vector<char> tempbuf(10000);
std::atomic_bool isdone(false);
bool hasreadhttpheader = false;

static const char* httpsep = "\r\n\r\n";

void readnetdata(asio::ssl::stream<asio::ip::tcp::socket>& asocket) {
	asocket.async_read_some(
		asio::buffer(tempbuf.data(), tempbuf.size()),
		[&](std::error_code ec, std::size_t len) {
			if(!ec) {
				if(!hasreadhttpheader) {
					auto it = std::search(tempbuf.begin(), tempbuf.end(), httpsep, httpsep + strlen(httpsep));
					if(it < tempbuf.end()) { //if separator found
						hasreadhttpheader = true;
						std::cout << "Added " << len - (it - tempbuf.begin()) - strlen(httpsep) << " bytes from API.\n";
						for(uint64_t i = (it - tempbuf.begin()) + strlen(httpsep); i < len; i++) {
							readbuf.push_back(tempbuf[i]);
						}
					}
					readnetdata(asocket);
					return;
				}
				
				std::cout << "Read " << len << " bytes from API.\n";
				readbuf.reserve(len);
				for(uint64_t i = 0; i < len; i++) {
					readbuf.push_back(tempbuf[i]);
				}
				readnetdata(asocket);
			}
			else {
				//sometimes servers are not ok and end stream after sending everything, we just dont treat it as an error
				if(ec == asio::error::eof || ec == asio::ssl::error::stream_truncated) { 
					std::cout << "Read finished.\n"; isdone = true; 
				}
				else { 
					std::cout << "Read so far:\n";
					for(uint64_t i = 0; i < readbuf.size(); i++) {
						std::cout << readbuf[i];
					}
					std::cout << "Error " << ec.value() << ": " << ec.message() << "\n"; 
					exit(-1);
				}
			}
		}
	);
}

void resetreadnetdata() {
	tempbuf.clear();
	tempbuf.resize(10000);
	isdone = false;
	hasreadhttpheader = false;
}

int main(int argc, char **argv) {
	std::cout << "NETWORK TESTING using ASIO\n(c) Martin, 2023\n";
	if(argc == 3) {
		programinfo.serverip = argv[1];
		programinfo.serverport = argv[2];
		//first arg doesnt count
		std::cout << 
			"ServerName: " << programinfo.serverip << "\n" <<
			"ServerPort: " << programinfo.serverport << "\n";
	}
	else {
		std::cout << "Error: Arguments not provided.\n";
		//exit(-1);
	}
	
	asio::error_code ec;
	asio::io_context iocontext;
	auto wg = asio::make_work_guard(iocontext.get_executor());
	std::thread contextthread([&](){ iocontext.run(); });
	
	//HTTPS
	asio::ssl::context sslcontext(asio::ssl::context::method::sslv23);
	asio::ssl::stream<asio::ip::tcp::socket> securesocket(iocontext, sslcontext);
	SSL_set_tlsext_host_name(securesocket.native_handle(), "mcapi.us");
	
	asio::ip::tcp::endpoint netendpoint;
	DNSresolve(&netendpoint, iocontext);
	
	std::string webrequest =
		"GET /server/status?ip=" + programinfo.serverip + "&port=" + programinfo.serverport + " HTTP/1.1\r\n"
		"Host: mcapi.us\r\n"
		"User-Agent: Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)\r\n" //otherwise cloudflare blocks...
		"Connection: close\r\n\r\n"
		;
	
	while(true) {
		securesocket = asio::ssl::stream<asio::ip::tcp::socket>(iocontext, sslcontext);
		SSL_set_tlsext_host_name(securesocket.native_handle(), "mcapi.us");
		
		connect(securesocket, netendpoint);
		
		readbuf.clear();
		resetreadnetdata();
		readnetdata(securesocket);
		
		securesocket.write_some(asio::buffer(webrequest.data(), webrequest.size()), ec);
		
		while(isdone == false) {}
		
		std::cout << "Read " << readbuf.size() << " bytes in total.\n-----\n";
		
		json js = json::parse(readbuf);
		std::cout << "Status: " << js["status"] << "\n";
		std::cout << "Online: " << js["online"] << "\n";
		std::cout << "MOTD: " << js["motd"] << "\n";
		std::cout << "Players: " << js["players"] << "\n";
		std::cout << "PlayersNow: " << js["players"]["now"] << "\n";
		std::cout << "PlayersMax: " << js["players"]["max"] << "\n";
		std::cout << "Sample: " << js["players"]["sample"] << "\n";
		std::cout << "ServerName: " << js["server"]["name"] << "\n";
		
		std::cout << "-----\n";
		for(int i = 60; i > 0; i--) {
			std::cout << "\rNext update in: " << std::setfill('0') << std::setw(2) << i << " second(s)." << std::flush;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << "\n" << std::setw(0);
	}
	
	wg.reset(); //work groups do not block!
	iocontext.stop();
	contextthread.join();
	
	return 0;
}