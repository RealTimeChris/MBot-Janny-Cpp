// Ban.hpp - Header for the "ban" command.
// aug 7, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	inline jsonifier::string convertToNumberEmoji(uint64_t value) {
		static constexpr jsonifier::string_view one{ ":one:" };
		static constexpr jsonifier::string_view two{ ":two:" };
		static constexpr jsonifier::string_view three{ ":three:" };
		static constexpr jsonifier::string_view four{ ":four:" };
		static constexpr jsonifier::string_view five{ ":five:" };
		static constexpr jsonifier::string_view six{ ":six:" };
		static constexpr jsonifier::string_view seven{ ":seven:" };
		static constexpr jsonifier::string_view eight{ ":eight:" };
		static constexpr jsonifier::string_view nine{ ":nine:" };
		static constexpr jsonifier::string_view zero{ ":zero:" };
		static constexpr jsonifier::string_view svArray[10]{ zero, one, two, three, four, five, six, seven, eight, nine };
		jsonifier::string returnValue{};
		auto newString = jsonifier::toString(value);
		for (auto& value: newString) {
			returnValue += svArray[jsonifier::strToInt64(value)] + " ";
		}
		return returnValue;
	}

	class ban : public base_function {
	  public:
		ban() {
			this->commandName	  = "ban";
			this->helpDescription = "bans a user from the current server.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ban @usermention, reason, #ofdaystopurgemessagesofuser.\n------");
			msgEmbed.setTitle("__**ban usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<ban>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember{ argsNew.getGuildMemberData() };


				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, false);
				if (!doWeHaveAdminPermission) {
					return;
				}

				input_event_data argsNewer = argsNew.getInputEventData();
				snowflake userId{};
				jsonifier::string reason;
				jsonifier::string whatAreWeDoing;
				int32_t daysDigit = 0;

				if (argsNew.getCommandArguments().values.size() > 2 && argsNew.getCommandArguments().values.contains("reason")) {
					reason = argsNew.getCommandArguments().values["reason"].operator jsonifier::string();
				}
				if (argsNew.getCommandArguments().values.size() > 3) {
					if (argsNew.getCommandArguments().values["numberofdaystopurge"].operator size_t() > 7 ||
						argsNew.getCommandArguments().values["numberofdaystopurge"].operator size_t() < 0) {
						jsonifier::string msgString = "------\n**please, enter a proper number of days for purging the user's messages (0-7) (!ban = @usermention, "
												"reason, #ofdaystopurge)**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor("fefefe");
						msgEmbed->setTitle("__**missing or invalid parameters:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					} else {
						daysDigit = argsNew.getCommandArguments().values["numberofdaystopurge"].operator size_t();
					}
				}
				whatAreWeDoing = argsNew.getSubCommandName();
				if (whatAreWeDoing == "add") {
					guild_member_data guildMember01  = guild_members::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
					guild_member_data botGuildMember = guild_members::getCachedGuildMember({ .guildMemberId = discord_core_client::getInstance()->getBotUser().id, .guildId = guild.id });
					auto userRoles				   = roles::getGuildMemberRolesAsync({ .guildMember = guildMember01, .guildId = guild.id }).get();
					int32_t highestUserRolePosition{ 0 };

					for (auto& value: userRoles) {
						if (value.position > highestUserRolePosition) {
							highestUserRolePosition = value.position;
						}
					}

					int32_t highestBotRolePosition{ 0 };
					auto botRoles = roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = guild.id }).get();
					for (auto& value: botRoles) {
						if (value.position > highestBotRolePosition) {
							highestBotRolePosition = value.position;
						}
					}
					if (highestUserRolePosition >= highestBotRolePosition) {
						jsonifier::string msgString = "------\n**sorry, but i cannot ban them as their highest role is higher than mine!**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor("fefefe");
						msgEmbed->setTitle("__**missing or invalid parameters:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					}

					create_guild_ban_data dataPackageNew{};
					dataPackageNew.deleteMessageDays = daysDigit;
					dataPackageNew.guildId			 = guild.id;

					dataPackageNew.reason		 = reason;
					dataPackageNew.guildMemberId = userId;

					guilds::createGuildBanAsync(dataPackageNew).get();
					discordGuild.getDataFromDB(managerAgent);
					bool areWeFound = false;
					for (auto& value: discordGuild.data.userBanInfo) {
						if (guildMember.user.id == value.userId) {
							areWeFound = true;
						}
					}
					user_ban_info userBanInfo;
					userBanInfo.userId = guildMember.user.id;
					if (!areWeFound) {
						discordGuild.data.userBanInfo.emplace_back(userBanInfo);
					}
					for (uint32_t x = 0; x < discordGuild.data.userBanInfo.size(); x += 1) {
						if (discordGuild.data.userBanInfo[x].userId == guildMember.user.id) {
							ban_info_lite newData;
							auto userNew = users::getUserAsync({ .userId = userId }).get();
							for (auto& value: discordGuild.data.userBanInfo[x].userBans) {
								if (value.userId == userNew.id) {
									discordGuild.data.userBanInfo.erase(discordGuild.data.userBanInfo.begin() + x);
								}
							}
							newData.userId	  = userId;
							newData.userName  = userNew.userName;
							newData.avatarUrl = userNew.getUserImageUrl<user_image_types::Avatar>();
							newData.bannedAt  = getTimeAndDate();
							newData.reason	  = reason;
							discordGuild.data.userBanInfo[x].userBans.emplace_back(newData);
							discordGuild.data.userBanInfo[x].userId = guildMember.user.id;
							discordGuild.writeDataToDB(managerAgent);

							jsonifier::string msgString = "------\n**nicely done! you owned that motherfucker " + jsonifier::string{ userNew.userName } +
								" good!**\nYour current ban count is: " + jsonifier::toString(discordGuild.data.userBanInfo[x].userBans.size()) + "\n------";
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setDescription(msgString);
							msgEmbed->setColor("fefefe");
							msgEmbed->setTitle("__**succesful ban:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							respond_to_input_event_data dataPackage03(argsNewer);
							dataPackage03.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage03.addMessageEmbed(*msgEmbed);
							argsNewer = input_events::respondToInputEventAsync(dataPackage03).get();
							return;
						}
					}
				}

				else if (whatAreWeDoing == "view") {
					snowflake currentNextId{ 0 };
					jsonifier::vector<ban_data> bansFinal{};
					jsonifier::vector<ban_data> bans{};
					do {
						bans = guilds::getGuildBansAsync({ .guildId = argsNew.getGuildMemberData().guildId, .after = currentNextId, .limit = 1000 }).get();
						if (bans.size() > 0) {
							currentNextId = bans.back().user.id;
							bansFinal.insert(bansFinal.end(), bans.begin(), bans.end());
						}
					} while (bans.size() > 0); 
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setDescription("------\n__**The number of bans in this server is:**__\n------\n__**" + convertToNumberEmoji(bansFinal.size()) + "**__\n------\n");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Banned users:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setColor("fefefe");
					respond_to_input_event_data responseData{ argsNew.getInputEventData() };
					responseData.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					responseData.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(responseData).get();
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "ban::execute()" << error.what() << std::endl;
			}
		}
		~ban(){};
	};

};// namespace discord_core_api

