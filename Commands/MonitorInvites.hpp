// MonitorInvites.hpp - Header for the "monitor invites" command.
// aug 3, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {


	class set_invites_channel : public base_function {
	  public:
		set_invites_channel() {
			this->commandName	  = "setinviteschannel";
			this->helpDescription = "sets the channel to track invites in, and enables it.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /setinviteschannel add/remove/view, where 'add' adds the channel, 'remove' removes it, and 'view' displays "
									"the current list of members and their invites counts.\n------");
			msgEmbed.setTitle("__**set invites channel_data usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<set_invites_channel>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember	 = guild_members::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, true);

				if (!doWeHaveAdminPermission) {
					return;
				}


				if (argsNew.getSubCommandName() == "add") {
					discordGuild.data.inviteReportingChannelId = channel.id;
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString =
						"**------\nNice! you've activated invite tracking by adding the channel <#" + channel.id + "> as the tracking channel for the invites!\n------** ";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**invite tracking channel_data added:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getSubCommandName() == "remove") {
					discordGuild.data.inviteReportingChannelId = 0;
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString =
						"**------\nNice! you've de-activated invite tracking by removing the channel <#" + channel.id + "> as the tracking channel for the invites!\n------** ";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**invite tracking channel_data disabled:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getSubCommandName() == "view") {
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Deferred_Response);
					auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
					jsonifier::vector<jsonifier::string> descriptionStrings;
					jsonifier::string msgString01;
					jsonifier::vector<discord_guild_member> discordGuildMembers;
					jsonifier::vector<uint64_t> idsAlreadyInUse;
					for (auto& value: guild.members) {
						auto guildMemberNew = guild_members::getCachedGuildMember({ .guildMemberId = value.user.id.operator const uint64_t&(), .guildId = guild.id });
						discord_guild_member discordGuildMember(managerAgent, guildMemberNew);
						discordGuildMember.getDataFromDB(managerAgent);
						bool doWeContinue = false;
						for (auto& value02: idsAlreadyInUse) {
							if (discordGuildMember.data.guildMemberId == value02) {
								doWeContinue = true;
								break;
							}
						}
						if (doWeContinue) {
							continue;
						}
						if (discordGuildMember.data.totalInvites > 0) {
							discordGuildMembers.emplace_back(std::move(discordGuildMember));
							idsAlreadyInUse.emplace_back(value.user.id.operator const uint64_t&());
						}
					}
					uint32_t maxIdx = 0;
					uint32_t len	= ( uint32_t )discordGuildMembers.size();
					for (uint32_t x = 0; x < len; x += 1) {
						maxIdx = x;
						for (uint32_t y = x + 1; y < len; y += 1) {
							if (discordGuildMembers.at(y).data.totalInvites > discordGuildMembers.at(maxIdx).data.totalInvites) {
								maxIdx = y;
							}
						}
						discord_guild_member tempMember  = std::move(discordGuildMembers.at(x));
						discordGuildMembers.at(x)	   = std::move(discordGuildMembers.at(maxIdx));
						discordGuildMembers.at(maxIdx) = std::move(tempMember);
					}
					invite_data vanityInvite = guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
					msgString01 += "__**the vanity url's invite count:**__ " + jsonifier::toString(vanityInvite.uses) + "\n";
					for (uint32_t x = 0; x < discordGuildMembers.size(); x += 1) {
						jsonifier::string currentString =
							"__**<@" + discordGuildMembers[x].data.guildMemberId + ">'s invite count:**__ " + jsonifier::toString(discordGuildMembers[x].data.totalInvites) + "\n";
						msgString01 += currentString;
						if (msgString01.size() + currentString.size() > 2048 || x == discordGuildMembers.size() - 1) {
							descriptionStrings.emplace_back(msgString01);
							msgString01 = jsonifier::string();
						}
					}
					int32_t currentIndex = 0;
					jsonifier::vector<embed_data> msgEmbeds;
					if (descriptionStrings.size() == 0) {
						jsonifier::string msgString = "**------\nLooks like there's no stored invites!\n------**";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor("fefefe");
						msgEmbed->setTitle("__**user_data invite counts 0 of 0:**__");
						msgEmbeds.emplace_back(*msgEmbed);
					} else {
						for (uint32_t x = 0; x < descriptionStrings.size(); x += 1) {
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(descriptionStrings[x]);
							msgEmbed->setColor("fefefe");
							msgEmbed->setTitle("__**user_data invite counts (" + jsonifier::toString((x + 1)) + " of " + jsonifier::toString(descriptionStrings.size()) + "):**__");
							msgEmbeds.emplace_back(*msgEmbed);
						}
					}

					moveThroughMessagePages(argsNew.getUserData().id.operator jsonifier::string(), argsNewer, currentIndex, msgEmbeds, false, 120000, false);
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "set_invites_channel::execute()" << error.what() << std::endl;
			}
		}
	};

	class monitor_invites_args {
	  public:
		monitor_invites_args(const guild_member_data guildMember) {
			this->guildMemberData = guildMember;
		}
		guild_member_data guildMemberData;
	};

	class monitor_invites {
	  public:
		monitor_invites(){};

		static void updateInvitesDataBaseToWrap(snowflake guildId) {
			try {
				guild_data guild = guilds::getGuildAsync({ .guildId = guildId }).get();
				discord_guild discordGuild{ managerAgent, guild };
				invite_data vanityInvite{};
				if (guild.vanityUrlCode != "") {
					vanityInvite					   = guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
					discordGuild.data.vanityInviteUses = vanityInvite.uses;
					discordGuild.writeDataToDB(managerAgent);
				}
				auto invites = guilds::getGuildInvitesAsync({ .guildId = guild.id }).get();
				for (uint32_t x = 0; x < invites.size(); x += 1) {
					for (auto& value2: guild.members) {
						guild_member_data guildMemberNew = guild_members::getCachedGuildMember({ .guildMemberId = value2.user.id, .guildId = guild.id });
						discord_guild_member discordGuildMember(managerAgent, guildMemberNew);
						if (invites[x].inviter.id == discordGuildMember.data.guildMemberId) {
							bool isItFound = false;
							for (uint32_t y = 0; y < discordGuildMember.data.invites.size(); y += 1) {
								if (invites[x].code == discordGuildMember.data.invites[y].inviteCode) {
									isItFound = true;
									discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + y);
								}
							}
							discord_invite_data inviteData;
							inviteData.inviteCode  = invites[x].code;
							inviteData.maxInvites  = invites[x].maxUses;
							inviteData.invitesUsed = invites[x].uses;
							discordGuildMember.data.invites.emplace_back(inviteData);
						}
						for (uint32_t y = 0; y < discordGuildMember.data.invites.size(); y += 1) {
							bool isItFound02 = false;
							for (uint32_t z = 0; z < invites.size(); z += 1) {
								if (discordGuildMember.data.invites[y].inviteCode == invites[z].code) {
									isItFound02 = true;
								}
							}
							if (!isItFound02) {
								discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + y);
							}
						}
						discordGuildMember.writeDataToDB(managerAgent);
					}
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "updateInvitesDataBaseToWrap error: " << error.what() << std::endl;
			}
			return;
		}

		static void execute(on_invite_creation_data dataPackage) {
			updateInvitesDataBaseToWrap(dataPackage.value.guildId);
		}

		static void execute(on_invite_deletion_data dataPackage) {
			updateInvitesDataBaseToWrap(dataPackage.value.guildId);
		}

		static void execute(monitor_invites_args argsNew) {
			try {
				auto invites	= guilds::getGuildInvitesAsync({ .guildId = argsNew.guildMemberData.guildId }).get();
				guild_data guild = guilds::getGuildAsync({ .guildId = argsNew.guildMemberData.guildId }).get();
				discord_guild discordGuild{ managerAgent, guild };
				invite_data vanityInvite;
				if (guild.vanityUrlCode != "") {
					vanityInvite = guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
				}

				if (discordGuild.data.inviteReportingChannelId != 0) {
					int32_t currengGuildMemberInvitesCount = 0;
					guild_member_data guildMemberInviterData;
					if (vanityInvite.uses >= ( int32_t )discordGuild.data.vanityInviteUses + 1) {
						goto pastPart;
					}

					for (auto& value: guild.members) {
						auto guildMemberNew = guild_members::getCachedGuildMember({ .guildMemberId = value.user.id, .guildId = guild.id });
						discord_guild_member discordGuildMember(managerAgent, guildMemberNew);
						for (uint32_t x = 0; x < discordGuildMember.data.invites.size(); x += 1) {
							bool isItFound = false;
							for (uint32_t y = 0; y < invites.size(); y += 1) {
								if (invites[y].code == discordGuildMember.data.invites[x].inviteCode) {
									isItFound = true;
									if (invites[y].uses >= discordGuildMember.data.invites[x].invitesUsed + 1) {
										guildMemberInviterData = guildMemberNew;
										bool areTheyFound	   = false;
										for (auto& value02: discordGuildMember.data.invitedMemberIds) {
											if (value02 == argsNew.guildMemberData.user.id) {
												areTheyFound = true;
											}
										}
										if (areTheyFound == false) {
											discordGuildMember.data.totalInvites += 1;
											discordGuildMember.data.invitedMemberIds.emplace_back(argsNew.guildMemberData.user.id.operator jsonifier::string());
										}
										discordGuildMember.data.invites[x].invitesUsed = invites[y].uses;
										discordGuildMember.data.invites[x].maxInvites  = invites[y].maxUses;
										currengGuildMemberInvitesCount				   = discordGuildMember.data.totalInvites;
										discordGuildMember.writeDataToDB(managerAgent);
										goto pastPart;
									}
								}
							}
							if (!isItFound) {
								guildMemberInviterData = guildMemberNew;
								bool areTheyFound	   = false;
								for (auto& value02: discordGuildMember.data.invitedMemberIds) {
									if (value02 == argsNew.guildMemberData.user.id) {
										areTheyFound = true;
									}
								}
								if (areTheyFound == false) {
									discordGuildMember.data.totalInvites += 1;
									discordGuildMember.data.invitedMemberIds.emplace_back(argsNew.guildMemberData.user.id.operator jsonifier::string());
								}
								discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + x);
								currengGuildMemberInvitesCount = discordGuildMember.data.totalInvites;
								discordGuildMember.writeDataToDB(managerAgent);
								goto pastPart;
							}
						}
					}

				pastPart:
					embed_data msgEmbed{};
					auto botUser = argsNew.guildMemberData.getUserData();
					msgEmbed.setAuthor(botUser.userName, botUser.getUserImageUrl<user_image_types::Avatar>());
					msgEmbed.setColor("fefefe");
					msgEmbed.setTimeStamp(getTimeAndDate());
					if (guildMemberInviterData.user.id != 0) {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited by the server member <@" + guildMemberInviterData.user.id + ">, (" +
							jsonifier::string{ guildMemberInviterData.getUserData().userName } + "), who now has " + jsonifier::toString(currengGuildMemberInvitesCount) +
							" invites!\n------**");
					} else if (discordGuild.data.vanityInviteUses <= vanityInvite.uses - 1 && vanityInvite.code != "") {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited using the vanity url code: " + guild.vanityUrlCode + ".\n------**");
						discordGuild.data.vanityInviteUses = vanityInvite.uses;
						discordGuild.writeDataToDB(managerAgent);
					} else {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited by an unknown server member!\n------**");
					}
					msgEmbed.setTitle("__**inviter status:**__");
					create_message_data dataPackage(discordGuild.data.inviteReportingChannelId);
					dataPackage.addMessageEmbed(msgEmbed);
					messages::createMessageAsync(dataPackage).get();
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "monitor_invites::execute()" << error.what() << std::endl;
			}
			return;
		}
		~monitor_invites(){};
	};
}// namespace discord_core_api
