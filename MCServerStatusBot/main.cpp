#include "botresponses.hpp" //includes other stuff

int main(int argc, char **argv) {
	std::cout << "MC Server Status Discord Bot\n(c) Martin/MegapolisPlater, 2023\nPress q/Q and ENTER to exit.\n";
	if(argc == 3) {
		programinfo.serverip = argv[1]; //first arg doesnt count
		programinfo.serverport = argv[2];
		std::cout << "ServerName: " << programinfo.serverip << "\n" << "ServerPort: " << programinfo.serverport << "\n";
	}
	else { std::cout << "Arguments not provided, using defaults.\n"; }
	
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
	
	programinfo.webrequest = makerequest();
	
	//get guilds, create arrays
	//dpp::guild_map gm = bot.bothandle.current_user_get_guilds_sync();
	
	std::thread stopthread([&](){
		char c;
		while(programinfo.mainloop) {
			std::cin >> c;
			if(c == 'Q' || c == 'q') {
				std::cout << "STOP key recieved.\n";
				programinfo.mainloop = false;
			}
		}
	});

	while(programinfo.mainloop) {
		securesocket = asio::ssl::stream<asio::ip::tcp::socket>(iocontext, sslcontext);
		SSL_set_tlsext_host_name(securesocket.native_handle(), "mcapi.us");
		connect(securesocket, netendpoint);
		
		readbuf.clear();
		resetreadnetdata();
		readnetdata(securesocket);
		securesocket.write_some(asio::buffer(programinfo.webrequest.data(), programinfo.webrequest.size()), ec);
		while(isdone.load() == false) {}
		
		std::cout << "Read " << readbuf.size() << " bytes in total.\n";
		programinfo.serverdata = json::parse(readbuf);
		std::cout << "[NETWORK] Data recieved and saved.\n";

		sendautomsg();

		programinfo.oldserver = programinfo.serverdata;
		
		for(int i = API_UPDATE_TIME_S; i > 0; i--) {
			if(!programinfo.mainloop) { goto ending; }
			if(i % 10 == 0) { std::cout << "[NETWORK] Next data update in: "<< i << "s.\n"; }
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	ending:
	
	std::cout << "Shutting down...\n";
	
	wg.reset(); //work groups do not block!
	iocontext.stop();
	contextthread.join();
	
	bot.bothandle.shutdown();
	if(bot.botthread.joinable()) { bot.botthread.join(); }
	
	if(stopthread.joinable()) { stopthread.join(); }
	
	return 0;
}