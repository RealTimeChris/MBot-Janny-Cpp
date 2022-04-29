// DatabaseEntities.hpp - Database stuff.
// May 24, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

#include <discordcoreapi/Index.hpp>

namespace DiscordCoreAPI {

	struct DiscordInviteData {
		std::string inviteCode{ "" };
		int32_t invitesUsed{ 0 };
		int32_t maxInvites{ 0 };
	};

	struct DeletionChannelData {
		int32_t minutesToWaitUntilDeleted{ 0 };
		bool currentlyBeingDeleted{ false };
		int32_t numberOfMessagesToSave{ 0 };
		std::string deletionMessageId{ "" };
		std::string channelId{ "" };
	};

	struct DiscordUserData {
		std::vector<std::string> botCommanders{ "", "", "" };
		std::string userName{ "" };
		std::string prefix{ "!" };
		std::string userId{ "" };
	};

	struct BanInfoLite {
		std::string avatarUrl{ "" };
		std::string userName{ "" };
		std::string bannedAt{ "" };
		std::string reason{ "" };
		std::string userId{ "" };
	};

	struct UserBanInfo {
		std::vector<BanInfoLite> userBans{};
		std::string userId{ "" };
	};

	struct RoleManager {
		std::vector<std::string> theRoles{};
		std::string channelId{ "" };
		std::string messageId{ "" };
		std::string message{ "" };
	};

	struct Log {
		std::string loggingChannelName{ "" };
		std::string loggingChannelId{ "" };
		std::string nameSmall{ "" };
		std::string name{ "" };
		bool enabled{ false };
	};

	struct DiscordGuildData {
		DiscordGuildData() {
			Log log{
				.loggingChannelName = "",
				.loggingChannelId = "",
				.nameSmall = "",
				.name = "",
			};
			log.name = "Guild Ban Add";
			log.nameSmall = "guildbanadd";
			this->logs.push_back(log);
			log.name = "Guild Ban remove";
			log.nameSmall = "guildbanremove";
			this->logs.push_back(log);
			log.name = "Guild Member Add";
			log.nameSmall = "guildmemberadd";
			this->logs.push_back(log);
			log.name = "Guild Member remove";
			log.nameSmall = "guildmemberremove";
			this->logs.push_back(log);
			log.name = "Display Name Change";
			log.nameSmall = "displaynamechange";
			this->logs.push_back(log);
			log.name = "Nickname Change";
			log.nameSmall = "nicknamechange";
			this->logs.push_back(log);
			log.name = "Role Add Or remove";
			log.nameSmall = "roleaddorremove";
			this->logs.push_back(log);
			log.name = "Invite Create";
			log.nameSmall = "invitecreate";
			this->logs.push_back(log);
			log.name = "Message Delete";
			log.nameSmall = "messagedelete";
			this->logs.push_back(log);
			log.name = "Message Delete Bulk";
			log.nameSmall = "messagedeletebulk";
			this->logs.push_back(log);
			log.name = "Role Create";
			log.nameSmall = "rolecreate";
			this->logs.push_back(log);
			log.name = "Role Delete";
			log.nameSmall = "roledelete";
			this->logs.push_back(log);
			log.name = "Username Change";
			log.nameSmall = "usernamechange";
			this->logs.push_back(log);
		}
		std::vector<DeletionChannelData> deletionChannels{};
		std::string inviteReportingChannelId{ "" };
		std::vector<std::string> defaultRoleIds{};
		std::vector<std::string> trackedUsers{};
		std::vector<UserBanInfo> userBanInfo{};
		std::vector<std::string> ghostedIds{};
		std::string borderColor{ "FEFEFE" };
		int32_t vanityInviteUses{ 0 };
		std::string guildName{ "" };
		RoleManager roleManager{};
		std::string guildId{ "" };
		uint32_t memberCount{ 0 };
		std::vector<Log> logs{};
	};

	/// A Permission overwrite, for a given Channel. \brief A Permission overwrite, for a given Channel.
	struct PermissionOverWriteData {
		std::string channelId{ "" };///< Channel id for which Channel this overwrite beint64_ts to.
		Permissions allow{ "" };///< Collection of Permissions to allow.
		Permissions deny{ "" };///< Collection of Permissions to deny.
		PermissionOverwritesType type{};///< Role or User type.
		std::string id{ "" };///< Id of the permission overwrite.
	};

	struct DiscordGuildMemberData {
		std::vector<PermissionOverWriteData> previousPermissionOverwrites{};
		std::vector<std::string> invitedMemberIds{};
		std::vector<std::string> previousRoleIds{};
		std::vector<DiscordInviteData> invites{};
		std::string guildMemberMention{ "" };
		std::string guildMemberId{ "" };
		std::string displayName{ "" };
		uint32_t totalInvites{ 0 };
		std::string globalId{ "" };
		std::string userName{ "" };
		std::string guildId{ "" };
	};

	enum class DatabaseWorkloadType {
		DISCORD_USER_WRITE = 0,
		DISCORD_USER_READ = 1,
		DISCORD_GUILD_WRITE = 2,
		DISCORD_GUILD_READ = 3,
		DISCORD_GUILD_MEMBER_WRITE = 4,
		DISCORD_GUILD_MEMBER_READ = 5
	};

	struct DatabaseWorkload {
		DiscordGuildMemberData guildMemberData{};
		DatabaseWorkloadType workloadType{};
		DiscordGuildData guildData{};
		DiscordUserData userData{};
	};

	struct DatabaseReturnValue {
		DiscordGuildMemberData discordGuildMember{};
		DiscordGuildData discordGuild{};
		DiscordUserData discordUser{};
	};

	class DatabaseManagerAgent {
	  protected:
		static mongocxx::instance instance;
		static std::mutex workloadMutex;
		static mongocxx::pool thePool;
		static std::string botUserId;

		static bsoncxx::builder::basic::document convertUserDataToDBDoc(DiscordUserData discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", discordUserData.userId));
				buildDoc.append(kvp("userId", discordUserData.userId));
				buildDoc.append(kvp("userName", discordUserData.userName));
				buildDoc.append(kvp("prefix", discordUserData.prefix));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(value);
					}
				}));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertUserDataToDBDoc()");
				return buildDoc;
			}
		}

		static DiscordUserData parseUserData(bsoncxx::document::value docValue) {
			DiscordUserData userData{};
			try {
				userData.userName = docValue.view()["userName"].get_utf8().value.to_string();
				userData.userName = docValue.view()["prefix"].get_utf8().value.to_string();
				userData.userId = docValue.view()["userId"].get_utf8().value.to_string();
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				std::vector<std::string> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.push_back(value.get_utf8().value.to_string());
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseUserData()");
				return userData;
			}
		}

		static bsoncxx::builder::basic::document convertGuildDataToDBDoc(DiscordGuildData discordGuildData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", discordGuildData.guildId));
				buildDoc.append(kvp("guildId", discordGuildData.guildId));
				buildDoc.append(kvp("guildName", discordGuildData.guildName));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int32(discordGuildData.memberCount)));
				buildDoc.append(kvp("vanityInviteUses", bsoncxx::types::b_int32(discordGuildData.vanityInviteUses)));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor));
				buildDoc.append(kvp("roleManager", [discordGuildData](bsoncxx::builder::basic::sub_document sub_document01) {
					sub_document01.append(kvp("theRoles", [discordGuildData](bsoncxx::builder::basic::sub_array sub_array01) {
						for (auto& value: discordGuildData.roleManager.theRoles) {
							sub_array01.append(value);
						}
					}));
					sub_document01.append(kvp("channelId", discordGuildData.roleManager.channelId));

					sub_document01.append(kvp("messageId", discordGuildData.roleManager.messageId));

					sub_document01.append(kvp("message", discordGuildData.roleManager.message));
				}));
				buildDoc.append(kvp("ghostedIds", [discordGuildData](bsoncxx::builder::basic::sub_array subArray01) {
					for (auto& value02: discordGuildData.ghostedIds) {
						subArray01.append(value02);
					}
				}));
				buildDoc.append(kvp("inviteReportingChannelId", discordGuildData.inviteReportingChannelId));
				buildDoc.append(kvp("defaultRoleIds",
					[discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
						for (auto& value: discordGuildData.defaultRoleIds) {
							subArray.append(value);
						}
					}

					));
				buildDoc.append(kvp("deletionChannels", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.deletionChannels) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("channelId", value.channelId),
								kvp("currentlyBeingDeleted", bsoncxx::types::b_bool(value.currentlyBeingDeleted)),
								kvp("deletionMessageId", value.deletionMessageId), kvp("numberOfMessagesToSave", value.numberOfMessagesToSave),
								kvp("minutesToWaitUntilDeleted", value.minutesToWaitUntilDeleted));
						});
					}
				}));
				buildDoc.append(kvp("userBanInfo", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.userBanInfo) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("userId", value.userId));
							subDocument.append(kvp("userBans", [value](bsoncxx::builder::basic::sub_array subArray02) {
								for (auto& value02: value.userBans) {
									subArray02.append([=](bsoncxx::builder::basic::sub_document subDocument02) {
										subDocument02.append(kvp("reason", value02.reason));
										subDocument02.append(kvp("userName", value02.userName));
										subDocument02.append(kvp("userId", value02.userId));
										subDocument02.append(kvp("bannedAt", value02.bannedAt));
										subDocument02.append(kvp("avatarUrl", value02.avatarUrl));
									});
								}
							}));
						});
					}
				}));
				buildDoc.append(kvp("logs", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.logs) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("enabled", bsoncxx::types::b_bool(value.enabled)), kvp("loggingChannelId", value.loggingChannelId),
								kvp("loggingChannelName", value.loggingChannelName), kvp("name", value.name), kvp("nameSmall", value.nameSmall));
						});
					}
				}));

				buildDoc.append(kvp("trackedUsers", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.trackedUsers) {
						subArray.append(value);
					}
				}));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertGuildDataToDBDoc()");
				return buildDoc;
			}
		};

		static DiscordGuildData parseGuildData(bsoncxx::document::value docValue) {
			DiscordGuildData guildData{};
			try {
				guildData.inviteReportingChannelId = docValue.view()["inviteReportingChannelId"].get_utf8().value.to_string();
				if (docValue.view()["ghostedIds"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: docValue.view()["ghostedIds"].get_array().value) {
						guildData.ghostedIds.push_back(value.get_value().get_utf8().value.to_string());
					}
				}

				guildData.borderColor = docValue.view()["borderColor"].get_utf8().value.to_string();
				guildData.vanityInviteUses = docValue.view()["vanityInviteUses"].get_int32().value;
				guildData.guildName = docValue.view()["guildName"].get_utf8().value.to_string();
				guildData.guildId = docValue.view()["guildId"].get_utf8().value.to_string();
				guildData.memberCount = docValue.view()["memberCount"].get_int32().value;
				for (auto& value: docValue.view()["deletionChannels"].get_array().value) {
					DeletionChannelData newData;
					newData.channelId = value.get_document().view()["channelId"].get_utf8().value.to_string();
					newData.currentlyBeingDeleted = value.get_document().view()["currentlyBeingDeleted"].get_bool().value;
					newData.deletionMessageId = value.get_document().view()["deletionMessageId"].get_utf8().value.to_string();
					newData.numberOfMessagesToSave = value.get_document().view()["numberOfMessagesToSave"].get_int32().value;
					if (value.get_document().view()["minutesToWaitUntilDeleted"].type() != bsoncxx::v_noabi::type::k_null) {
						newData.minutesToWaitUntilDeleted = value.get_document().view()["minutesToWaitUntilDeleted"].get_int32().value;
					}
					guildData.deletionChannels.push_back(newData);
				}
				for (auto& value: docValue.view()["userBanInfo"].get_array().value) {
					UserBanInfo newData;
					newData.userId = value["userId"].get_utf8().value.to_string();
					for (auto& value2: value["userBans"].get_array().value) {
						BanInfoLite newData02;
						newData02.userId = value2["userId"].get_utf8().value.to_string();
						newData02.userName = value2["userName"].get_utf8().value.to_string();
						newData02.avatarUrl = value2["avatarUrl"].get_utf8().value.to_string();
						newData02.bannedAt = value2["bannedAt"].get_utf8().value.to_string();
						newData02.reason = value2["reason"].get_utf8().value.to_string();
						newData.userBans.push_back(newData02);
					}
					guildData.userBanInfo.push_back(newData);
				}
				for (auto& value: docValue.view()["trackedUsers"].get_array().value) {
					guildData.trackedUsers.push_back(value.get_utf8().value.to_string());
				}
				auto roleManager = docValue.view()["roleManager"].get_document();
				if (roleManager.view()["theRoles"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: roleManager.view()["theRoles"].get_array().value) {
						guildData.roleManager.theRoles.push_back(value.get_utf8().value.to_string());
					}
				}

				guildData.roleManager.channelId = roleManager.view()["channelId"].get_utf8().value.to_string();
				guildData.roleManager.messageId = roleManager.view()["messageId"].get_utf8().value.to_string();
				guildData.roleManager.message = roleManager.view()["message"].get_utf8().value.to_string();
				for (auto& value: docValue.view()["defaultRoleIds"].get_array().value) {
					guildData.defaultRoleIds.push_back(value.get_utf8().value.to_string());
				}
				for (auto& value: docValue.view()["logs"].get_array().value) {
					Log newData;
					newData.enabled = value.get_document().value["enabled"].get_bool().value;
					newData.loggingChannelId = value.get_document().value["loggingChannelId"].get_utf8().value.to_string();
					newData.loggingChannelName = value.get_document().value["loggingChannelName"].get_utf8().value.to_string();
					newData.name = value.get_document().value["name"].get_utf8().value.to_string();
					newData.nameSmall = value.get_document().value["nameSmall"].get_utf8().value.to_string();
					for (int32_t y = 0; y < guildData.logs.size(); y += 1) {
						if (guildData.logs[y].nameSmall == newData.nameSmall) {
							guildData.logs[y] = newData;
						}
					}
				}
				return guildData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseGuildData()");
				return guildData;
			}
		};

		static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(DiscordGuildMemberData discordGuildMemberData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("guildMemberMention", discordGuildMemberData.guildMemberMention));
				buildDoc.append(kvp("_id", discordGuildMemberData.globalId));
				buildDoc.append(kvp("guildId", discordGuildMemberData.guildId));
				buildDoc.append(kvp("guildMemberId", discordGuildMemberData.guildMemberId));
				buildDoc.append(kvp("globalId", discordGuildMemberData.globalId));
				buildDoc.append(kvp("userName", discordGuildMemberData.userName));
				buildDoc.append(kvp("displayName", discordGuildMemberData.displayName));
				buildDoc.append(kvp("totalInvites", bsoncxx::types::b_int32(discordGuildMemberData.totalInvites)));
				buildDoc.append(kvp("previousPermissionOverwrites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (PermissionOverWriteData value: discordGuildMemberData.previousPermissionOverwrites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("allow", bsoncxx::types::b_utf8(static_cast<std::string>(value.allow))),
								kvp("type", bsoncxx::types::b_int64(( int64_t )value.type)),
								kvp("deny", bsoncxx::types::b_utf8(static_cast<std::string>(value.deny))), kvp("id", value.id));
						});
					}
				}));
				buildDoc.append(kvp("invitedMemberIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.invitedMemberIds) {
						subArray.append(value);
					}
				}));
				buildDoc.append(kvp("previousRoleIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.previousRoleIds) {
						subArray.append(value);
					}
				}));
				buildDoc.append(kvp("invites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.invites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("inviteCode", value.inviteCode), kvp("invitesUsed", bsoncxx::types::b_int32(value.invitesUsed)),
								kvp("maxInvites", bsoncxx::types::b_int32(value.maxInvites)));
						});
					}
				}));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertGuildMemberDataToDBDoc()");
				return buildDoc;
			}
		};

		static DiscordGuildMemberData parseGuildMemberData(bsoncxx::document::value docValue) {
			DiscordGuildMemberData guildMemberData{};
			try {
				guildMemberData.guildMemberMention = docValue.view()["guildMemberMention"].get_utf8().value.to_string();
				guildMemberData.guildId = docValue.view()["guildId"].get_utf8().value.to_string();
				guildMemberData.displayName = docValue.view()["displayName"].get_utf8().value.to_string();
				guildMemberData.globalId = docValue.view()["globalId"].get_utf8().value.to_string();
				guildMemberData.guildMemberId = docValue.view()["guildMemberId"].get_utf8().value.to_string();
				guildMemberData.userName = docValue.view()["userName"].get_utf8().value.to_string();
				guildMemberData.totalInvites = docValue.view()["totalInvites"].get_int32().value;
				for (auto& value: docValue.view()["previousRoleIds"].get_array().value) {
					guildMemberData.previousRoleIds.push_back(value.get_utf8().value.to_string());
				}
				for (auto& value: docValue.view()["invitedMemberIds"].get_array().value) {
					guildMemberData.invitedMemberIds.push_back(value.get_utf8().value.to_string());
				}
				for (auto& value: docValue.view()["previousPermissionOverwrites"].get_array().value) {
					PermissionOverWriteData newOverWriteData{};
					newOverWriteData.allow = value.get_document().view()["allow"].get_utf8().value.to_string();
					newOverWriteData.deny = value.get_document().view()["deny"].get_utf8().value.to_string();
					newOverWriteData.id = value.get_document().view()["id"].get_utf8().value.to_string();
					newOverWriteData.type = static_cast<PermissionOverwritesType>(value.get_document().view()["type"].get_int32().value);
					guildMemberData.previousPermissionOverwrites.push_back(newOverWriteData);
				}

				for (auto& value: docValue.view()["invites"].get_array().value) {
					DiscordInviteData newData{};
					newData.inviteCode = value.get_document().view()["inviteCode"].get_utf8().value.to_string();
					newData.invitesUsed = value.get_document().view()["invitesUsed"].get_int32().value;
					newData.maxInvites = value.get_document().view()["maxInvites"].get_int32().value;
					guildMemberData.invites.push_back(newData);
				}
				return guildMemberData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseGuildMemberData()");
				return guildMemberData;
			}
		};

	  public:
		static void initialize(std::string botUserIdNew) {
			DatabaseManagerAgent::botUserId = botUserIdNew;
			auto newClient = DatabaseManagerAgent::getClient();
			mongocxx::database newDataBase = (*newClient)[DatabaseManagerAgent::botUserId];
			mongocxx::collection newCollection = newDataBase[DatabaseManagerAgent::botUserId];
		}

		static mongocxx::pool::entry getClient() {
			return DatabaseManagerAgent::thePool.acquire();
		}

		static DatabaseReturnValue submitWorkloadAndGetResults(DatabaseWorkload workload) {
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex };
			DatabaseReturnValue newData{};
			try {
				mongocxx::pool::entry thePtr = DatabaseManagerAgent::getClient();
				auto newDataBase = (*thePtr)[DatabaseManagerAgent::botUserId];
				auto newCollection = newDataBase[DatabaseManagerAgent::botUserId];
				switch (workload.workloadType) {
					case (DatabaseWorkloadType::DISCORD_USER_WRITE): {
						auto doc = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.userData.userId));
						auto resultNew = newCollection.find_one(document.view());
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_USER_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.userData.userId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordUserData userData = DatabaseManagerAgent::parseUserData(*resultNew.get_ptr());
							newData.discordUser = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_WRITE): {
						auto doc = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildData.guildId));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildData.guildId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordGuildData guildData = DatabaseManagerAgent::parseGuildData(*resultNew.get_ptr());
							newData.discordGuild = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_MEMBER_WRITE): {
						auto doc = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_MEMBER_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordGuildMemberData guildMemberData = DatabaseManagerAgent::parseGuildMemberData(*resultNew.get_ptr());
							newData.discordGuildMember = guildMemberData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
				}
			} catch (...) {
				reportException("DatabaseManagerAgent::run() Error: ");
				return newData;
			}
			return newData;
		}
	};

	class DiscordUser {
	  public:
		static int32_t guildCount;

		DiscordUserData data{};

		DiscordUser(std::string userNameNew, std::string userIdNew) {
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB();
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_USER_WRITE;
			workload.userData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_USER_READ;
			workload.userData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != "") {
				this->data = result.discordUser;
			}
		}
	};

	class DiscordGuild {
	  public:
		DiscordGuildData data{};

		DiscordGuild(GuildData guildData) {
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB();
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_WRITE;
			workload.guildData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_READ;
			workload.guildData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != "") {
				this->data = result.discordGuild;
			}
		}
	};

	class DiscordGuildMember {
	  public:
		DiscordGuildMemberData data{};

		DiscordGuildMember(GuildMemberData guildMemberData) {
			this->data.guildMemberId = guildMemberData.user.id;
			this->data.guildId = guildMemberData.guildId;
			this->data.globalId = this->data.guildId + " + " + this->data.guildMemberId;
			this->getDataFromDB();
			if (guildMemberData.nick == "") {
				this->data.displayName = guildMemberData.user.userName;
				this->data.guildMemberMention = "<@" + this->data.guildMemberId + ">";
			} else {
				this->data.displayName = guildMemberData.nick;
				this->data.guildMemberMention = "<@!" + this->data.guildMemberId + ">";
			}
			this->data.userName = guildMemberData.user.userName;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_MEMBER_WRITE;
			workload.guildMemberData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_MEMBER_READ;
			workload.guildMemberData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != "") {
				this->data = result.discordGuildMember;
			}
		}
	};

	mongocxx::instance DatabaseManagerAgent::instance{};
	mongocxx::pool DatabaseManagerAgent::thePool{ mongocxx::uri{} };
	std::string DatabaseManagerAgent::botUserId{ "" };
	std::mutex DatabaseManagerAgent::workloadMutex{};
	int32_t DiscordUser::guildCount{ 0 };

}