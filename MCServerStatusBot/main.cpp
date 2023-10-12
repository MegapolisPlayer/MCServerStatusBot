#include "botresponses.hpp" //includes other stuff

int main(int argc, char **argv) {
	std::cout << "MC Server Status Discord Bot\n(c) Martin/MegapolisPlater, 2023\nPress q/Q and ENTER to exit.\nExit after the shards have started.\n";
	
	initbot();
	
	//BASIC ASIO
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
	
	std::thread stopthread([&](){
		std::cout << "STOP key reciever active. Wait for shard start.\n";
		char c;
		while(programinfotype::mainloop) {
			std::cin >> c;
			if(c == 'Q' || c == 'q') {
				std::cout << "STOP key recieved.\n";
				programinfotype::mainloop = false;
			}
		}
	});
	
	while(programinfotype::mainloop) {
		for(auto& [key, programinfo] : programinfos) {
			if(programinfo.serverip.empty()) { 
				std::cout << "[NETWORK] No read for server " << dpp::find_guild(key)->name << ", no server set up.\n";
				continue;
			}
			std::cout << "[NETWORK] Reading server data for server " << dpp::find_guild(key)->name << ".\n";
			
			securesocket = asio::ssl::stream<asio::ip::tcp::socket>(iocontext, sslcontext);
			SSL_set_tlsext_host_name(securesocket.native_handle(), "mcapi.us");
			connect(securesocket, netendpoint);
			
			readbuf.clear();
			resetreadnetdata();
			readnetdata(securesocket);
			programinfo.webrequest = makerequest(key);
			securesocket.write_some(asio::buffer(programinfo.webrequest.data(), programinfo.webrequest.size()), ec);
			while(isdone.load() == false) {}
			
			std::cout << "Read " << readbuf.size() << " bytes in total.\n";
			programinfo.serverdata = json::parse(readbuf);
			std::cout << "[NETWORK] Data recieved and saved.\n";

			sendautomsg(key);

			programinfo.oldserver = programinfo.serverdata;
		}
		
		for(int i = API_UPDATE_TIME_S; i > 0; i--) {
			if(!programinfotype::mainloop) { goto ending; }
			if(i % 10 == 0) { std::cout << "[NETWORK] Next data update in: "<< i << "s.\n"; }
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	ending:
	
	std::cout << "Shutting down...\n";
	
	wg.reset(); //work groups do not block!
	iocontext.stop();
	contextthread.join();
	
	bottype::bothandle.shutdown();
	if(bottype::botthread.joinable()) { bottype::botthread.join(); }
	
	if(stopthread.joinable()) { stopthread.join(); }
	
	return 0;
}