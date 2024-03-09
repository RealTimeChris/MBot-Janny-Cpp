// Avatar.hpp - Header for the getUserImageUrl(UserImageTypes::Avatar) command.
// nov 10, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"
#include <regex>

namespace discord_core_api {

	class avatar : public base_function {
	  public:
		avatar() {
			this->commandName	  = "getUserImageUrl<user_image_types::Avatar>()";
			this->helpDescription = "displays a user's getUserImageUrl<user_image_types::Avatar>().";
			this->helpEmbed.setTitle("__**avatar usage:**__");
			this->helpEmbed.setTimeStamp(getTimeAndDate());
			this->helpEmbed.setColor("fe_fe_fe");
			this->helpEmbed.setDescription("------\nEnter /getUserImageUrl<user_image_types::Avatar>() @usermention.\n------");
		}

		unique_ptr<base_function> create() {
			return makeUnique<avatar>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };


				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember{ argsNew.getGuildMemberData() };
				auto newString = argsNew.getCommandArguments().values["user"].operator jsonifier::string();
				snowflake userId{ jsonifier::strToUint64(newString) };

				guild_member_data guildMemberGet{ guild_members::getGuildMemberAsync({ .guildMemberId = userId, .guildId = guild.id }).get() };

				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription("<@" + guildMemberGet.user.id + ">'s avatar");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setImage(guildMemberGet.getUserData().getUserImageUrl<user_image_types::Avatar>() + "?size=4096");
				msgEmbed.setTitle("__**User Avatar:**__");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = input_events::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::exception& error) {
				std::cout << "avatar::execute()" << error.what() << std::endl;
			}
		}
		~avatar(){};
	};

}// namespace discord_core_api
