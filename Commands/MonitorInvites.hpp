// MonitorInvites.hpp - Header for the "monitor invites" command.
// Aug 3, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {


	class SetInvitesChannel : public BaseFunction {
	  public:
		SetInvitesChannel() {
			this->commandName = "setinviteschannel";
			this->helpDescription = "Sets the channel to track invites in, and enables it.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /setinviteschannel add/remove/view, where 'add' adds the channel, 'remove' removes it, and 'view' displays "
									"the current list of members and their invites counts.\n------");
			msgEmbed.setTitle("__**Set Invites Channel Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<SetInvitesChannel>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();
				InputEvents::deleteInputEventResponseAsync(newArgs.eventData);
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				if (newArgs.commandData.optionsArgs[0] == "add") {
					discordGuild.data.inviteReportingChannelId = newArgs.eventData.getChannelId();
					discordGuild.writeDataToDB();
					std::string msgString = "**------\nNice! You've activated invite tracking by adding the channel <#" + newArgs.eventData.getChannelId() +
						"> as the tracking channel for the invites!\n------** ";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Invite Tracking Channel Added:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (newArgs.commandData.optionsArgs[0] == "remove") {
					discordGuild.data.inviteReportingChannelId = "";
					discordGuild.writeDataToDB();
					std::string msgString = "**------\nNice! You've de-activated invite tracking by removing the channel <#" + newArgs.eventData.getChannelId() +
						"> as the tracking channel for the invites!\n------** ";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Invite Tracking Channel Disabled:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (newArgs.commandData.optionsArgs[0] == "view") {
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					std::vector<std::string> descriptionStrings;
					std::string msgString01;
					std::vector<DiscordGuildMember> discordGuildMembers;
					std::vector<std::string> idsAlreadyInUse;
					for (auto& value: guild.members) {
						auto guildMemberNew = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = value, .guildId = guild.id }).get();
						DiscordGuildMember discordGuildMember(guildMemberNew);
						discordGuildMember.getDataFromDB();
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
							discordGuildMembers.push_back(std::move(discordGuildMember));
							idsAlreadyInUse.push_back(value);
						}
					}
					uint32_t maxIdx = 0;
					uint32_t len = ( uint32_t )discordGuildMembers.size();
					for (uint32_t x = 0; x < len; x += 1) {
						maxIdx = x;
						for (uint32_t y = x + 1; y < len; y += 1) {
							if (discordGuildMembers.at(y).data.totalInvites > discordGuildMembers.at(maxIdx).data.totalInvites) {
								maxIdx = y;
							}
						}
						DiscordGuildMember tempMember = std::move(discordGuildMembers.at(x));
						discordGuildMembers.at(x) = std::move(discordGuildMembers.at(maxIdx));
						discordGuildMembers.at(maxIdx) = std::move(tempMember);
					}
					InviteData vanityInvite = Guilds::getGuildVanityInviteAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
					msgString01 += "__**The vanity Url's Invite Count:**__ " + std::to_string(vanityInvite.uses) + "\n";
					for (uint32_t x = 0; x < discordGuildMembers.size(); x += 1) {
						std::string currentString =
							"__**<@" + discordGuildMembers[x].data.guildMemberId + ">'s Invite Count:**__ " + std::to_string(discordGuildMembers[x].data.totalInvites) + "\n";
						msgString01 += currentString;
						if (msgString01.size() + currentString.size() > 2048 || x == discordGuildMembers.size() - 1) {
							descriptionStrings.push_back(msgString01);
							msgString01 = std::string();
						}
					}
					int32_t currentIndex = 0;
					std::vector<EmbedData> msgEmbeds;
					if (descriptionStrings.size() == 0) {
						std::string msgString = "**------\nLooks like there's no stored invites!\n------**";
						std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**User Invite Counts 0 of 0:**__");
						msgEmbeds.push_back(*msgEmbed);
					} else {
						for (uint32_t x = 0; x < descriptionStrings.size(); x += 1) {
							std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
							msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(descriptionStrings[x]);
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setTitle("__**User Invite Counts (" + std::to_string((x + 1)) + " of " + std::to_string(descriptionStrings.size()) + "):**__");
							msgEmbeds.push_back(*msgEmbed);
						}
					}

					moveThroughMessagePages(newArgs.eventData.getAuthorId(), newEvent, currentIndex, msgEmbeds, false, 120000);
				}
				return;
			} catch (...) {
				reportException("SetInvitesChannel::execute()");
			}
		}
	};

	class MonitorInvitesArgs {
	  public:
		MonitorInvitesArgs(GuildMember guildMember, DiscordCoreAPI::DiscordCoreClient* discordCoreClient) {
			this->discordCoreClient = discordCoreClient;
			this->guildMemberData = guildMember;
		}
		DiscordCoreAPI::DiscordCoreClient* discordCoreClient{ nullptr };
		GuildMemberData guildMemberData;
	};

	class MonitorInvites {
	  public:
		MonitorInvites(){};

		static void updateInvitesDataBaseToWrap(std::string guildId) {
			try {
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = guildId }).get();
				DiscordGuild discordGuild{ guild };
				InviteData vanityInvite{};
				if (guild.vanityUrlCode != "") {
					vanityInvite = Guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
					discordGuild.data.vanityInviteUses = vanityInvite.uses;
					discordGuild.writeDataToDB();
				}
				std::vector<InviteData> invites = Guilds::getGuildInvitesAsync({ .guildId = guild.id }).get();
				for (uint32_t x = 0; x < invites.size(); x += 1) {
					for (auto& value2: guild.members) {
						auto guildMemberNew = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = value2, .guildId = guild.id }).get();
						DiscordGuildMember discordGuildMember(guildMemberNew);
						if (invites[x].inviter.id == discordGuildMember.data.guildMemberId) {
							bool isItFound = false;
							for (uint32_t y = 0; y < discordGuildMember.data.invites.size(); y += 1) {
								if (invites[x].code == discordGuildMember.data.invites[y].inviteCode) {
									isItFound = true;
									discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + y);
								}
							}
							DiscordInviteData inviteData;
							inviteData.inviteCode = invites[x].code;
							inviteData.maxInvites = invites[x].maxUses;
							inviteData.invitesUsed = invites[x].uses;
							discordGuildMember.data.invites.push_back(inviteData);
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
						discordGuildMember.writeDataToDB();
					}
				}
				return;
			} catch (...) {
				reportException("updateInvitesDataBaseToWrap Error: ");
			}
			return;
		}

		static CoRoutine<void> execute(OnInviteCreationData dataPackage) {
			co_await NewThreadAwaitable<void>();
			updateInvitesDataBaseToWrap(dataPackage.invite.guildId);
			co_return;
		}

		static CoRoutine<void> execute(OnInviteDeletionData dataPackage) {
			co_await NewThreadAwaitable<void>();
			updateInvitesDataBaseToWrap(dataPackage.guildId);
			co_return;
		}

		static void execute(MonitorInvitesArgs newArgs) {
			try {
				auto invites = Guilds::getGuildInvitesAsync({ .guildId = newArgs.guildMemberData.guildId }).get();
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.guildMemberData.guildId }).get();
				DiscordGuild discordGuild{ guild };
				InviteData vanityInvite;
				if (guild.vanityUrlCode != "") {
					vanityInvite = Guilds::getGuildVanityInviteAsync({ .guildId = guild.id }).get();
				}

				if (discordGuild.data.inviteReportingChannelId != "") {
					int32_t currengGuildMemberInvitesCount = 0;
					GuildMemberData guildMemberInviterData;
					if (vanityInvite.uses >= ( int32_t )discordGuild.data.vanityInviteUses + 1) {
						goto pastPart;
					}

					for (auto& value: guild.members) {
						auto guildMemberNew = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = value, .guildId = guild.id }).get();
						DiscordGuildMember discordGuildMember(guildMemberNew);
						for (uint32_t x = 0; x < discordGuildMember.data.invites.size(); x += 1) {
							bool isItFound = false;
							for (uint32_t y = 0; y < invites.size(); y += 1) {
								if (invites[y].code == discordGuildMember.data.invites[x].inviteCode) {
									isItFound = true;
									if (invites[y].uses >= discordGuildMember.data.invites[x].invitesUsed + 1) {
										guildMemberInviterData = guildMemberNew;
										bool areTheyFound = false;
										for (auto& value02: discordGuildMember.data.invitedMemberIds) {
											if (value02 == newArgs.guildMemberData.user.id) {
												areTheyFound = true;
											}
										}
										if (areTheyFound == false) {
											discordGuildMember.data.totalInvites += 1;
											discordGuildMember.data.invitedMemberIds.push_back(newArgs.guildMemberData.user.id);
										}
										discordGuildMember.data.invites[x].invitesUsed = invites[y].uses;
										discordGuildMember.data.invites[x].maxInvites = invites[y].maxUses;
										currengGuildMemberInvitesCount = discordGuildMember.data.totalInvites;
										discordGuildMember.writeDataToDB();
										goto pastPart;
									}
								}
							}
							if (!isItFound) {
								guildMemberInviterData = guildMemberNew;
								bool areTheyFound = false;
								for (auto& value02: discordGuildMember.data.invitedMemberIds) {
									if (value02 == newArgs.guildMemberData.user.id) {
										areTheyFound = true;
									}
								}
								if (areTheyFound == false) {
									discordGuildMember.data.totalInvites += 1;
									discordGuildMember.data.invitedMemberIds.push_back(newArgs.guildMemberData.user.id);
								}
								discordGuildMember.data.invites.erase(discordGuildMember.data.invites.begin() + x);
								currengGuildMemberInvitesCount = discordGuildMember.data.totalInvites;
								discordGuildMember.writeDataToDB();
								goto pastPart;
							}
						}
					}

				pastPart:
					EmbedData msgEmbed{};
					auto botUser = newArgs.discordCoreClient->getBotUser();
					msgEmbed.setAuthor(botUser.userName, botUser.avatar);
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setTimeStamp(getTimeAndDate());
					if (guildMemberInviterData.user.id != "") {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + newArgs.guildMemberData.user.id + ">, (" +
							newArgs.guildMemberData.user.userName + ") was invited by the server member <@" + guildMemberInviterData.user.id + ">, (" +
							guildMemberInviterData.user.userName + "), who now has " + std::to_string(currengGuildMemberInvitesCount) + " invites!\n------**");
					} else if (discordGuild.data.vanityInviteUses <= vanityInvite.uses - 1 && vanityInvite.code != "") {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + newArgs.guildMemberData.user.id + ">, (" +
							newArgs.guildMemberData.user.userName + ") was invited using the vanity Url code: " + guild.vanityUrlCode + ".\n------**");
						discordGuild.data.vanityInviteUses = vanityInvite.uses;
						discordGuild.writeDataToDB();
					} else {
						msgEmbed.setDescription("**------\nIt appears as though the server member <@" + newArgs.guildMemberData.user.id + ">, (" +
							newArgs.guildMemberData.user.userName + ") was invited by an unknown server member!\n------**");
					}
					msgEmbed.setTitle("__**Inviter Status:**__");
					CreateMessageData dataPackage(discordGuild.data.inviteReportingChannelId);
					dataPackage.addMessageEmbed(msgEmbed);
					Messages::createMessageAsync(dataPackage).get();
				}
				return;
			} catch (...) {
				reportException("MonitorInvites::execute()");
			}
			return;
		}
		~MonitorInvites(){};
	};
}