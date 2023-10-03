// HelperFunctions.hpp - Header for some helper functions.
// may 28, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "DatabaseEntities.hpp"

namespace discord_core_api {

	bool checkForBotCommanderStatus(guild_member_data guildMember, discord_user& discordUser) {
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

	bool doWeHaveAdminPermissions(const base_function_arguments& argsNew, input_event_data& eventData, discord_guild discordGuild, channel_data& channel, guild_member_data& guildMember,
		bool isItEphemeral, bool displayResponse = true) {
		respond_to_input_event_data dataPackage{ eventData };
		if (isItEphemeral) {
			dataPackage.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
		} else {
			dataPackage.setResponseType(input_event_response_type::Deferred_Response);
		}
		input_events::respondToInputEventAsync(dataPackage).get();
		bool doWeHaveAdmin = permissions{ guildMember.permissions }.checkForPermission(guildMember, channel, permission::administrator);
		if (doWeHaveAdmin) {
			return true;
		}
		discord_user discordUser(managerAgent, jsonifier::string{ discord_core_client::getInstance()->getBotUser().id.operator jsonifier::string() },
			discord_core_client::getInstance()->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			jsonifier::string msgString = "------\n**sorry, but you don't have the permissions required for that!**\n------";
			embed_data msgEmbed{};
			msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Default_Avatar));
			msgEmbed.setColor("fefefe");
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**permissions issue:**__");
			respond_to_input_event_data dataPackageNew{ eventData };
			dataPackageNew.addMessageEmbed(msgEmbed);
			dataPackageNew.setResponseType(input_event_response_type::Edit_Interaction_Response);
			eventData = input_events::respondToInputEventAsync(dataPackageNew).get();
		}
		return false;
	}
}
