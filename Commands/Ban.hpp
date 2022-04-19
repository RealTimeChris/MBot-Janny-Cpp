// Ban.hpp - Header for the "ban" command.
// Aug 7, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Ban : public BaseFunction {
	  public:
		Ban() {
			this->commandName = "ban";
			this->helpDescription = "Bans a user from the current server.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ban @USERMENTION, REASON, #OFDAYSTOPURGEMESSAGESOFUSER.\n------");
			msgEmbed.setTitle("__**Ban Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Ban>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData->getChannelId() }).get();

				if (areWeInADM(*newArgs.eventData, channel)) {
					return;
				}

				InputEvents::deleteInputEventResponseAsync(std::make_unique<InputEventData>(*newArgs.eventData)).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData->getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData->getAuthorId(), .guildId = newArgs.eventData->getGuildId() })
						.get();

				if (!doWeHaveAdminPermissions(newArgs, *newArgs.eventData, discordGuild, channel, guildMember)) {
					return;
				}

				std::unique_ptr<InputEventData> newEvent = std::make_unique<InputEventData>(*newArgs.eventData);
				std::regex userIdRegex("\\d{18}");
				std::regex digitDaysRegex("\\d{1}");
				std::string userId;
				std::string reason;
				std::string whatAreWeDoing;
				int32_t daysDigit = 0;

				if (newArgs.commandData.optionsArgs.size() > 2) {
					reason = newArgs.commandData.optionsArgs[2];
				}
				if (newArgs.commandData.optionsArgs.size() > 3) {
					if (!std::regex_search(newArgs.commandData.optionsArgs[3], digitDaysRegex) || std::stoll(newArgs.commandData.optionsArgs[3]) > 7 ||
						std::stoll(newArgs.commandData.optionsArgs[3]) < 0) {
						std::string msgString = "------\n**Please, enter a proper number of days for purging the user's messages (0-7) (!ban = @USERMENTION, "
												"REASON, #OFDAYSTOPURGE)**\n------";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
						msgEmbed.setDescription(msgString);
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setTitle("__**Missing or Invalid Parameters:**__");
						msgEmbed.setTimeStamp(getTimeAndDate());
						RespondToInputEventData dataPackage(*newEvent);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						newEvent = InputEvents::respondToEvent(dataPackage);
						return;
					} else {
						std::cmatch dayDigitMatch;
						std::regex_search(newArgs.commandData.optionsArgs[3].c_str(), dayDigitMatch, digitDaysRegex);
						daysDigit = stol(dayDigitMatch.str());
					}
				}
				whatAreWeDoing = newArgs.commandData.subCommandName;
				if (whatAreWeDoing == "add") {
					GuildMember guildMember01 =
						GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = userId, .guildId = newArgs.eventData->getGuildId() }).get();
					GuildMember botGuildMember = GuildMembers::getCachedGuildMemberAsync(
						{ .guildMemberId = newArgs.discordCoreClient->getBotUser().id, .guildId = newArgs.eventData->getGuildId() })
													 .get();
					auto userRoles = Roles::getGuildMemberRolesAsync({ .guildMember = guildMember01, .guildId = newArgs.eventData->getGuildId() }).get();
					int32_t highestUserRolePosition{ 0 };

					for (auto& value: userRoles) {
						if (value.position > highestUserRolePosition) {
							highestUserRolePosition = value.position;
						}
					}

					int32_t highestBotRolePosition{ 0 };
					auto botRoles = Roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = newArgs.eventData->getGuildId() }).get();
					for (auto& value: botRoles) {
						if (value.position > highestBotRolePosition) {
							highestBotRolePosition = value.position;
						}
					}
					if (highestUserRolePosition >= highestBotRolePosition) {
						std::string msgString = "------\n**Sorry, but I cannot ban them as their highest role is higher than mine!**\n------";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(newArgs.eventData->getUserName(), newArgs.eventData->getAvatarUrl());
						msgEmbed.setDescription(msgString);
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setTitle("__**Missing or Invalid Parameters:**__");
						msgEmbed.setTimeStamp(getTimeAndDate());
						RespondToInputEventData dataPackage(*newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						newEvent = InputEvents::respondToEvent(dataPackage);
						return;
					}

					CreateGuildBanData dataPackageNew;
					dataPackageNew.deleteMessageDays = daysDigit;
					dataPackageNew.guildId = newEvent->getGuildId();
					dataPackageNew.reason = reason;
					dataPackageNew.guildMemberId = userId;

					if (reason != "") {
						std::string msgString01 = "------\n**Hello! You have been REDACTED from the server " + guild.name + " for the following reason:** " +
							reason + "\nHave a great day!\n------";
						EmbedData msgEmbed01;
						msgEmbed01.setAuthor(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().avatar);
						msgEmbed01.setTimeStamp(getTimeAndDate());
						msgEmbed01.setDescription(msgString01);
						msgEmbed01.setColor(discordGuild.data.borderColor);
						msgEmbed01.setTitle("__**You've been REDACTED:**__");
						auto DmChannel = Channels::createDMChannelAsync({ .userId = userId }).get();
						CreateMessageData dataPackage{ DmChannel.id };
						dataPackage.addMessageEmbed(msgEmbed01);
						Messages::createMessageAsync(dataPackage).get();
					}

					Guilds::createGuildBanAsync(dataPackageNew).get();
					discordGuild.getDataFromDB();
					bool areWeFound = false;
					for (auto& value: discordGuild.data.userBanInfo) {
						if (newEvent->getAuthorId() == value.userId) {
							areWeFound = true;
						}
					}
					UserBanInfo userBanInfo;
					userBanInfo.userId = newEvent->getAuthorId();
					if (!areWeFound) {
						discordGuild.data.userBanInfo.push_back(userBanInfo);
					}
					for (uint32_t x = 0; x < discordGuild.data.userBanInfo.size(); x += 1) {
						if (discordGuild.data.userBanInfo[x].userId == newEvent->getAuthorId()) {
							BanInfoLite newData;
							auto guildMemberNew = Users::getUserAsync({ .userId = userId }).get();
							for (auto& value: discordGuild.data.userBanInfo[x].userBans) {
								if (value.userId == guildMemberNew.id) {
									discordGuild.data.userBanInfo.erase(discordGuild.data.userBanInfo.begin() + x);
								}
							}
							newData.userId = userId;
							newData.userName = guildMemberNew.userName;
							newData.avatarUrl = guildMemberNew.avatar;
							newData.bannedAt = getTimeAndDate();
							newData.reason = reason;
							discordGuild.data.userBanInfo[x].userBans.push_back(newData);
							discordGuild.data.userBanInfo[x].userId = newEvent->getAuthorId();
							discordGuild.writeDataToDB();

							std::string msgString = "------\n**Nicely done! You owned that motherfucker " + guildMemberNew.userName +
								" good!**\nYour current ban count is: " + std::to_string(discordGuild.data.userBanInfo[x].userBans.size()) + "\n------";
							EmbedData msgEmbed{};
							msgEmbed.setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
							msgEmbed.setDescription(msgString);
							msgEmbed.setColor(discordGuild.data.borderColor);
							msgEmbed.setTitle("__**Succesful Ban:**__");
							msgEmbed.setTimeStamp(getTimeAndDate());
							RespondToInputEventData dataPackage03(*newEvent);
							dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage03.addMessageEmbed(msgEmbed);
							newEvent = InputEvents::respondToEvent(dataPackage03);
							return;
						}
					}
				}

				else if (whatAreWeDoing == "view") {
					uint32_t maxIdx = 0;
					uint32_t len = ( uint32_t )discordGuild.data.userBanInfo.size();
					for (uint32_t x = 0; x < len; x += 1) {
						maxIdx = x;
						for (uint32_t y = x + 1; y < len; y += 1) {
							if (discordGuild.data.userBanInfo.at(y).userBans.size() > discordGuild.data.userBanInfo.at(maxIdx).userBans.size()) {
								maxIdx = y;
							}
						}
						UserBanInfo tempMember = discordGuild.data.userBanInfo.at(x);
						discordGuild.data.userBanInfo.at(x) = discordGuild.data.userBanInfo.at(maxIdx);
						discordGuild.data.userBanInfo.at(maxIdx) = tempMember;
					}
					discordGuild.writeDataToDB();
					uint32_t membersPerPage = 20;
					uint32_t totalPageCount = 0;
					if (discordGuild.data.userBanInfo.size() % membersPerPage > 0) {
						totalPageCount = ( uint32_t )trunc(discordGuild.data.userBanInfo.size() / membersPerPage) + 1;
					} else {
						totalPageCount = ( uint32_t )trunc(discordGuild.data.userBanInfo.size() / membersPerPage);
					}
					uint32_t currentPage = 0;
					std::vector<EmbedData> pageEmbeds;
					std::vector<std::string> pageStrings;
					for (int32_t x = 0; x < discordGuild.data.userBanInfo.size(); x += 1) {
						if (x % membersPerPage == 0) {
							pageEmbeds.push_back(EmbedData());
							pageStrings.push_back("------\n__**Enter 'Select' to view your trophy case!**__\n");
						}

						std::string msgString = "__**#" + std::to_string(currentPage * membersPerPage + ((x % membersPerPage) + 1)) + " | Name:**__ <@!" +
							discordGuild.data.userBanInfo[x].userId + "> __**| Ban Count:**__ " +
							std::to_string(discordGuild.data.userBanInfo[x].userBans.size()) + "\n";

						pageStrings[currentPage] += msgString;
						if (x % membersPerPage == membersPerPage - 1 || x == discordGuild.data.userBanInfo.size() - 1) {
							pageStrings[currentPage] += "------";
							pageEmbeds[currentPage].setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
							pageEmbeds[currentPage].setDescription(pageStrings[currentPage]);
							pageEmbeds[currentPage].setTimeStamp(getTimeAndDate());
							pageEmbeds[currentPage].setTitle(
								"__**Ban Leaderboard (Page " + std::to_string(currentPage + 1) + " of " + std::to_string(totalPageCount) + ")**__");
							pageEmbeds[currentPage].setColor(discordGuild.data.borderColor);
							currentPage += 1;
						}
					}
					if (pageEmbeds.size() == 0) {
						std::string msgString = "------\n**Sorry, but noone has banned anyone yet!**\n------";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
						msgEmbed.setDescription(msgString);
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setTitle("__**No Bans:**__");
						msgEmbed.setTimeStamp(getTimeAndDate());
						pageEmbeds.push_back(msgEmbed);
					}

					currentPage = 0;
					RespondToInputEventData dataPackage(*newEvent);
					dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
					newEvent = InputEvents::respondToEvent(dataPackage);
					auto returnValue = moveThroughMessagePages(
						newEvent->getRequesterId(), std::make_unique<InputEventData>(*newEvent), currentPage, pageEmbeds, false, 120000, true);
					if (returnValue.buttonId == "exit" || returnValue.buttonId == "empty") {
						InputEvents::deleteInputEventResponseAsync(std::make_unique<InputEventData>(returnValue.inputEventData));
						return;
					}
					if (returnValue.buttonId == "select") {
						std::vector<EmbedData> msgEmbeds;
						for (auto& value: discordGuild.data.userBanInfo) {
							if (value.userId == newArgs.eventData->getAuthorId()) {
								for (auto& value2: value.userBans) {
									EmbedData newEmbed;
									newEmbed.setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
									newEmbed.setDescription(
										"------\n__**Date Banned:**__ " + value2.bannedAt + "\n__**Reason:**__ " + value2.reason + "\n------");
									newEmbed.setTimeStamp(getTimeAndDate());
									newEmbed.setTitle("__**Banned User: " + value2.userName + "**__");
									newEmbed.setImage(value2.avatarUrl);
									newEmbed.setColor(discordGuild.data.borderColor);
									msgEmbeds.push_back(newEmbed);
								}
							}
						}
						if (msgEmbeds.size() == 0) {
							EmbedData newEmbed;
							newEmbed.setAuthor(newEvent->getUserName(), newEvent->getAvatarUrl());
							newEmbed.setDescription("------\n__**You have not banned anyone yet!**__\n------");
							newEmbed.setTimeStamp(getTimeAndDate());
							newEmbed.setTitle("__**Banned Users:**__");
							newEmbed.setTimeStamp(getTimeAndDate());
							newEmbed.setColor(discordGuild.data.borderColor);
							msgEmbeds.push_back(newEmbed);
						}
						int32_t currentPageIndex02 = 0;
						moveThroughMessagePages(newEvent->getRequesterId(), std::make_unique<InputEventData>(returnValue.inputEventData), currentPageIndex02,
							msgEmbeds, true, 120000, true);
					}
				}
				return;
			} catch (...) {
				reportException("Ban::execute()");
			}
		}
		virtual ~Ban(){};
	};

};
