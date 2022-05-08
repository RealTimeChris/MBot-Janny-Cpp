// PushMe.hpp - Header for the "PushMe" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class PushMe : public BaseFunction {
	  public:
		PushMe() {
			this->commandName = "pushme";
			this->helpDescription = "Push me to find out!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("Enter /pushme");
			msgEmbed.setTitle("__**PushMe Usage**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<PushMe>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() })
						.get();

				std::string msgString = "------\n__**Push Me!**__\n------";
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**PushMe:**__");
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.addButton(false, "push_me", "Press-Me", ButtonStyle::Success, "✅");
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();

				ButtonCollector buttonCollector{ eventNew };
				auto resultData = buttonCollector.collectButtonData(false, 120000, 1, newArgs.eventData.getAuthorId()).get();
				if (resultData[0].buttonId == "push_me") {
					TimeoutGuildMemberData dataPackage02{};
					dataPackage02.guildId = newArgs.eventData.getGuildId();
					dataPackage02.guildMemberId = newArgs.eventData.getAuthorId();
					dataPackage02.numOfMinutesToTimeoutFor = TimeoutDurations::Five_Minutes;
					dataPackage02.reason = "PRESSED THE BUTTTON!";
					GuildMembers::timeoutGuildMemberAsync(dataPackage02).get();
				}

				RespondToInputEventData dataPackage03{ *resultData[0].interactionData };
				dataPackage03.addMessageEmbed(msgEmbed);
				dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				InputEvents::respondToInputEventAsync(dataPackage03).get();

				return;

			} catch (...) {
				reportException("PushMe::execute()");
			}
		}
		virtual ~PushMe(){};
	};

}