// Ghost.hpp - Header for the "ghost" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Ghost : public BaseFunction {
	  public:
		Ghost() {
			this->commandName = "ghost";
			this->helpDescription = "'Ghosts' or 'unghosts' a server memeber.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ghost add, REASON, @USERMENTION or /ghost remove, @USERMENTION, /ghost view.\n------");
			msgEmbed.setTitle("__**Ghost Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Ghost>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ newArgs.eventData->getChannelId() }).get();
				bool areWeInADm = areWeInADM(*newArgs.eventData, channel);
				if (areWeInADm) {
					return;
				}

				InputEvents::deleteInputEventResponseAsync(std::make_unique<InputEventData>(*newArgs.eventData)).get();

				Guild guild = Guilds::getCachedGuildAsync({ newArgs.eventData->getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember sendingGuildMember =
					GuildMembers::getGuildMemberAsync({ .guildMemberId = newArgs.eventData->getAuthorId(), .guildId = newArgs.eventData->getGuildId() }).get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, *newArgs.eventData, discordGuild, channel, sendingGuildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				std::string whatAreWeDoing;
				std::regex userMentionRegex("<@!\\d{18}>");
				std::regex userIdRegexp("\\d{18}");
				std::string ghostReason;
				std::string userId;
				if (newArgs.commandData.subCommandName == "view") {
					whatAreWeDoing = "viewing";
					userId = newArgs.eventData->getAuthorId();
				}
				if (newArgs.commandData.optionsArgs.size() > 0 && newArgs.commandData.subCommandName == "add") {
					whatAreWeDoing = "add";
					std::string argOne = newArgs.commandData.optionsArgs[0];
					std::string argTwo = newArgs.commandData.optionsArgs[1];
					ghostReason = argTwo;
					std::cmatch userIDMatch;
					std::regex_search(argOne.c_str(), userIDMatch, userIdRegexp);
					std::string userIDOne = userIDMatch.str();
					userId = userIDOne;
				} else if (newArgs.commandData.optionsArgs.size() > 0 && newArgs.commandData.subCommandName == "remove") {
					whatAreWeDoing = "remove";
					std::string argOne = newArgs.commandData.optionsArgs[0];
					std::cmatch userIDMatch;
					std::regex_search(argOne.c_str(), userIDMatch, userIdRegexp);
					std::string userIDOne = userIDMatch.str();
					userId = userIDOne;
				}

				InputEventData newEvent01 = *newArgs.eventData;

				GuildMember targetGuildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = userId, .guildId = newArgs.eventData->getGuildId() }).get();
				DiscordGuildMember discordGuildMember(targetGuildMember);

				if (whatAreWeDoing == "add") {
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::Week;
					modifyData.guildId = newArgs.eventData->getGuildId();
					modifyData.guildMemberId = targetGuildMember.user.id;
					modifyData.reason = ghostReason;
					targetGuildMember = GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					if (targetGuildMember.user.id == "") {
						std::string msgString = "------\n**Hello! There was an error while trying to ghost <@!" + userId + ">**\n------\n";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setDescription(msgString);
						msgEmbed.setTimeStamp(getTimeAndDate());
						msgEmbed.setTitle("__**Ghosting Error:**__");
						RespondToInputEventData dataPackage(newEvent01);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						auto eventNew = InputEvents::respondToEvent(dataPackage);
						return;
					}

					discordGuild.data.ghostedIds.push_back(targetGuildMember.user.id);
					discordGuild.writeDataToDB();

					std::string msgString = "------\n**Hello! You've been REDACTED, on the server " + guild.name +
						" for the following reason(s): " + ghostReason + "\n Please, contact a moderator or admin to clear this issue up! Thanks!**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**You\'ve been ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					std::string msgString2 = "------\n**Hello! You've ghosted the following member:** <@!" + targetGuildMember.user.id + "> (" +
						targetGuildMember.user.userName + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.user.userName, sendingGuildMember.user.avatar);
					msgEmbed2.setColor(discordGuild.data.borderColor);
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**New Server Member Ghosted:**__");
					RespondToInputEventData dataPackage02(newEvent01);
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = InputEvents::respondToEvent(dataPackage02);
				} else if (whatAreWeDoing == "viewing") {
					std::string msgString = "------\n";

					for (auto& value: discordGuild.data.ghostedIds) {
						msgString += "<@!" + value + ">\n";
					}

					msgString += "------";

					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.eventData->getUserName(), newArgs.eventData->getAvatarUrl());
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Currently Ghosted Members:**__");
					RespondToInputEventData dataPackage(newEvent01);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto eventNew = InputEvents::respondToEvent(dataPackage);
					return;
				} else if (whatAreWeDoing == "remove") {
					bool isItThere{ false };
					int32_t index{ 0 };
					for (uint32_t x = 0; x < discordGuild.data.ghostedIds.size(); x += 1) {
						if (discordGuild.data.ghostedIds[x] == targetGuildMember.user.id) {
							isItThere = true;
							index = x;
							break;
						}
					}
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::None;
					modifyData.guildId = newArgs.eventData->getGuildId();
					modifyData.reason = ghostReason;
					modifyData.guildMemberId = targetGuildMember.user.id;
					targetGuildMember = GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					if (targetGuildMember.user.id == "" || !isItThere) {
						std::string msgString = "------\n**Hello! There was an error while trying to un-ghost <@!" + userId + ">**\n------\n";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setDescription(msgString);
						msgEmbed.setTimeStamp(getTimeAndDate());
						msgEmbed.setTitle("__**Un-Ghosting Error:**__");
						RespondToInputEventData dataPackage(newEvent01);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						auto eventNew = InputEvents::respondToEvent(dataPackage);
						return;
					}

					discordGuild.data.ghostedIds.erase(discordGuild.data.ghostedIds.begin() + index);
					discordGuild.writeDataToDB();
					std::string msgString = "------\n**Hello! You\'ve had your redacted status removed! Have a great day!**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**You\'ve been un-ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					std::string msgString2 = "------\n**Hello! You've un-ghosted the following member:** <@!" + targetGuildMember.user.id + "> (" +
						targetGuildMember.user.userName + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.user.userName, sendingGuildMember.user.avatar);
					msgEmbed2.setColor(discordGuild.data.borderColor);
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**New Server Member Un-Ghosted:**__");
					RespondToInputEventData dataPackage02(newEvent01);
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = InputEvents::respondToEvent(dataPackage02);
					return;
				}
				return;
			} catch (...) {
				reportException("Ghost::execute()");
			}
		}
		virtual ~Ghost(){};
	};
}