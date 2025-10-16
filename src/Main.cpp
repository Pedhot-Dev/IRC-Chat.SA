#include "IRCClient.h"
#include "plugin.h"
#include "sampapi/CChat.h"
#include "sampapi/CInput.h"
#include "sampapi/CNetGame.h"
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
            RefChat()->AddMessage(-1, "{FFFF00}Usage: /sendtoirc <message>");
            return;
        }

        irc.SendMessage(params);
        RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + cfg.USERNAME + ": " + std::string(params)).c_str());
    });

    irc.SetOnMessage([](const std::string &msg) {
        if (msg.find("PRIVMSG") != std::string::npos) {
            size_t prefixEnd = msg.find('!');
            size_t msgStart = msg.find(" :");
            if (prefixEnd != std::string::npos && msgStart != std::string::npos) {
                std::string nick = msg.substr(1, prefixEnd - 1);
                std::string text = msg.substr(msgStart + 2);
                RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + nick + ": " + text).c_str());
            }
        }
        else if (msg.find("JOIN") != std::string::npos) {
            size_t start = msg.find(':');
            size_t ex = msg.find('!');
            std::string nick = msg.substr(start + 1, ex - start - 1);
            RefChat()->AddMessage(-1, ("{AAAAFF}[IRC] " + nick + " has joined the chat.").c_str());
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