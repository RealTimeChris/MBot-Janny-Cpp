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

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				Guild guild = Guilds::getCachedGuildAsync({ newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };
				GetGuildMemberData dataPackage00{};
				dataPackage00.guildMemberId = newArgs.eventData.getAuthorId();
				dataPackage00.guildId = guild.id;
				GuildMember sendingGuildMember = GuildMembers::getCachedGuildMemberAsync(dataPackage00).get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, sendingGuildMember);
				RespondToInputEventData dataPackage{ std::ref(newArgs.eventData) };
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				if (!doWeHaveAdminPerms) {
					return;
				}
				std::string whatAreWeDoing;
				std::regex userMentionRegex("<@\\d{18}>");
				std::regex userIdRegexp("\\d{18}");
				std::string ghostReason;
				uint64_t userId{};
				if (newArgs.commandData.subCommandName == "view") {
					whatAreWeDoing = "viewing";
					userId = newArgs.eventData.getAuthorId();
				}
				if (newArgs.commandData.optionsArgs.size() > 0 && newArgs.commandData.subCommandName == "add") {
					whatAreWeDoing = "add";
					std::string argOne = newArgs.commandData.optionsArgs[0];
					std::string argTwo = newArgs.commandData.optionsArgs[1];
					ghostReason = argTwo;
					std::cmatch userIDMatch;
					std::regex_search(argOne.c_str(), userIDMatch, userIdRegexp);
					std::string userIDOne = userIDMatch.str();
					userId = stoull(userIDOne);
				} else if (newArgs.commandData.optionsArgs.size() > 0 && newArgs.commandData.subCommandName == "remove") {
					whatAreWeDoing = "remove";
					std::string argOne = newArgs.commandData.optionsArgs[0];
					std::cmatch userIDMatch;
					std::regex_search(argOne.c_str(), userIDMatch, userIdRegexp);
					std::string userIDOne = userIDMatch.str();
					userId = stoull(userIDOne);
				}
				InputEventData newEvent01 = newEvent;

				GuildMember targetGuildMember = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = userId, .guildId = guild.id }).get();
				DiscordGuildMember discordGuildMember(targetGuildMember);
				if (whatAreWeDoing == "add") {
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::Week;
					modifyData.guildId = guild.id;
					modifyData.guildMemberId = targetGuildMember.id;
					modifyData.reason = ghostReason;
					targetGuildMember = GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					std::string msgString = "------\n**Hello! You've been REDACTED, on the server " + guild.name + " for the following reason(s): " + ghostReason +
						"\n Please, contact a moderator or admin to clear this issue up! Thanks!**\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**You\'ve been ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					if (targetGuildMember.id == 0) {
						std::string msgString = "------\n**Hello! There was an error while trying to ghost <@" + std::to_string(userId) + ">**\n------\n";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Ghosting Error:**__");
						RespondToInputEventData dataPackage(newEvent01);
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.ghostedIds.push_back(targetGuildMember.id);
					discordGuild.writeDataToDB();

					std::string msgString2 =
						"------\n**Hello! You've ghosted the following member:** <@" + std::to_string(targetGuildMember.id) + "> (" + targetGuildMember.userName + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.userName, sendingGuildMember.userAvatar);
					msgEmbed2.setColor(discordGuild.data.borderColor);
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**New Server Member Ghosted:**__");
					RespondToInputEventData dataPackage02(newEvent01);
					dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get();
				} else if (whatAreWeDoing == "viewing") {
					std::string msgString = "------\n";

					for (auto& value: discordGuild.data.ghostedIds) {
						msgString += "<@" + std::to_string(value) + ">\n";
					}

					msgString += "------";

					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Currently Ghosted Members:**__");
					RespondToInputEventData dataPackage(newEvent01);
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (whatAreWeDoing == "remove") {
					bool isItThere{ false };
					int32_t index{ 0 };
					for (uint32_t x = 0; x < discordGuild.data.ghostedIds.size(); x += 1) {
						if (discordGuild.data.ghostedIds[x] == targetGuildMember.id) {
							isItThere = true;
							index = x;
							break;
						}
					}
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::None;
					modifyData.guildId = guild.id;
					modifyData.reason = ghostReason;
					modifyData.guildMemberId = targetGuildMember.id;
					targetGuildMember = GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					if (targetGuildMember.id == 0 || !isItThere) {
						std::string msgString = "------\n**Hello! There was an error while trying to un-ghost <@" + std::to_string(userId) + ">**\n------\n";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Un-Ghosting Error:**__");
						RespondToInputEventData dataPackage(newEvent01);
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.ghostedIds.erase(discordGuild.data.ghostedIds.begin() + index);
					discordGuild.writeDataToDB();
					std::string msgString = "------\n**Hello! You\'ve had your redacted status removed! Have a great day!**\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**You\'ve been un-ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					std::string msgString2 =
						"------\n**Hello! You've un-ghosted the following member:** <@" + std::to_string(targetGuildMember.id) + "> (" + targetGuildMember.userName + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.userName, sendingGuildMember.userAvatar);
					msgEmbed2.setColor(discordGuild.data.borderColor);
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**New Server Member Un-Ghosted:**__");
					RespondToInputEventData dataPackage02(newEvent01);
					dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get();
					return;
				}
				return;
			} catch (...) {
				reportException("Ghost::execute()");
			}
		}
		~Ghost(){};
	};
}