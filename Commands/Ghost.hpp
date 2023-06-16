// Ghost.hpp - Header for the "ghost" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Ghost : public BaseFunction {
	  public:
		Ghost() {
			this->commandName	  = "ghost";
			this->helpDescription = "'Ghosts' or 'unghosts' a server memeber.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ghost add, REASON, @USERMENTION or /ghost remove, @USERMENTION, /ghost view.\n------");
			msgEmbed.setTitle("__**Ghost Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Ghost>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData sendingGuildMember = argsNew.getGuildMemberData();

				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, sendingGuildMember, true);
				auto argsNewer				 = argsNew.getInputEventData();
				if (!doWeHaveAdminPermission) {
					return;
				}
				jsonifier::string whatAreWeDoing{};
				jsonifier::string ghostReason{};
				Snowflake userId{};
				if (argsNew.getSubCommandName() == "view") {
					whatAreWeDoing = "viewing";
					userId		   = sendingGuildMember.user.id;
				}
				if (argsNew.getCommandArguments().values.size() > 0 && argsNew.getSubCommandName() == "add") {
					whatAreWeDoing		  = "add";
					jsonifier::string argOne	  = argsNew.getCommandArguments().values["user"].value;
					jsonifier::string argTwo	  = argsNew.getCommandArguments().values["reason"].value;
					ghostReason			  = argTwo;
					jsonifier::string userIDOne = argOne;
					userId						  = std::stoull(userIDOne.data());
				} else if (argsNew.getCommandArguments().values.size() > 0 && argsNew.getSubCommandName() == "remove") {
					whatAreWeDoing		  = "remove";
					jsonifier::string argOne	  = argsNew.getCommandArguments().values["user"].value;
					jsonifier::string userIDOne = argOne;
					userId						  = std::stoull(userIDOne.data());
				}
				InputEventData newEvent01 = argsNewer;

				GuildMemberData targetGuildMember = GuildMembers::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
				DiscordGuildMember discordGuildMember(managerAgent, targetGuildMember);
				if (whatAreWeDoing == "add") {
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::Week;
					modifyData.guildId					= guild.id;
					modifyData.guildMemberId			= targetGuildMember.user.id;
					modifyData.reason					= ghostReason;
					targetGuildMember					= GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					jsonifier::string msgString = "------\n**Hello! You've been REDACTED, on the server " + jsonifier::string{ guild.name } + " for the following reason(s): " + ghostReason +
						"\n Please, contact a moderator or admin to clear this issue up! Thanks!**\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(DiscordCoreClient::getInstance()->getBotUser().userName, DiscordCoreClient::getInstance()->getBotUser().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**You\'ve been ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					if (targetGuildMember.user.id == 0) {
						jsonifier::string msgStringNew = "------\n**Hello! There was an error while trying to ghost <@" + userId + ">**\n------\n";
						UniquePtr<EmbedData> msgEmbedNew{ makeUnique<EmbedData>() };
						msgEmbedNew->setAuthor(DiscordCoreClient::getInstance()->getBotUser().userName, DiscordCoreClient::getInstance()->getBotUser().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbedNew->setColor(discordGuild.data.borderColor);
						msgEmbedNew->setDescription(msgStringNew);
						msgEmbedNew->setTimeStamp(getTimeAndDate());
						msgEmbedNew->setTitle("__**Ghosting Error:**__");
						RespondToInputEventData dataPackage01(newEvent01);
						dataPackage01.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage01.addMessageEmbed(*msgEmbedNew);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage01).get();
						return;
					}

					discordGuild.data.ghostedIds.emplace_back(targetGuildMember.user.id);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString2 = "------\n**Hello! You've ghosted the following member:** <@" + targetGuildMember.user.id + "> (" +
						jsonifier::string{ targetGuildMember.getUserData().userName } + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.getUserData().userName, sendingGuildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
					msgEmbed2.setColor(discordGuild.data.borderColor);
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**New Server Member Ghosted:**__");
					RespondToInputEventData dataPackage02(newEvent01);
					dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get();
				} else if (whatAreWeDoing == "viewing") {
					jsonifier::string msgString = "------\n";

					for (auto& value: discordGuild.data.ghostedIds) {
						msgString += "<@" + value + ">\n";
					}

					msgString += "------";

					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Currently Ghosted Members:**__");
					RespondToInputEventData dataPackage01(newEvent01);
					dataPackage01.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage01.addMessageEmbed(*msgEmbed);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage01).get();
					return;
				} else if (whatAreWeDoing == "remove") {
					bool isItThere{ false };
					int32_t index{ 0 };
					for (uint32_t x = 0; x < discordGuild.data.ghostedIds.size(); x += 1) {
						if (discordGuild.data.ghostedIds[x] == targetGuildMember.user.id) {
							isItThere = true;
							index	  = x;
							break;
						}
					}
					TimeoutGuildMemberData modifyData{};
					modifyData.numOfMinutesToTimeoutFor = TimeoutDurations::None;
					modifyData.guildId					= guild.id;
					modifyData.reason					= ghostReason;
					modifyData.guildMemberId			= targetGuildMember.user.id;
					targetGuildMember					= GuildMembers::timeoutGuildMemberAsync(modifyData).get();

					if (targetGuildMember.user.id == 0 || !isItThere) {
						jsonifier::string msgString = "------\n**Hello! There was an error while trying to un-ghost <@" + userId + ">**\n------\n";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(DiscordCoreClient::getInstance()->getBotUser().userName, DiscordCoreClient::getInstance()->getBotUser().getUserImageUrl(UserImageTypes::Avatar));
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
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString = "------\n**Hello! You\'ve had your redacted status removed! Have a great day!**\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(DiscordCoreClient::getInstance()->getBotUser().userName, DiscordCoreClient::getInstance()->getBotUser().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**You\'ve been un-ghosted:**__");

					auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
					CreateMessageData dataPackage{ DmChannel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					Messages::createMessageAsync(dataPackage).get();

					jsonifier::string msgString2 = "------\n**Hello! You've un-ghosted the following member:** <@" + targetGuildMember.user.id + "> (" +
						jsonifier::string{ targetGuildMember.getUserData().userName } + ")\n------";
					EmbedData msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.getUserData().userName, sendingGuildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
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
			} catch (const std::exception& error) {
				std::cout << "Ghost::execute()" << error.what() << std::endl;
			}
		}
		~Ghost(){};
	};
}// namespace DiscordCoreAPI
