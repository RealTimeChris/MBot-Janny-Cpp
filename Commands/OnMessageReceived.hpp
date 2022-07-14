// OnMessageReceived.hpp - Header for the OnMessageReceived function.
// Mar 18, 2022
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	CoRoutine<void> onMessageReceived(DiscordCoreAPI::OnMessageCreationData dataPackage) {
		co_await NewThreadAwaitable<void>();
		if (dataPackage.message.content.find("discord.gg") != std::string::npos && dataPackage.message.author.id != stoull(std::string("898368255121559634")) &&
			dataPackage.message.author.id != stoull(std::string("936263227417964604"))) {
			DeleteMessageData deleteMessageData{};
			deleteMessageData.timeDelay = 0;
			deleteMessageData.reason = "Banned content!";
			deleteMessageData.channelId = dataPackage.message.channelId;
			deleteMessageData.messageId = dataPackage.message.id;
			deleteMessageData.timeStamp = dataPackage.message.timestamp;
			Messages::deleteMessageAsync(deleteMessageData).get();
		}
		co_return;
	}

}