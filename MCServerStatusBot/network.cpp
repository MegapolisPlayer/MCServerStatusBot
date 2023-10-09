#include "network.hpp"

programinfotype programinfo;

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

std::string makerequest() {
	if(programinfo.serverport.empty()) {
		return 
			"GET /server/status?ip=" + programinfo.serverip + " HTTP/1.1\r\n"
			"Host: mcapi.us\r\n"
			"User-Agent: Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)\r\n" //otherwise cloudflare blocks...
			"Connection: close\r\n\r\n";
	}
	else {
		return 
			"GET /server/status?ip=" + programinfo.serverip + "&port=" + programinfo.serverport + " HTTP/1.1\r\n"
			"Host: mcapi.us\r\n"
			"User-Agent: Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)\r\n" //otherwise cloudflare blocks...
			"Connection: close\r\n\r\n";
	}
}

std::vector<char> readbuf;
std::vector<char> tempbuf(10000);
std::atomic_bool isdone(false);
bool hasreadhttpheader = false;

const char* httpsep = "\r\n\r\n";

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
					std::cout << "Read finished.\n"; isdone.store(true);
				}
				else { 
					std::cout << "Read so far:\n";
					for(uint64_t i = 0; i < readbuf.size(); i++) { std::cout << readbuf[i]; }
					std::cout << "Error " << ec.value() << ": " << ec.message() << "\n"; exit(-1);
				}
			}
		}
	);
}

void resetreadnetdata() {
	tempbuf.clear(); tempbuf.resize(10000);
	isdone.store(false); hasreadhttpheader = false;
}