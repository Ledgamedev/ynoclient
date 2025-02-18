#ifndef EP_CHAT_MULTIPLAYER_H
#define EP_CHAT_MULTIPLAYER_H

#include <string>

namespace Chat_Multiplayer {
	void tryCreateChatWindow();
	void gotMessage(std::string name, std::string trip, std::string msg, std::string src);
	void gotInfo(std::string msg);
	void loadPreferences(std::string name, std::string trip);
	void focus();
	void processInputs();
}

#endif