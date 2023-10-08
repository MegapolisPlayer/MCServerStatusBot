//Minecraft Server Status Discord Bot
//by Martin/MegapolisPlayer
//Licensed under CC-BY 4.0
//
//SECTIONS
//--
//ASIO functions       - 
//Bot responses        -
//Main function, setup -
//Main function, bot   -
//Main function, loop  -
//Main function, end   -

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>

#include "json.hpp"
using namespace nlohmann;

#include "dpp/dpp.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ssl.hpp"

#define API_UPDATE_TIME_S 90

static struct {
	std::string serverip = "witch.hostify.cz";
	std::string serverport = "38170";
	std::string bottoken = "";
	
	std::string webrequest;
	
	json oldserver;
	json serverdata;
	
	bool mainloop = true;
} programinfo;

//ASIO FUNCTIONS

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
	return 
		"GET /server/status?ip=" + programinfo.serverip + "&port=" + programinfo.serverport + " HTTP/1.1\r\n"
		"Host: mcapi.us\r\n"
		"User-Agent: Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)\r\n" //otherwise cloudflare blocks...
		"Connection: close\r\n\r\n";
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
					for(uint64_t i = 0; i < readbuf.size(); i++) { std::cout << readbuf[i]; }
					std::cout << "Error " << ec.value() << ": " << ec.message() << "\n"; exit(-1);
				}
			}
		}
	);
}

void resetreadnetdata() {
	tempbuf.clear(); tempbuf.resize(10000);
	isdone = false; hasreadhttpheader = false;
}

//BOT RESPONSES

static struct {
	dpp::cluster bothandle = dpp::cluster(programinfo.bottoken);
	dpp::snowflake channelid = 0; //channel id for automatic updates
	bool autoenabled = false;
} bot;

namespace botresponse {
	void serverinfo(const dpp::slashcommand_t& aevent) {
		//get server information - all of it
		std::cout << "[COMMANDS] /serverinfo was called.\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Server information");
		ebd.set_description("All of the information regarding the server.\n");
		ebd.set_author(
			"MCServerStatusBot by Martin/MegapolisPlayer",
			"https://megapolisplayer.github.io", 
			"https://megapolisplayer.github.io/assets/icons/favicon.png"
		);
		ebd.set_color(128);
		
		if(programinfo.serverdata["online"].template get<bool>() == false) {
			//server offline
			ebd.add_field("Server offline!", "This server is currently offline.", true);
			msg.add_embed(ebd); aevent.reply(msg); return;
		}
		
		ebd.add_field("Name of Server", programinfo.serverdata["server"]["name"], true);
		ebd.add_field("Online", ((programinfo.serverdata["online"].template get<bool>() == true) ? "Yes" : "No"), true);
		ebd.add_field("Message", programinfo.serverdata["motd"], true);
		
		uint32_t playersnow = programinfo.serverdata["players"]["now"].template get<uint32_t>();
		ebd.add_field("Current Players", std::to_string(playersnow), true);
		ebd.add_field("Maximum Players", std::to_string(programinfo.serverdata["players"]["max"].template get<uint32_t>()), true);
		
		std::string playernamemsg;
		json playersnames = programinfo.serverdata["players"]["sample"];
			
		for(uint32_t i = 0; i < playersnow; i++) {
			playernamemsg += playersnames[i]["name"].template get<std::string>();
			if(i != playersnow - 1) { playernamemsg += ", "; }
		}
		if(playersnow == 0) { playernamemsg += "none"; }
		
		ebd.add_field("List of Players", playernamemsg, false);
		
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	void status(const dpp::slashcommand_t& aevent) {
		//get server status
		std::cout << "[COMMANDS] /status was called.\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Server status");
		ebd.set_description("Current server status\n");
		ebd.set_author(
			"MCServerStatusBot by Martin/MegapolisPlayer",
			"https://megapolisplayer.github.io", 
			"https://megapolisplayer.github.io/assets/icons/favicon.png"
		);
		ebd.set_color(128);
		
		if(programinfo.serverdata["online"].template get<bool>() == false) {
			ebd.add_field("Server offline!", "This server is currently offline.", true);
			msg.add_embed(ebd);	aevent.reply(msg); return;
		}
		
		ebd.add_field("Name of Server", programinfo.serverdata["server"]["name"], true);
		ebd.add_field("Online", ((programinfo.serverdata["online"].template get<bool>() == true) ? "Yes" : "No"), true);
		ebd.add_field("Message", programinfo.serverdata["motd"], true);
		
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	void players(const dpp::slashcommand_t& aevent) {
		//get player amount and their names
		std::cout << "[COMMANDS] /players was called.\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Player information");
		ebd.set_description("A list of Players.\n");
		ebd.set_author(
			"MCServerStatusBot by Martin/MegapolisPlayer",
			"https://megapolisplayer.github.io", 
			"https://megapolisplayer.github.io/assets/icons/favicon.png"
		);
		ebd.set_color(128);
		
		uint32_t playersnow = programinfo.serverdata["players"]["now"].template get<uint32_t>();
		ebd.add_field("Current Players", std::to_string(playersnow), true);
		ebd.add_field("Maximum Players", std::to_string(programinfo.serverdata["players"]["max"].template get<uint32_t>()), true);
		
		std::string playernamemsg;
		json playersnames = programinfo.serverdata["players"]["sample"];
			
		for(uint32_t i = 0; i < playersnow; i++) {
			playernamemsg += playersnames[i]["name"].template get<std::string>();
			if(i != playersnow - 1) {
				playernamemsg += ", ";
			}
		}
		if(playersnow == 0) { playernamemsg += "none"; }
		
		ebd.add_field("List of Players", playernamemsg, false);
		
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	void help(const dpp::slashcommand_t& aevent) {
		//send list of commands, in an embed
		std::cout << "[COMMANDS] /help was called.\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Commands");
		ebd.set_description("List of all of the commands which this bot supports.\n");
		ebd.set_author(
			"MCServerStatusBot by Martin/MegapolisPlayer",
			"https://megapolisplayer.github.io", 
			"https://megapolisplayer.github.io/assets/icons/favicon.png"
		);
		ebd.set_color(128);
		
		ebd.add_field("Regular", "Commands accessible to everyone.", false);
		
		ebd.add_field("/serverinfo", "Gets general information about the server.", true);
		ebd.add_field("/status", "Gets current server status.", true);
		ebd.add_field("/players", "List and amount of players currently online.", true);
		ebd.add_field("/help", "Gets general information about the server.", true);
		ebd.add_field("/credits", "Gets general information about the server.", true);
		
		ebd.add_field("Admin only", "Commands accessible only to admins.", false);
		
		ebd.add_field("/home", "Changes the bot so that automatic updates are written to this channel.", true);
		ebd.add_field("/change [ip] [port]", "Changes IP and port of server to display.", true);
		ebd.add_field("/enableautoupdate", "Enables automatic updates.", true);
		ebd.add_field("/disableautoupdate", "Disables automatic updates.", true);
		
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	void credits(const dpp::slashcommand_t& aevent) {
		//send embed with credits
		std::cout << "[COMMANDS] /credits was called.\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Author");
		ebd.set_description(
			"MCServerStatusBot is a bot for getting information about a Minecraft server in a Discord Server.\n"
			"Supports automatic updates, changing server IP and port while running and slash commands."
		);
		ebd.set_author(
			"MCServerStatusBot by Martin/MegapolisPlayer",
			"https://megapolisplayer.github.io", 
			"https://megapolisplayer.github.io/assets/icons/favicon.png"
		);
		ebd.set_color(128);
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	
	void home(const dpp::slashcommand_t& aevent) {
		//set this channel to home channel, admin only
		std::cout << "[COMMANDS] /home was called.\n";
		bot.channelid = aevent.command.channel_id;
		
		dpp::channel homechannel = bot.bothandle.channel_get_sync(bot.channelid);
		
		aevent.reply(dpp::message("Channel " + homechannel.name + " is now the home channel.").set_flags(dpp::m_ephemeral));
	}
	void change(const dpp::slashcommand_t& aevent) {
		//changes the server IP and port, admin only, TODO: finish
		std::cout << "[COMMANDS] /chnage [ip] [port] was called.\n";
		
		programinfo.serverip = std::get<std::string>(aevent.get_parameter("ip"));
		programinfo.serverport = std::get<std::string>(aevent.get_parameter("port"));
		
		programinfo.webrequest = makerequest();
		
		aevent.reply(dpp::message("IP and port changed.\nIP is now " + programinfo.serverip + " and the port is now " + programinfo.serverport).set_flags(dpp::m_ephemeral));
	}
	void enableautoupdate(const dpp::slashcommand_t& aevent) {
		std::cout << "[COMMANDS] /enableautoupdate was called.\n";
		bot.autoenabled = true; 
		aevent.reply(dpp::message("Automatic updates enabled.").set_flags(dpp::m_ephemeral));
	}
	void disableautoupdate(const dpp::slashcommand_t& aevent) {
		std::cout << "[COMMANDS] /disableautoupdate was called.\n";
		bot.autoenabled = false; 
		aevent.reply(dpp::message("Automatic updates disabled.").set_flags(dpp::m_ephemeral));
	}
}

int main(int argc, char **argv) {
	std::cout << "MC Server Status Discord Bot\n(c) Martin/MegapolisPlater, 2023\nPress q/Q and ENTER to exit.\n";
	if(argc == 3) {
		programinfo.serverip = argv[1]; //first arg doesnt count
		programinfo.serverport = argv[2];
		std::cout << "ServerName: " << programinfo.serverip << "\n" << "ServerPort: " << programinfo.serverport << "\n";
	}
	else { std::cout << "Arguments not provided, using defaults.\n"; }
	
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
	
	bot.bothandle.on_log(dpp::utility::cout_logger());
	
	bot.bothandle.on_slashcommand([&](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "serverinfo") { botresponse::serverinfo(event); return; }
		if (event.command.get_command_name() == "status") { botresponse::status(event); return; }
		if (event.command.get_command_name() == "players") { botresponse::players(event); return; }
		if (event.command.get_command_name() == "help") { botresponse::help(event); return; }
		if (event.command.get_command_name() == "credits") { botresponse::credits(event); return; }
		
		if (event.command.get_command_name() == "home") { botresponse::home(event); return; }
		if (event.command.get_command_name() == "change") { botresponse::change(event); return; }
		if (event.command.get_command_name() == "enableautoupdate") { botresponse::enableautoupdate(event); return; }
		if (event.command.get_command_name() == "disableautoupdate") { botresponse::disableautoupdate(event); return; }
		
	});
	bot.bothandle.on_ready([&](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			//normal commands
			
			bot.bothandle.global_command_create(dpp::slashcommand("serverinfo", "Gets general information about the server.", bot.bothandle.me.id));
			bot.bothandle.global_command_create(dpp::slashcommand("status", "Gets current server status.", bot.bothandle.me.id));
			bot.bothandle.global_command_create(dpp::slashcommand("players", "List and amount of players currently online.", bot.bothandle.me.id));
			bot.bothandle.global_command_create(dpp::slashcommand("help", "Help menu with bot usage.", bot.bothandle.me.id));
			bot.bothandle.global_command_create(dpp::slashcommand("credits", "Author of the bot.", bot.bothandle.me.id));
			
			//admin only commands
			
			dpp::slashcommand homecommand("home", "(admin only) Changes the bot so that automatic updates are written to this channel.", bot.bothandle.me.id);
			homecommand.default_member_permissions = 0; //admin only
			bot.bothandle.global_command_create(homecommand);
			
			dpp::slashcommand changecommand("change", "Params: [ip], [port]; (admin only) Changes IP and port of server to display.", bot.bothandle.me.id);
			changecommand.default_member_permissions = 0;
			changecommand.add_option(dpp::command_option(dpp::co_string, "ip", "IP of the new server", true));
			changecommand.add_option(dpp::command_option(dpp::co_integer, "port", "Port of the new server", true));
			bot.bothandle.global_command_create(changecommand);
			
			dpp::slashcommand enableaucommand("enableautoupdate", "(admin only) Enables automatic updates.", bot.bothandle.me.id);
			enableaucommand.default_member_permissions = 0;
			bot.bothandle.global_command_create(enableaucommand);
			
			dpp::slashcommand disableaucommand("disableautoupdate", "(admin only) Disables automatic updates.", bot.bothandle.me.id);
			disableaucommand.default_member_permissions = 0;
			bot.bothandle.global_command_create(disableaucommand);
		}
	});
	
	std::thread botthread([&](){ bot.bothandle.start(dpp::st_wait); });
	
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

	uint32_t playersnow = 0;
	uint32_t playersold = 0;
	while(programinfo.mainloop) {
		securesocket = asio::ssl::stream<asio::ip::tcp::socket>(iocontext, sslcontext);
		SSL_set_tlsext_host_name(securesocket.native_handle(), "mcapi.us");
		connect(securesocket, netendpoint);
		
		readbuf.clear();
		resetreadnetdata();
		readnetdata(securesocket);
		securesocket.write_some(asio::buffer(programinfo.webrequest.data(), programinfo.webrequest.size()), ec);
		while(isdone == false) {}
		
		std::cout << "Read " << readbuf.size() << " bytes in total.\n";
		programinfo.serverdata = json::parse(readbuf);
		std::cout << "[NETWORK] Data recieved and saved.\n";
		
		if(bot.channelid) {
			playersnow = programinfo.serverdata["players"]["now"].template get<uint32_t>();
			if(!programinfo.oldserver.is_null()) {
				playersold = programinfo.oldserver["players"]["now"].template get<uint32_t>();
			}
		}
		else {
			playersold = playersnow; //block if statement
		}
		
		if(playersnow != playersold) {
			dpp::embed ebd;
			ebd.set_title("Server update");
			ebd.set_author(
				"MCServerStatusBot by Martin/MegapolisPlayer",
				"https://megapolisplayer.github.io", 
				"https://megapolisplayer.github.io/assets/icons/favicon.png"
			);
			ebd.set_color(128);
			
			std::string playernamemsg;
			
			std::vector<std::string> playersoldv;
			for(uint32_t i = 0; i < playersold; i++) {
				playersoldv.push_back(programinfo.oldserver["players"]["sample"][i]["name"].template get<std::string>());
			}

			std::vector<std::string> playersnowv;
			for(uint32_t i = 0; i < playersnow; i++) {
				playersnowv.push_back(programinfo.serverdata["players"]["sample"][i]["name"].template get<std::string>());
			}
						
			std::sort(playersoldv.begin(), playersoldv.end());
			std::sort(playersnowv.begin(), playersnowv.end());
			std::vector<std::string> playersdiff;
			std::set_symmetric_difference(playersoldv.begin(), playersoldv.end(), playersnowv.begin(), playersnowv.end(), std::back_inserter(playersdiff));

			for(uint32_t i = 0; i < playersdiff.size(); i++) {
				playernamemsg += playersdiff[i];
				if(i != playersdiff.size() - 1) {
					playernamemsg += ", ";
				}
			}
			
			if(playersold < playersnow) {
				ebd.set_description("New people have joined the server!\n");
				ebd.add_field("Player(s) joined:", playernamemsg, false);
			}
			else if(playersold > playersnow) {
				ebd.set_description("People have left the server!\n");
				ebd.add_field("Player(s) left:", playernamemsg, false);
			}
			
			bot.bothandle.message_create(dpp::message(bot.channelid, ebd));
		}
		
		programinfo.oldserver = programinfo.serverdata;
		
		for(int i = API_UPDATE_TIME_S; i > 0; i--) {
			if(!programinfo.mainloop) { goto ending; }
			if(i % 10 == 0) {
				std::cout << "[NETWORK] Next data update in: "<< i << "s.\n";
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	ending:
	
	std::cout << "Shutting down...\n";
	
	wg.reset(); //work groups do not block!
	iocontext.stop();
	contextthread.join();
	
	bot.bothandle.shutdown();
	botthread.join();
	
	stopthread.join();
	
	return 0;
}