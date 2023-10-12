#include "botresponses.hpp"

dpp::cluster bottype::bothandle = dpp::cluster(
	"" //TODO: add bot token here! ATTN
);
std::thread bottype::botthread;

std::unordered_map<dpp::snowflake, bottype> bots;

void sendautomsg(dpp::snowflake aguildid) {
	uint32_t playersnow = 0;
	uint32_t playersold = 0;
	
	//port can be empty
	if(bots.at(aguildid).channelid && !programinfos.at(aguildid).serverip.empty()) {
		playersnow = programinfos.at(aguildid).serverdata["players"]["now"].template get<uint32_t>();
		if(!programinfos.at(aguildid).oldserver.is_null()) {
			playersold = programinfos.at(aguildid).oldserver["players"]["now"].template get<uint32_t>();
		}
	}
	else {
		playersold = playersnow; //block if statement
	}
	
	if(playersnow != playersold) {
		dpp::embed ebd;
		ebd.set_title("Server update");
		ebd.set_author(EMBED_AUTHOR_STRING);
		ebd.set_color(128);
		
		std::string playernamemsg;
		
		std::vector<std::string> playersoldv;
		for(uint32_t i = 0; i < playersold; i++) {
			playersoldv.push_back(programinfos.at(aguildid).oldserver["players"]["sample"][i]["name"].template get<std::string>());
		}

		std::vector<std::string> playersnowv;
		for(uint32_t i = 0; i < playersnow; i++) {
			playersnowv.push_back(programinfos.at(aguildid).serverdata["players"]["sample"][i]["name"].template get<std::string>());
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
		
		bottype::bothandle.message_create(dpp::message(bots.at(aguildid).channelid, ebd));
	}
}

namespace botresponse {
	void serverinfo(const dpp::slashcommand_t& aevent) {
		//get server information - all of it
		std::cout << "[COMMANDS] /serverinfo was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		if(programinfos.at(aevent.command.guild_id).serverdata.is_null()) {
			aevent.reply(dpp::message("No server information available yet").set_flags(dpp::m_ephemeral));
			std::cout << "[COMMANDS] /serverinfo returned - NO SERVER INFO AVAILABLE.\n";
			return;
		}
		
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Server information");
		ebd.set_description("All of the information regarding the server.\n");
		ebd.set_author(EMBED_AUTHOR_STRING);
		ebd.set_color(128);
		
		if(programinfos.at(aevent.command.guild_id).serverdata["online"].template get<bool>() == false) {
			//server offline
			ebd.add_field("Server offline!", "This server is currently offline.", true);
			msg.add_embed(ebd); aevent.reply(msg); return;
		}
		
		ebd.add_field("Name of Server", programinfos.at(aevent.command.guild_id).serverdata["server"]["name"], true);
		ebd.add_field("Online", ((programinfos.at(aevent.command.guild_id).serverdata["online"].template get<bool>() == true) ? "Yes" : "No"), true);
		ebd.add_field("Message", programinfos.at(aevent.command.guild_id).serverdata["motd"], true);
		
		uint32_t playersnow = programinfos.at(aevent.command.guild_id).serverdata["players"]["now"].template get<uint32_t>();
		ebd.add_field("Current Players", std::to_string(playersnow), true);
		ebd.add_field("Maximum Players", std::to_string(programinfos.at(aevent.command.guild_id).serverdata["players"]["max"].template get<uint32_t>()), true);
		
		std::string playernamemsg;
		json playersnames = programinfos.at(aevent.command.guild_id).serverdata["players"]["sample"];
			
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
		std::cout << "[COMMANDS] /status was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		if(programinfos.at(aevent.command.guild_id).serverdata.is_null()) {
			aevent.reply(dpp::message("No server information available yet").set_flags(dpp::m_ephemeral));
			std::cout << "[COMMANDS] /status returned - NO SERVER INFO AVAILABLE.\n";
			return;
		}
		
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Server status");
		ebd.set_description("Current server status\n");
		ebd.set_author(EMBED_AUTHOR_STRING);
		ebd.set_color(128);
		
		if(programinfos.at(aevent.command.guild_id).serverdata["online"].template get<bool>() == false) {
			ebd.add_field("Server offline!", "This server is currently offline.", true);
			msg.add_embed(ebd);	aevent.reply(msg); return;
		}
		
		ebd.add_field("Name of Server", programinfos.at(aevent.command.guild_id).serverdata["server"]["name"], true);
		ebd.add_field("Online", ((programinfos.at(aevent.command.guild_id).serverdata["online"].template get<bool>() == true) ? "Yes" : "No"), true);
		ebd.add_field("Message", programinfos.at(aevent.command.guild_id).serverdata["motd"], true);
		
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	void players(const dpp::slashcommand_t& aevent) {
		//get player amount and their names
		std::cout << "[COMMANDS] /players was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		if(programinfos.at(aevent.command.guild_id).serverdata.is_null()) { 
			aevent.reply(dpp::message("No server information available yet").set_flags(dpp::m_ephemeral));
			std::cout << "[COMMANDS] /players returned - NO SERVER INFO AVAILABLE.\n";
			return;
		}
		
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Player information");
		ebd.set_description("A list of Players.\n");
		ebd.set_author(EMBED_AUTHOR_STRING);
		ebd.set_color(128);
		
		uint32_t playersnow = programinfos.at(aevent.command.guild_id).serverdata["players"]["now"].template get<uint32_t>();
		ebd.add_field("Current Players", std::to_string(playersnow), true);
		ebd.add_field("Maximum Players", std::to_string(programinfos.at(aevent.command.guild_id).serverdata["players"]["max"].template get<uint32_t>()), true);
		
		std::string playernamemsg;
		json playersnames = programinfos.at(aevent.command.guild_id).serverdata["players"]["sample"];
			
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
		std::cout << "[COMMANDS] /help was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Commands");
		ebd.set_description("List of all of the commands which this bot supports.\n");
		ebd.set_author(EMBED_AUTHOR_STRING);
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
		std::cout << "[COMMANDS] /credits was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		dpp::message msg;
		dpp::embed ebd;
		ebd.set_title("Author");
		ebd.set_description(
			"MCServerStatusBot is a bot for getting information about a Minecraft server in a Discord Server.\n"
			"Supports automatic updates, changing server IP and port while running and slash commands."
		);
		ebd.set_author(EMBED_AUTHOR_STRING);
		ebd.set_color(128);
		msg.add_embed(ebd);
		aevent.reply(msg);
	}
	
	void home(const dpp::slashcommand_t& aevent) {
		//set this channel to home channel, admin only
		std::cout << "[COMMANDS] /home was called on server " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		bots.at(aevent.command.guild_id).channelid = aevent.command.channel_id;
		
		dpp::channel homechannel = bottype::bothandle.channel_get_sync(bots.at(aevent.command.guild_id).channelid);
		
		aevent.reply(dpp::message("Channel " + homechannel.name + " is now the home channel.").set_flags(dpp::m_ephemeral));
	}
	void change(const dpp::slashcommand_t& aevent) {
		//changes the server IP and port, admin only
		if(std::get<dpp::command_interaction>(aevent.command.data).options.size() > 1) {
			std::cout << "[COMMANDS] /change [ip] [port] was called on " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		}
		else { std::cout << "[COMMANDS] /change [ip] was called on " + dpp::find_guild(aevent.command.guild_id)->name + ".\n"; }
		
		programinfos.at(aevent.command.guild_id).serverip = std::get<std::string>(aevent.get_parameter("ip"));
		
		if(std::holds_alternative<std::string>(aevent.get_parameter("port"))) {
			programinfos.at(aevent.command.guild_id).serverport = std::get<std::string>(aevent.get_parameter("port"));
		}
		
		programinfos.at(aevent.command.guild_id).webrequest = makerequest(aevent.command.guild_id);
		
		if(std::get<dpp::command_interaction>(aevent.command.data).options.size() > 1) {
			aevent.reply(
				dpp::message(
					"IP and port changed.\nIP is now " +
					programinfos.at(aevent.command.guild_id).serverip +
					" and the port is now " + programinfos.at(aevent.command.guild_id).serverport
				).set_flags(dpp::m_ephemeral)
			);
		}
		else {
			std::cout << "[COMMANDS] /change [ip] [port] was called on " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
			aevent.reply(
				dpp::message(
					"IP changed.\nIP is now " +
					programinfos.at(aevent.command.guild_id).serverip
				).set_flags(dpp::m_ephemeral)
			);
		}
	}
	void enableautoupdate(const dpp::slashcommand_t& aevent) {
		std::cout << "[COMMANDS] /enableautoupdate was called on " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		bots.at(aevent.command.guild_id).autoenabled = true; 
		aevent.reply(dpp::message("Automatic updates enabled.").set_flags(dpp::m_ephemeral));
	}
	void disableautoupdate(const dpp::slashcommand_t& aevent) {
		std::cout << "[COMMANDS] /disableautoupdate was called on " + dpp::find_guild(aevent.command.guild_id)->name + ".\n";
		bots.at(aevent.command.guild_id).autoenabled = false; 
		aevent.reply(dpp::message("Automatic updates disabled.").set_flags(dpp::m_ephemeral));
	}
}

void initbot() {
	bottype::bothandle.global_commands_get([&](const dpp::confirmation_callback_t& aconcl){
		for(auto& [key, command] : std::get<dpp::slashcommand_map>(aconcl.value)) {
			bottype::bothandle.global_command_delete(command.id);
		};
	});
	
	bottype::bothandle.on_log(dpp::utility::cout_logger());
	
	bottype::bothandle.on_slashcommand([&](const dpp::slashcommand_t& event) {
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
	bottype::bothandle.on_ready([&](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			//normal commands
			
			bottype::bothandle.global_command_create(dpp::slashcommand("serverinfo", "Gets general information about the server.", bottype::bothandle.me.id));
			bottype::bothandle.global_command_create(dpp::slashcommand("status", "Gets current server status.", bottype::bothandle.me.id));
			bottype::bothandle.global_command_create(dpp::slashcommand("players", "List and amount of players currently online.", bottype::bothandle.me.id));
			bottype::bothandle.global_command_create(dpp::slashcommand("help", "Help menu with bot usage.", bottype::bothandle.me.id));
			bottype::bothandle.global_command_create(dpp::slashcommand("credits", "Author of the bot.", bottype::bothandle.me.id));
			
			//admin only commands
			
			dpp::slashcommand homecommand("home", "(admin only) Changes the bot so that automatic updates are written to this channel.", bottype::bothandle.me.id);
			homecommand.default_member_permissions = 0; //admin only
			bottype::bothandle.global_command_create(homecommand);
			
			dpp::slashcommand changecommand("change", "Params: [ip], [port]; (admin only) Changes IP and port of server to display.", bottype::bothandle.me.id);
			changecommand.default_member_permissions = 0;
			changecommand.add_option(dpp::command_option(dpp::co_string, "ip", "IP of the new server", true));
			changecommand.add_option(dpp::command_option(dpp::co_string, "port", "Port of the new server", false));
			bottype::bothandle.global_command_create(changecommand);
			
			dpp::slashcommand enableaucommand("enableautoupdate", "(admin only) Enables automatic updates.", bottype::bothandle.me.id);
			enableaucommand.default_member_permissions = 0;
			bottype::bothandle.global_command_create(enableaucommand);
			
			dpp::slashcommand disableaucommand("disableautoupdate", "(admin only) Disables automatic updates.", bottype::bothandle.me.id);
			disableaucommand.default_member_permissions = 0;
			bottype::bothandle.global_command_create(disableaucommand);
		}
	});
	bottype::bothandle.on_guild_create([&](const dpp::guild_create_t& event) {
		programinfos.emplace(std::make_pair(event.created->id, programinfotype{}));
		bots.emplace(std::make_pair(event.created->id, bottype{}));
		std::cout << "[SERVER] Bot added to server " + event.created->name + ".\n";
	});
	bottype::bothandle.on_guild_delete([&](const dpp::guild_delete_t& event) {
		programinfos.erase(programinfos.find(event.deleted->id));
		bots.erase(bots.find(event.deleted->id));
		std::cout << "[SERVER] Bot removed from server " + event.deleted->name + ".\n";
	});
	
	bottype::botthread = std::thread([&](){ 
		std::cout << "[BOT] Bot thread started!\n";
		bottype::bothandle.start(dpp::st_wait); 
	});
}