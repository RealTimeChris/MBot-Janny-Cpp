// Avatar.hpp - Header for the getUserImageUrl(UserImageTypes::Avatar) command.
// Nov 10, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"
#include <regex>

namespace DiscordCoreAPI {

	class Avatar : public BaseFunction {
	  public:
		Avatar() {
			this->commandName	  = "getUserImageUrl(UserImageTypes::Avatar)";
			this->helpDescription = "Displays a user's getUserImageUrl(UserImageTypes::Avatar).";
			this->helpEmbed.setTitle("__**Avatar Usage:**__");
			this->helpEmbed.setTimeStamp(getTimeAndDate());
			this->helpEmbed.setColor("FeFeFe");
			this->helpEmbed.setDescription("------\nEnter /getUserImageUrl(UserImageTypes::Avatar) @USERMENTION.\n------");
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Avatar>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };


				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };
				auto newString = argsNew.getCommandArguments().values["user"].value.operator jsonifier::string();
				Snowflake userId{ std::stoull(newString.substr(1, newString.size() - 2).data()) };

				GuildMemberData guildMemberGet{ GuildMembers::getGuildMemberAsync({ .guildMemberId = userId, .guildId = guild.id }).get() };

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription("<@" + guildMemberGet.user.id + ">'s Avatar");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setImage(guildMemberGet.getUserData().getUserImageUrl(UserImageTypes::Avatar) + "?size=4096");
				msgEmbed.setTitle("__**UserData Avatar:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::exception& error) {
				std::cout << "Avatar::execute()" << error.what() << std::endl;
			}
		}
		~Avatar(){};
	};

}// namespace DiscordCoreAPI
