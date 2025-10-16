#include "Config.h"

namespace util
{
	void ExtractColorFromString(std::string in, CRGBA &out)
	{
		// Remove whitespace
		in.erase(std::ranges::remove_if(in, isspace).begin(), in.end());

		bool didR = false, didG = false, didB = false, didA = false;

		size_t pos = 0;
		for (unsigned char i = 0; i < 4; i++)
		{
			pos = in.find(',');

			if (!didR)
			{
				out.r = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didR = true;
			}
			else if (!didG)
			{
				out.g = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didG = true;
			}
			else if (!didB)
			{
				out.b = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didB = true;
			}
			else if (!didA)
			{
				out.a = (unsigned char)std::stoi(in.substr(0, pos).c_str());
				didA = true;
			}

			in.erase(0, pos + 1);
		}
	}

	Config::Config(const char *filename)
	{
		mINI::INIFile file(filename);

		mINI::INIStructure ini;
		file.read(ini);

		HOST = ini["Settings"]["Host"];
		PORT = std::stoi(ini["Settings"]["Port"]);
		USERNAME = ini["Settings"]["Username"];
		CHANNEL = "#" + ini["Settings"]["Channel"];
		file.write(ini);
	}
} // namespace util