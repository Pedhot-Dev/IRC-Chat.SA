#include "IRCClient.h"
#include "plugin.h"
#include "sampapi/CChat.h"
#include "sampapi/CInput.h"
#include "util/Config.h"
#include <thread>

using namespace plugin;
using namespace sampapi::v03dl;

IRCClient irc;
util::Config cfg = util::Config("IRCChat.SA.ini");

void InitializeIRC() {
    if (!RefChat() || !RefInputBox()) return;

    RefChat()->AddMessage(-1, "{00FFFF}[IRCPlugin] Initializing...");
    if (irc.Connect(cfg.HOST, cfg.PORT, cfg.USERNAME, cfg.CHANNEL)) {
        RefChat()->AddMessage(-1, "{00FF00}[IRC] Connected to server!");
    } else {
        RefChat()->AddMessage(-1, "{FF0000}[IRC] Connection failed.");
    }

    RefInputBox()->AddCommand("stoirc", [](const char *params) {
        if (!irc.IsConnected()) {
            RefChat()->AddMessage(-1, "{FF0000}You are not connected to IRC.");
            return;
        }
        if (!params || strlen(params) == 0) {
            RefChat()->AddMessage(-1, "{FFFF00}Usage: /stoirc <message>");
            return;
        }

        irc.SendMessage(params, "true");
        RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + cfg.USERNAME + ": " + std::string(params)).c_str());
    });

    RefInputBox()->AddCommand("stoirc_pm", [](const char *params) {
        if (!irc.IsConnected()) {
            RefChat()->AddMessage(-1, "{FF0000}You are not connected to IRC.");
            return;
        }
        if (!params || strlen(params) == 0) {
            RefChat()->AddMessage(-1, "{FFFF00}Usage: /stoirc_pm <user> <message>");
            return;
        }

        std::string paramsStr(params);
        size_t spacePos = paramsStr.find(' ');

        if (spacePos == std::string::npos) {
            RefChat()->AddMessage(-1, "{FFFF00}Usage: /stoirc_pm <user> <message>");
            return;
        }

        std::string user = paramsStr.substr(0, spacePos);
        std::string message = paramsStr.substr(spacePos + 1);

        if (message.empty()) {
            RefChat()->AddMessage(-1, "{FFFF00}Usage: /stoirc_pm <user> <message>");
            return;
        }

        irc.SendMessage(message, user);
        RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] PM to " + user + ": " + message).c_str());
    });

    irc.SetOnMessage([](const std::string &msg) {
        if (msg.find("PRIVMSG") != std::string::npos) {
            size_t prefixEnd = msg.find('!');
            size_t channelStart = msg.find("PRIVMSG ") + 8;
            size_t channelEnd = msg.find(' ', channelStart);
            size_t msgStart = msg.find(" :");

            if (prefixEnd != std::string::npos && msgStart != std::string::npos) {
                std::string nick = msg.substr(1, prefixEnd - 1);
                std::string target = msg.substr(channelStart, channelEnd - channelStart);
                std::string text = msg.substr(msgStart + 2);

                if (target[0] != '#' && target[0] != '&') {
                    RefChat()->AddMessage(-1, ("{FFA500}[IRC] PM from " + nick + ": " + text).c_str());
                } else {
                    RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + nick + ": " + text).c_str());
                }
            }
        }
        else if (msg.find("JOIN") != std::string::npos) {
            size_t start = msg.find(':');
            size_t ex = msg.find('!');
            std::string nick = msg.substr(start + 1, ex - start - 1);
            RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + nick + " has joined the chat.").c_str());
        }
        else if (msg.find("QUIT") != std::string::npos) {
            size_t start = msg.find(':');
            size_t ex = msg.find('!');
            std::string nick = msg.substr(start + 1, ex - start - 1);
            RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + nick + " has quit the chat.").c_str());
        }
    });


    RefChat()->AddMessage(-1, "{00FFFF}[IRCPlugin] Ready!");
}

class IRCPlugin {
public:
    struct TimerData {
        std::function<void()> callback;
        uint64_t endTime;
        bool repeat;
        uint64_t interval;
    };

    static inline std::vector<TimerData> timers;

    static void SetTimer(const std::function<void()> &callback, int delayMs, bool repeat = false) {
        TimerData t;
        t.callback = callback;
        t.endTime = GetTickCount64() + delayMs;
        t.repeat = repeat;
        t.interval = delayMs;
        timers.push_back(t);
    }

    static void Process() {
        uint64_t now = GetTickCount64();
        for (auto it = timers.begin(); it != timers.end();) {
            if (now >= it->endTime) {
                it->callback();
                if (it->repeat) {
                    it->endTime = now + it->interval;
                    ++it;
                } else {
                    it = timers.erase(it);
                    continue;
                }
            } else {
                ++it;
            }
        }
    }

    IRCPlugin() {
        Events::processScriptsEvent += []() {
            Process();
        };

        Events::initGameEvent += []() {
            SetTimer([]() {
                InitializeIRC();
                irc.StartReceiving();
            }, 2000, false);
        };
    }
} g_ircPlugin;