# MCServerStatusBot
Discord bot for checking Minecraft server status, written in C++

# Some information
This bot uses the mcapi.us service in order to get the information.
Due to this being a service run by a single person, this bot only sends requests and data updates every 120 seconds.
This value is configurable in the `include.hpp` file by changing the `API_UPDATE_TIME_S` macro.
Before use it is necessary to set the bot token. This is done in the `botresponses.cpp` file at line 4.
(the very top of the file, the token is provided as a parameter to the dpp::cluster constructor.

# First-party hosting
I do host this bot however I do not guarantee 24/7 coverage.
If you want to use the first-party service on your Discord server, click this link:
https://discord.com/api/oauth2/authorize?client_id=1162171089879842916&permissions=27483160706112&scope=bot

# Libraries
This project uses DPP/D++, ASIO (non-Boost) and Nlohmann's JSON library.
ASIO and JSON libraries are included however DPP must be set up separately.

# Licensing
This bot is licensed under the CC-BY 4.0 license.
The full text of the license may be found here:
https://creativecommons.org/licenses/by/4.0/legalcode.en
