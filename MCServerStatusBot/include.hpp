#ifndef MCSSB_INCLUDE
#define MCSSB_INCLUDE

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

//Interval (in seconds) when the bot updates MC server statuses, default 120 (2min)
#define API_UPDATE_TIME_S 120

#define EMBED_AUTHOR_STRING "MCServerStatusBot by Martin/MegapolisPlayer", \
							"https://megapolisplayer.github.io", \
							"https://megapolisplayer.github.io/assets/icons/favicon.png" \

#endif