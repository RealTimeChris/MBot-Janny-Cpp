// OnMessageReceived.hpp - Header for the OnMessageReceived function.
// Mar 18, 2022
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	void onMessageReceived(DiscordCoreAPI::OnMessageCreationData dataPackage) {
		if (dataPackage.message.content.find("discord.gg") != std::string::npos) {
			DeleteMessageData deleteMessageData{};
			deleteMessageData.timeDelay = 0;
			deleteMessageData.reason = "Banned content!";
			deleteMessageData.channelId = dataPackage.message.channelId;
			deleteMessageData.messageId = dataPackage.message.id;
			deleteMessageData.timeStamp = dataPackage.message.timestamp;
			Messages::deleteMessageAsync(deleteMessageData).get();
		}
	}

}