// HelperFunctions.hpp - Header for some helper functions.
// May 28, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../DatabaseEntities.hpp"

namespace DiscordCoreAPI {

	bool areWeInADM(InputEventData eventData, Channel channel, bool displayResponse = true) {
		auto currentChannelType = channel.type;
		if (currentChannelType == ChannelType::Dm) {
			if (displayResponse) {
				std::string msgString = "------\n**Sorry, but we can't do that in a direct message!**\n------";
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(eventData.getMessageData().interaction.user.userName, eventData.getMessageData().author.avatar);
				msgEmbed.setColor("FEFEFE");
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Direct Message Issue:**__");
				RespondToInputEventData responseData{ eventData };
				responseData.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				responseData.addMessageEmbed(msgEmbed);
				auto event01 = InputEvents::respondToEvent(responseData);
			}
			return true;
		}
		return false;
	}

	bool checkForBotCommanderStatus(GuildMember guildMember, DiscordUser& discordUser) {
		bool areWeACommander;
		for (auto& value: discordUser.data.botCommanders) {
			if (guildMember.user.id == value) {
				areWeACommander = true;
				return areWeACommander;
				break;
			}
		}
		return false;
	}

	bool doWeHaveAdminPermissions(BaseFunctionArguments newArgs, InputEventData eventData, DiscordGuild discordGuild, Channel channel, GuildMember guildMember,
		bool displayResponse = true) {
		bool doWeHaveAdmin = guildMember.permissions.checkForPermission(guildMember, channel, Permission::Administrator);

		if (doWeHaveAdmin) {
			return true;
		}
		DiscordCoreAPI::DiscordUser discordUser(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			std::string msgString = "------\n**Sorry, but you don't have the permissions required for that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(guildMember.user.userName, guildMember.user.avatar);
			msgEmbed.setColor(discordGuild.data.borderColor);
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**Permissions Issue:**__");
			RespondToInputEventData dataPackage{ eventData };
			dataPackage.addMessageEmbed(msgEmbed);
			dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
			InputEvents::respondToEvent(dataPackage);
		}
		return false;
	}
}
