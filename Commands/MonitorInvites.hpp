// MonitorInvites.hpp - Header for the "monitor invites" command.
// Aug 3, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {


	class SetInvitesChannel : public BaseFunction {
	  public:
		SetInvitesChannel() {
			this->commandName	  = "setinviteschannel";
			this->helpDescription = "Sets the channel to track invites in, and enables it.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /setinviteschannel add/remove/view, where 'add' adds the channel, 'remove' removes it, and 'view' displays "
									"the current list of members and their invites counts.\n------");
			msgEmbed.setTitle("__**Set Invites ChannelData Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetInvitesChannel>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember	 = GuildMembers::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, true);

				if (!doWeHaveAdminPermission) {
					return;
				}


				if (argsNew.getSubCommandName() == "add") {
					discordGuild.data.inviteReportingChannelId = channel.id;
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString =
						"**------\nNice! You've activated invite tracking by adding the channel <#" + channel.id + "> as the tracking channel for the invites!\n------** ";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Invite Tracking ChannelData Added:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getSubCommandName() == "remove") {
					discordGuild.data.inviteReportingChannelId = 0;
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString =
						"**------\nNice! You've de-activated invite tracking by removing the channel <#" + channel.id + "> as the tracking channel for the invites!\n------** ";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Invite Tracking ChannelData Disabled:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getSubCommandName() == "view") {
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					jsonifier::vector<jsonifier::string> descriptionStrings;
					jsonifier::string msgString01;
					jsonifier::vector<DiscordGuildMember> discordGuildMembers;
					jsonifier::vector<uint64_t> idsAlreadyInUse;
					for (auto& value: guild.members) {
						auto guildMemberNew = GuildMembers::getCachedGuildMember({ .guildMemberId = value.user.id.operator const uint64_t&(), .guildId = guild.id });
						DiscordGuildMember discordGuildMember(managerAgent, guildMemberNew);
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
						DiscordGuildMember tempMember  = std::move(discordGuildMembers.at(x));
						discordGuildMembers.at(x)	   = std::move(discordGuildMembers.at(maxIdx));
						discordGuildMembers.at(maxIdx) = std::move(tempMember);
					}
					InviteData vanityInvite = Guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
					msgString01 += "__**The vanity Url's Invite Count:**__ " + jsonifier::toString(vanityInvite.uses) + "\n";
					for (uint32_t x = 0; x < discordGuildMembers.size(); x += 1) {
						jsonifier::string currentString =
							"__**<@" + discordGuildMembers[x].data.guildMemberId + ">'s Invite Count:**__ " + jsonifier::toString(discordGuildMembers[x].data.totalInvites) + "\n";
						msgString01 += currentString;
						if (msgString01.size() + currentString.size() > 2048 || x == discordGuildMembers.size() - 1) {
							descriptionStrings.emplace_back(msgString01);
							msgString01 = jsonifier::string();
						}
					}
					int32_t currentIndex = 0;
					jsonifier::vector<EmbedData> msgEmbeds;
					if (descriptionStrings.size() == 0) {
						jsonifier::string msgString = "**------\nLooks like there's no stored invites!\n------**";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**UserData Invite Counts 0 of 0:**__");
						msgEmbeds.emplace_back(*msgEmbed);
					} else {
						for (uint32_t x = 0; x < descriptionStrings.size(); x += 1) {
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(descriptionStrings[x]);
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setTitle("__**UserData Invite Counts (" + jsonifier::toString((x + 1)) + " of " + jsonifier::toString(descriptionStrings.size()) + "):**__");
							msgEmbeds.emplace_back(*msgEmbed);
						}
					}

					moveThroughMessagePages(argsNew.getUserData().id.operator jsonifier::string(), argsNewer, currentIndex, msgEmbeds, false, 120000, false);
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "SetInvitesChannel::execute()" << error.what() << std::endl;
			}
		}
	};

	class MonitorInvitesArgs {
	  public:
		MonitorInvitesArgs(const GuildMemberData guildMember) {
			this->guildMemberData = guildMember;
		}
		GuildMemberData guildMemberData;
	};

	class MonitorInvites {
	  public:
		MonitorInvites(){};

		static void updateInvitesDataBaseToWrap(Snowflake guildId) {
			try {
				GuildData guild = Guilds::getGuildAsync({ .guildId = guildId }).get();
				DiscordGuild discordGuild{ managerAgent, guild };
				InviteData vanityInvite{};
				if (guild.vanityUrlCode != "") {
					vanityInvite					   = Guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
					discordGuild.data.vanityInviteUses = vanityInvite.uses;
					discordGuild.writeDataToDB(managerAgent);
				}
				auto invites = Guilds::getGuildInvitesAsync({ .guildId = guild.id }).get();
				for (uint32_t x = 0; x < invites.size(); x += 1) {
					for (auto& value2: guild.members) {
						GuildMemberData guildMemberNew = GuildMembers::getCachedGuildMember({ .guildMemberId = value2.user.id, .guildId = guild.id });
						DiscordGuildMember discordGuildMember(managerAgent, guildMemberNew);
						if (invites[x].inviter.id == discordGuildMember.data.guildMemberId) {
							bool isItFound = false;
							for (uint32_t y = 0; y < discordGuildMember.data.invites.size(); y += 1) {
								if (invites[x].code == discordGuildMember.data.invites[y].inviteCode) {
									isItFound = true;
									discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + y);
								}
							}
							DiscordInviteData inviteData;
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
				std::cout << "updateInvitesDataBaseToWrap Error: " << error.what() << std::endl;
			}
			return;
		}

		static void execute(OnInviteCreationData dataPackage) {
			updateInvitesDataBaseToWrap(dataPackage.value.guildId);
		}

		static void execute(OnInviteDeletionData dataPackage) {
			updateInvitesDataBaseToWrap(dataPackage.value.guildId);
		}

		static void execute(MonitorInvitesArgs argsNew) {
			try {
				auto invites	= Guilds::getGuildInvitesAsync({ .guildId = argsNew.guildMemberData.guildId }).get();
				GuildData guild = Guilds::getGuildAsync({ .guildId = argsNew.guildMemberData.guildId }).get();
				DiscordGuild discordGuild{ managerAgent, guild };
				InviteData vanityInvite;
				if (guild.vanityUrlCode != "") {
					vanityInvite = Guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
				}

				if (discordGuild.data.inviteReportingChannelId != 0) {
					int32_t currengGuildMemberInvitesCount = 0;
					GuildMemberData guildMemberInviterData;
					if (vanityInvite.uses >= ( int32_t )discordGuild.data.vanityInviteUses + 1) {
						goto pastPart;
					}

					for (auto& value: guild.members) {
						auto guildMemberNew = GuildMembers::getCachedGuildMember({ .guildMemberId = value.user.id, .guildId = guild.id });
						DiscordGuildMember discordGuildMember(managerAgent, guildMemberNew);
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
											discordGuildMember.data.invitedMemberIds.emplace_back(argsNew.guildMemberData.user.id);
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
									discordGuildMember.data.invitedMemberIds.emplace_back(argsNew.guildMemberData.user.id);
								}
								discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + x);
								currengGuildMemberInvitesCount = discordGuildMember.data.totalInvites;
								discordGuildMember.writeDataToDB(managerAgent);
								goto pastPart;
							}
						}
					}

				pastPart:
					EmbedData msgEmbed{};
					auto botUser = argsNew.guildMemberData.getUserData();
					msgEmbed.setAuthor(botUser.userName, botUser.getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setTimeStamp(getTimeAndDate());
					if (guildMemberInviterData.user.id != 0) {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited by the server member <@" + guildMemberInviterData.user.id + ">, (" +
							jsonifier::string{ guildMemberInviterData.getUserData().userName } + "), who now has " + jsonifier::toString(currengGuildMemberInvitesCount) +
							" invites!\n------**");
					} else if (discordGuild.data.vanityInviteUses <= vanityInvite.uses - 1 && vanityInvite.code != "") {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited using the vanity Url code: " + guild.vanityUrlCode + ".\n------**");
						discordGuild.data.vanityInviteUses = vanityInvite.uses;
						discordGuild.writeDataToDB(managerAgent);
					} else {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + argsNew.guildMemberData.user.id + ">, (" +
							jsonifier::string{ argsNew.guildMemberData.getUserData().userName } + ") was invited by an unknown server member!\n------**");
					}
					msgEmbed.setTitle("__**Inviter Status:**__");
					CreateMessageData dataPackage(discordGuild.data.inviteReportingChannelId);
					dataPackage.addMessageEmbed(msgEmbed);
					Messages::createMessageAsync(dataPackage).get();
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "MonitorInvites::execute()" << error.what() << std::endl;
			}
			return;
		}
		~MonitorInvites(){};
	};
}// namespace DiscordCoreAPI
