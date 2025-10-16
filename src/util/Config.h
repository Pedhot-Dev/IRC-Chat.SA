#pragma once
#include "CRGBA.h"
#include "mini/ini.h"

namespace util
{
	struct Config
	{
		std::string HOST;
		int PORT;
		std::string USERNAME;
		std::string CHANNEL;

		Config(const char *filename);
	};
} // namespace util