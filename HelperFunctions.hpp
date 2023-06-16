// HelperFunctions.hpp - Header for some helper functions.
// May 28, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "DatabaseEntities.hpp"

namespace DiscordCoreAPI {

	bool checkForBotCommanderStatus(GuildMemberData guildMember, DiscordUser& discordUser) {
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

	bool doWeHaveAdminPermissions(BaseFunctionArguments& argsNew, InputEventData& eventData, DiscordGuild discordGuild, ChannelData& channel, GuildMemberData& guildMember,
		bool isItEphemeral, bool displayResponse = true) {
		RespondToInputEventData dataPackage{ eventData };
		if (isItEphemeral) {
			dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
		} else {
			dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
		}
		InputEvents::respondToInputEventAsync(dataPackage).get();
		bool doWeHaveAdmin = Permissions{ guildMember.permissions }.checkForPermission(guildMember, channel, Permission::Administrator);
		if (doWeHaveAdmin) {
			return true;
		}
		DiscordUser discordUser(managerAgent, DiscordCoreClient::getInstance()->getBotUser().userName, DiscordCoreClient::getInstance()->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			jsonifier::string msgString = "------\n**Sorry, but you don't have the permissions required for that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Default_Avatar));
			msgEmbed.setColor(discordGuild.data.borderColor);
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**Permissions Issue:**__");
			RespondToInputEventData dataPackageNew{ eventData };
			dataPackageNew.addMessageEmbed(msgEmbed);
			dataPackageNew.setResponseType(InputEventResponseType::Edit_Interaction_Response);
			eventData = InputEvents::respondToInputEventAsync(dataPackageNew).get();
		}
		return false;
	}
}
