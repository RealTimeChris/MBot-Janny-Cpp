// Avatar.hpp - Header for the avatar command.
// Nov 10, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"
#include <regex>

namespace DiscordCoreAPI {

	class Avatar : public BaseFunction {
	  public:
		Avatar() {
			this->commandName = "avatar";
			this->helpDescription = "Displays a user's avatar.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /avatar @USERMENTION.\n------");
			msgEmbed.setTitle("__**Avatar Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Avatar>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() }).get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				uint64_t userId{ 0 };
				std::regex userIdRegex("\\d{18}");
				std::cmatch userIDMatch{};
				std::regex_search(newArgs.optionsArgs[0].c_str(), userIDMatch, userIdRegex);
				std::string userIDOne = userIDMatch.str();
				userId = stoull(userIDOne);

				GuildMember guildMemberGet = GuildMembers::getGuildMemberAsync({ .guildMemberId = userId, .guildId = newArgs.eventData.getGuildId() }).get();

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription("<@" + std::to_string(guildMemberGet.id) + ">'s Avatar");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setImage(guildMemberGet.userAvatar);
				msgEmbed.setTitle("__**User Avatar:**__");
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (...) {
				reportException("Avatar::execute()");
			}
		}
		~Avatar(){};
	};

}
