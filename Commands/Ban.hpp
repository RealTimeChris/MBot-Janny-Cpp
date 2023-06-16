// Ban.hpp - Header for the "ban" command.
// Aug 7, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Ban : public BaseFunction {
	  public:
		Ban() {
			this->commandName	  = "ban";
			this->helpDescription = "Bans a user from the current server.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ban @USERMENTION, REASON, #OFDAYSTOPURGEMESSAGESOFUSER.\n------");
			msgEmbed.setTitle("__**Ban Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Ban>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };


				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, false);
				if (!doWeHaveAdminPermission) {
					return;
				}

				InputEventData argsNewer = argsNew.getInputEventData();
				Snowflake userId{};
				jsonifier::string reason;
				jsonifier::string whatAreWeDoing;
				int32_t daysDigit = 0;

				if (argsNew.getCommandArguments().values.size() > 2 && argsNew.getCommandArguments().values.contains("reason")) {
					reason = argsNew.getCommandArguments().values["reason"].value;
				}
				if (argsNew.getCommandArguments().values.size() > 3) {
					if (std::stoull(argsNew.getCommandArguments().values["numberofdaystopurge"].value) > 7 ||
						std::stoull(argsNew.getCommandArguments().values["numberofdaystopurge"].value) < 0) {
						jsonifier::string msgString = "------\n**Please, enter a proper number of days for purging the user's messages (0-7) (!ban = @USERMENTION, "
												"REASON, #OFDAYSTOPURGE)**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**Missing or Invalid Parameters:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					} else {
						daysDigit = std::stol(argsNew.getCommandArguments().values["numberofdaystopurge"].value);
					}
				}
				whatAreWeDoing = argsNew.getSubCommandName();
				if (whatAreWeDoing == "add") {
					GuildMemberData guildMember01  = GuildMembers::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
					GuildMemberData botGuildMember = GuildMembers::getCachedGuildMember({ .guildMemberId = DiscordCoreClient::getInstance()->getBotUser().id, .guildId = guild.id });
					auto userRoles				   = Roles::getGuildMemberRolesAsync({ .guildMember = guildMember01, .guildId = guild.id }).get();
					int32_t highestUserRolePosition{ 0 };

					for (auto& value: userRoles) {
						if (value.position > highestUserRolePosition) {
							highestUserRolePosition = value.position;
						}
					}

					int32_t highestBotRolePosition{ 0 };
					auto botRoles = Roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = guild.id }).get();
					for (auto& value: botRoles) {
						if (value.position > highestBotRolePosition) {
							highestBotRolePosition = value.position;
						}
					}
					if (highestUserRolePosition >= highestBotRolePosition) {
						jsonifier::string msgString = "------\n**Sorry, but I cannot ban them as their highest role is higher than mine!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**Missing or Invalid Parameters:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}

					CreateGuildBanData dataPackageNew{};
					dataPackageNew.deleteMessageDays = daysDigit;
					dataPackageNew.guildId			 = guild.id;

					dataPackageNew.reason		 = reason;
					dataPackageNew.guildMemberId = userId;

					Guilds::createGuildBanAsync(dataPackageNew).get();
					discordGuild.getDataFromDB(managerAgent);
					bool areWeFound = false;
					for (auto& value: discordGuild.data.userBanInfo) {
						if (guildMember.user.id == value.userId) {
							areWeFound = true;
						}
					}
					UserBanInfo userBanInfo;
					userBanInfo.userId = guildMember.user.id;
					if (!areWeFound) {
						discordGuild.data.userBanInfo.emplace_back(userBanInfo);
					}
					for (uint32_t x = 0; x < discordGuild.data.userBanInfo.size(); x += 1) {
						if (discordGuild.data.userBanInfo[x].userId == guildMember.user.id) {
							BanInfoLite newData;
							auto userNew = Users::getUserAsync({ .userId = userId }).get();
							for (auto& value: discordGuild.data.userBanInfo[x].userBans) {
								if (value.userId == userNew.id) {
									discordGuild.data.userBanInfo.erase(discordGuild.data.userBanInfo.begin() + x);
								}
							}
							newData.userId	  = userId;
							newData.userName  = userNew.userName;
							newData.avatarUrl = userNew.getUserImageUrl(UserImageTypes::Avatar);
							newData.bannedAt  = getTimeAndDate();
							newData.reason	  = reason;
							discordGuild.data.userBanInfo[x].userBans.emplace_back(newData);
							discordGuild.data.userBanInfo[x].userId = guildMember.user.id;
							discordGuild.writeDataToDB(managerAgent);

							jsonifier::string msgString = "------\n**Nicely done! You owned that motherfucker " + jsonifier::string{ userNew.userName } +
								" good!**\nYour current ban count is: " + jsonifier::toString(discordGuild.data.userBanInfo[x].userBans.size()) + "\n------";
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setDescription(msgString);
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setTitle("__**Succesful Ban:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							RespondToInputEventData dataPackage03(argsNewer);
							dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage03.addMessageEmbed(*msgEmbed);
							argsNewer = InputEvents::respondToInputEventAsync(dataPackage03).get();
							return;
						}
					}
				}

				else if (whatAreWeDoing == "view") {
					uint32_t maxIdx = 0;
					uint32_t len	= ( uint32_t )discordGuild.data.userBanInfo.size();
					for (uint32_t x = 0; x < len; x += 1) {
						maxIdx = x;
						for (uint32_t y = x + 1; y < len; y += 1) {
							if (discordGuild.data.userBanInfo.at(y).userBans.size() > discordGuild.data.userBanInfo.at(maxIdx).userBans.size()) {
								maxIdx = y;
							}
						}
						UserBanInfo tempMember					 = discordGuild.data.userBanInfo.at(x);
						discordGuild.data.userBanInfo.at(x)		 = discordGuild.data.userBanInfo.at(maxIdx);
						discordGuild.data.userBanInfo.at(maxIdx) = tempMember;
					}
					discordGuild.writeDataToDB(managerAgent);
					uint32_t membersPerPage = 20;
					uint32_t totalPageCount = 0;
					if (discordGuild.data.userBanInfo.size() % membersPerPage > 0) {
						totalPageCount = ( uint32_t )trunc(discordGuild.data.userBanInfo.size() / membersPerPage) + 1;
					} else {
						totalPageCount = ( uint32_t )trunc(discordGuild.data.userBanInfo.size() / membersPerPage);
					}
					uint32_t currentPage = 0;
					jsonifier::vector<EmbedData> pageEmbeds;
					jsonifier::vector<jsonifier::string> pageStrings;
					for (int32_t x = 0; x < discordGuild.data.userBanInfo.size(); x += 1) {
						if (x % membersPerPage == 0) {
							pageEmbeds.emplace_back(EmbedData());
							pageStrings.emplace_back("------\n__**Enter 'Select' to view your trophy case!**__\n");
						}

						jsonifier::string msgString = "__**#" + jsonifier::toString(currentPage * membersPerPage + ((x % membersPerPage) + 1)) + " | Name:**__ <@" +
							discordGuild.data.userBanInfo[x].userId + "> __**| Ban Count:**__ " + jsonifier::toString(discordGuild.data.userBanInfo[x].userBans.size()) + "\n";

						pageStrings[currentPage] += msgString;
						if (x % membersPerPage == membersPerPage - 1 || x == discordGuild.data.userBanInfo.size() - 1) {
							pageStrings[currentPage] += "------";
							pageEmbeds[currentPage].setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							pageEmbeds[currentPage].setDescription(pageStrings[currentPage]);
							pageEmbeds[currentPage].setTimeStamp(getTimeAndDate());
							pageEmbeds[currentPage].setTitle("__**Ban Leaderboard (Page " + jsonifier::toString(currentPage + 1) + " of " + jsonifier::toString(totalPageCount) + ")**__");
							pageEmbeds[currentPage].setColor(discordGuild.data.borderColor);
							currentPage += 1;
						}
					}
					if (pageEmbeds.size() == 0) {
						jsonifier::string msgString = "------\n**Sorry, but noone has banned anyone yet!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**No Bans:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						pageEmbeds.emplace_back(*msgEmbed);
					}

					currentPage		 = 0;
					auto returnValue = moveThroughMessagePages(guildMember.user.id.operator jsonifier::string(), argsNewer, currentPage, pageEmbeds, false, 120000, true);
					if (returnValue.buttonId == "exit" || returnValue.buttonId == "empty") {
						InputEvents::deleteInputEventResponseAsync(returnValue.inputEventData);
						return;
					}
					if (returnValue.buttonId == "select") {
						jsonifier::vector<EmbedData> msgEmbeds;
						for (auto& value: discordGuild.data.userBanInfo) {
							if (value.userId == guildMember.user.id) {
								for (auto& value2: value.userBans) {
									UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
									msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
									msgEmbed->setDescription("------\n__**Date Banned:**__ " + value2.bannedAt + "\n__**Reason:**__ " + value2.reason + "\n------");
									msgEmbed->setTimeStamp(getTimeAndDate());
									msgEmbed->setTitle("__**Banned UserData: " + value2.userName + "**__");
									msgEmbed->setImage(value2.avatarUrl);
									msgEmbed->setColor(discordGuild.data.borderColor);
									msgEmbeds.emplace_back(*msgEmbed);
								}
							}
						}
						if (msgEmbeds.size() == 0) {
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setDescription("------\n__**You have not banned anyone yet!**__\n------");
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Banned Users:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbeds.emplace_back(*msgEmbed);
						}
						int32_t currentPageIndex02 = 0;
						moveThroughMessagePages(guildMember.user.id.operator jsonifier::string(), InputEventData(returnValue.inputEventData), currentPageIndex02, msgEmbeds, true,
							120000, true);
					}
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "Ban::execute()" << error.what() << std::endl;
			}
		}
		~Ban(){};
	};

};// namespace DiscordCoreAPI
