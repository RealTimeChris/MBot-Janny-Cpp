// DatabaseEntities.hpp - Database stuff.
// May 24, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

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
		uint64_t deletionMessageId{ 0 };
		uint64_t channelId{ 0 };
	};

	struct DiscordUserData {
		std::vector<uint64_t> botCommanders{ 0, 0, 0 };
		std::string userName{ "" };
		uint64_t userId{ 0 };
	};

	struct BanInfoLite {
		std::string avatarUrl{ "" };
		std::string userName{ "" };
		std::string bannedAt{ "" };
		std::string reason{ "" };
		uint64_t userId{ 0 };
	};

	struct UserBanInfo {
		std::vector<BanInfoLite> userBans{};
		uint64_t userId{ 0 };
	};

	struct RoleManager {
		std::vector<uint64_t> theRoles{};
		std::string message{ "" };
		uint64_t channelId{ 0 };
		uint64_t messageId{ 0 };
	};

	struct Log {
		std::string loggingChannelName{ "" };
		uint64_t loggingChannelId{ 0 };
		std::string nameSmall{ "" };
		std::string name{ "" };
		bool enabled{ false };
	};

	struct DiscordGuildData {
		DiscordGuildData() {
			Log log{
				.loggingChannelName = "",
				.loggingChannelId = 0,
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
		uint64_t inviteReportingChannelId{ 0 };
		std::vector<uint64_t> defaultRoleIds{};
		std::vector<UserBanInfo> userBanInfo{};
		std::vector<uint64_t> trackedUsers{};
		std::string borderColor{ "FEFEFE" };
		std::vector<uint64_t> ghostedIds{};
		int32_t vanityInviteUses{ 0 };
		std::string guildName{ "" };
		uint32_t memberCount{ 0 };
		RoleManager roleManager{};
		std::vector<Log> logs{};
		uint64_t guildId{ 0 };
	};

	/// A Permission overwrite, for a given Channel. \brief A Permission overwrite, for a given Channel.
	struct PermissionOverWriteData {
		PermissionOverwritesType type{};///< Role or User type.
		uint64_t channelId{ 0 };///< Channel id for which Channel this overwrite beint64_ts to.
		Permissions allow{ "" };///< Collection of Permissions to allow.
		Permissions deny{ "" };///< Collection of Permissions to deny.
		uint64_t id{ 0 };///< Id of the permission overwrite.
	};

	struct DiscordGuildMemberData {
		std::vector<PermissionOverWriteData> previousPermissionOverwrites{};
		std::vector<std::string> invitedMemberIds{};
		std::vector<DiscordInviteData> invites{};
		std::vector<uint64_t> previousRoleIds{};
		std::string guildMemberMention{ "" };
		std::string displayName{ "" };
		uint64_t guildMemberId{ 0 };
		uint32_t totalInvites{ 0 };
		std::string globalId{ "" };
		std::string userName{ "" };
		uint64_t guildId{ 0 };
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
	  public:
		static void initialize(uint64_t botUserIdNew) {
			DatabaseManagerAgent::botUserId = botUserIdNew;
			auto newClient = DatabaseManagerAgent::getClient();
			mongocxx::database newDataBase = (*newClient)[std::to_string(DatabaseManagerAgent::botUserId)];
			mongocxx::collection newCollection = newDataBase[std::to_string(DatabaseManagerAgent::botUserId)];
		}

		static mongocxx::pool::entry getClient() {
			return DatabaseManagerAgent::thePool.acquire();
		}

		static DatabaseReturnValue submitWorkloadAndGetResults(DatabaseWorkload workload) {
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex };
			DatabaseReturnValue newData{};
			try {
				mongocxx::pool::entry thePtr = DatabaseManagerAgent::getClient();
				auto newDataBase = (*thePtr)[std::to_string(DatabaseManagerAgent::botUserId)];
				auto newCollection = newDataBase[std::to_string(DatabaseManagerAgent::botUserId)];
				switch (workload.workloadType) {
					case (DatabaseWorkloadType::DISCORD_USER_WRITE): {
						auto doc = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.userData.userId)));
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
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.userData.userId)));
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
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.guildData.guildId)));
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
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.guildData.guildId)));
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
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId.c_str()));
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
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId.c_str()));
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
				return newData;
			} catch (...) {
				reportException("DatabaseManagerAgent::run() Error: ");
				return newData;
			}
			
		}

	  protected:
		static mongocxx::instance instance;
		static std::mutex workloadMutex;
		static mongocxx::pool thePool;
		static uint64_t botUserId;

		static bsoncxx::builder::basic::document convertUserDataToDBDoc(DiscordUserData discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(discordUserData.userId)));
				buildDoc.append(kvp("userId", bsoncxx::types::b_int64(discordUserData.userId)));
				buildDoc.append(kvp("userName", discordUserData.userName.c_str()));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(bsoncxx::types::b_int64(value));
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
				userData.userId = static_cast<uint64_t>(docValue.view()["userId"].get_int64().value);
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				std::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.push_back(value.get_int64().value);
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
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(discordGuildData.guildId)));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(discordGuildData.guildId)));
				buildDoc.append(kvp("guildName", discordGuildData.guildName.c_str()));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int32(discordGuildData.memberCount)));
				buildDoc.append(kvp("vanityInviteUses", bsoncxx::types::b_int32(discordGuildData.vanityInviteUses)));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor.c_str()));
				buildDoc.append(kvp("roleManager", [discordGuildData](bsoncxx::builder::basic::sub_document sub_document01) {
					sub_document01.append(kvp("theRoles", [discordGuildData](bsoncxx::builder::basic::sub_array sub_array01) {
						for (auto& value: discordGuildData.roleManager.theRoles) {
							sub_array01.append(bsoncxx::types::b_int64(value));
						}
					}));
					sub_document01.append(kvp("channelId", bsoncxx::types::b_int64(discordGuildData.roleManager.channelId)));

					sub_document01.append(kvp("messageId", bsoncxx::types::b_int64(discordGuildData.roleManager.messageId)));

					sub_document01.append(kvp("message", discordGuildData.roleManager.message.c_str()));
				}));
				buildDoc.append(kvp("ghostedIds", [discordGuildData](bsoncxx::builder::basic::sub_array subArray01) {
					for (auto& value02: discordGuildData.ghostedIds) {
						subArray01.append(bsoncxx::types::b_int64(value02));
					}
				}));
				buildDoc.append(kvp("inviteReportingChannelId", bsoncxx::types::b_int64(discordGuildData.inviteReportingChannelId)));
				buildDoc.append(kvp("defaultRoleIds",
					[discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
						for (auto& value: discordGuildData.defaultRoleIds) {
							subArray.append(bsoncxx::types::b_int64(value));
						}
					}

					));
				buildDoc.append(kvp("deletionChannels", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.deletionChannels) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("channelId", bsoncxx::types::b_int64(value.channelId)),
								kvp("currentlyBeingDeleted", bsoncxx::types::b_bool(value.currentlyBeingDeleted)),
								kvp("deletionMessageId", bsoncxx::types::b_int64(value.deletionMessageId)), kvp("numberOfMessagesToSave", value.numberOfMessagesToSave),
								kvp("minutesToWaitUntilDeleted", bsoncxx::types::b_int32(value.minutesToWaitUntilDeleted)));
						});
					}
				}));
				buildDoc.append(kvp("userBanInfo", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.userBanInfo) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("userId", bsoncxx::types::b_int64(value.userId)));
							subDocument.append(kvp("userBans", [value](bsoncxx::builder::basic::sub_array subArray02) {
								for (auto& value02: value.userBans) {
									subArray02.append([=](bsoncxx::builder::basic::sub_document subDocument02) {
										subDocument02.append(kvp("reason", value02.reason.c_str()));
										subDocument02.append(kvp("userName", value02.userName.c_str()));
										subDocument02.append(kvp("userId", bsoncxx::types::b_int64(value02.userId)));
										subDocument02.append(kvp("bannedAt", value02.bannedAt.c_str()));
										subDocument02.append(kvp("avatarUrl", value02.avatarUrl.c_str()));
									});
								}
							}));
						});
					}
				}));
				buildDoc.append(kvp("logs", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.logs) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("enabled", bsoncxx::types::b_bool(value.enabled)), kvp("loggingChannelId", bsoncxx::types::b_int64(value.loggingChannelId)),
								kvp("loggingChannelName", value.loggingChannelName.c_str()), kvp("name", value.name.c_str()), kvp("nameSmall", value.nameSmall.c_str()));
						});
					}
				}));

				buildDoc.append(kvp("trackedUsers", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.trackedUsers) {
						subArray.append(bsoncxx::types::b_int64(value));
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
				guildData.inviteReportingChannelId = docValue.view()["inviteReportingChannelId"].get_int64().value;
				if (docValue.view()["ghostedIds"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: docValue.view()["ghostedIds"].get_array().value) {
						guildData.ghostedIds.push_back(value.get_value().get_int64().value);
					}
				}

				guildData.borderColor = docValue.view()["borderColor"].get_utf8().value.to_string();
				guildData.vanityInviteUses = docValue.view()["vanityInviteUses"].get_int32().value;
				guildData.guildName = docValue.view()["guildName"].get_utf8().value.to_string();
				guildData.guildId = docValue.view()["guildId"].get_int64().value;
				guildData.memberCount = docValue.view()["memberCount"].get_int32().value;
				for (auto& value: docValue.view()["deletionChannels"].get_array().value) {
					DeletionChannelData newData;
					newData.channelId = value.get_document().view()["channelId"].get_int64().value;					
					newData.currentlyBeingDeleted = value.get_document().view()["currentlyBeingDeleted"].get_bool().value;
					newData.deletionMessageId = value.get_document().view()["deletionMessageId"].get_int64().value;
					newData.numberOfMessagesToSave = value.get_document().view()["numberOfMessagesToSave"].get_int32().value;
					if (value.get_document().view()["minutesToWaitUntilDeleted"].type() != bsoncxx::v_noabi::type::k_null) {
						newData.minutesToWaitUntilDeleted = value.get_document().view()["minutesToWaitUntilDeleted"].get_int32().value;
					}
					guildData.deletionChannels.push_back(newData);
				}
				for (auto& value: docValue.view()["userBanInfo"].get_array().value) {
					UserBanInfo newData;
					newData.userId = value["userId"].get_int64().value;					
					for (auto& value2: value["userBans"].get_array().value) {
						BanInfoLite newData02;
						newData02.userId = value2["userId"].get_int64().value;
						newData02.userName = value2["userName"].get_utf8().value.to_string();
						newData02.avatarUrl = value2["avatarUrl"].get_utf8().value.to_string();
						newData02.bannedAt = value2["bannedAt"].get_utf8().value.to_string();
						newData02.reason = value2["reason"].get_utf8().value.to_string();
						newData.userBans.push_back(newData02);
					}
					guildData.userBanInfo.push_back(newData);
				}
				for (auto& value: docValue.view()["trackedUsers"].get_array().value) {
					guildData.trackedUsers.push_back(value.get_int64().value);
				}
				auto roleManager = docValue.view()["roleManager"].get_document();
				if (roleManager.view()["theRoles"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: roleManager.view()["theRoles"].get_array().value) {
						guildData.roleManager.theRoles.push_back(value.get_int64().value);
					}
				}

				guildData.roleManager.channelId = roleManager.view()["channelId"].get_int64().value;
				guildData.roleManager.messageId = roleManager.view()["messageId"].get_int64().value;
				guildData.roleManager.message = roleManager.view()["message"].get_utf8().value.to_string();
				for (auto& value: docValue.view()["defaultRoleIds"].get_array().value) {
					guildData.defaultRoleIds.push_back(value.get_int64().value);
				}
				for (auto& value: docValue.view()["logs"].get_array().value) {
					Log newData;
					newData.enabled = value.get_document().value["enabled"].get_bool().value;
					newData.loggingChannelId = value.get_document().value["loggingChannelId"].get_int64().value;
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
				buildDoc.append(kvp("guildMemberMention", discordGuildMemberData.guildMemberMention.c_str()));
				buildDoc.append(kvp("_id", discordGuildMemberData.globalId.c_str()));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(discordGuildMemberData.guildId)));
				buildDoc.append(kvp("guildMemberId", bsoncxx::types::b_int64(discordGuildMemberData.guildMemberId)));
				buildDoc.append(kvp("globalId", discordGuildMemberData.globalId.c_str()));
				buildDoc.append(kvp("userName", discordGuildMemberData.userName.c_str()));
				buildDoc.append(kvp("displayName", discordGuildMemberData.displayName.c_str()));
				buildDoc.append(kvp("totalInvites", bsoncxx::types::b_int32(discordGuildMemberData.totalInvites)));
				buildDoc.append(kvp("previousPermissionOverwrites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (PermissionOverWriteData value: discordGuildMemberData.previousPermissionOverwrites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("allow", bsoncxx::types::b_utf8(static_cast<std::string>(static_cast<StringWrapper>(value.allow)).c_str())),
								kvp("type", bsoncxx::types::b_int64(( int64_t )value.type)),
								kvp("deny", bsoncxx::types::b_utf8(static_cast<std::string>(static_cast<StringWrapper>(value.deny)).c_str())),
								kvp("id", std::to_string(value.id).c_str()));
						});
					}
				}));
				buildDoc.append(kvp("invitedMemberIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.invitedMemberIds) {
						subArray.append(value.c_str());
					}
				}));
				buildDoc.append(kvp("previousRoleIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.previousRoleIds) {
						subArray.append(bsoncxx::types::b_int64(value));
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
				guildMemberData.guildId = docValue.view()["guildId"].get_int64().value;
				guildMemberData.displayName = docValue.view()["displayName"].get_utf8().value.to_string();
				guildMemberData.globalId = docValue.view()["globalId"].get_utf8().value.to_string();
				guildMemberData.guildMemberId = docValue.view()["guildMemberId"].get_int64().value;
				guildMemberData.userName = docValue.view()["userName"].get_utf8().value.to_string();
				guildMemberData.totalInvites = docValue.view()["totalInvites"].get_int32().value;
				for (auto& value: docValue.view()["previousRoleIds"].get_array().value) {
					guildMemberData.previousRoleIds.push_back(value.get_int64().value);
				}
				for (auto& value: docValue.view()["invitedMemberIds"].get_array().value) {
					guildMemberData.invitedMemberIds.push_back(value.get_utf8().value.to_string());
				}
				for (auto& value: docValue.view()["previousPermissionOverwrites"].get_array().value) {
					PermissionOverWriteData newOverWriteData{};
					newOverWriteData.allow = value.get_document().view()["allow"].get_utf8().value.to_string();
					newOverWriteData.deny = value.get_document().view()["deny"].get_utf8().value.to_string();
					newOverWriteData.id = value.get_document().view()["id"].get_int64().value;
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
	};

	class DiscordUser {
	  public:
		static int32_t guildCount;

		DiscordUserData data{};

		DiscordUser(std::string userNameNew, uint64_t userIdNew) {
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
			if (result.discordUser.userId != 0) {
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
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class DiscordGuildMember {
	  public:
		DiscordGuildMemberData data{};

		DiscordGuildMember(GuildMemberData guildMemberData) {
			this->data.guildMemberId = guildMemberData.id;
			this->data.guildId = guildMemberData.guildId;
			this->data.globalId = std::to_string(this->data.guildId) + " + " + std::to_string(this->data.guildMemberId);
			this->getDataFromDB();
			if (guildMemberData.nick == "") {
				this->data.displayName = guildMemberData.userName;
				this->data.guildMemberMention = "<@" + std::to_string(this->data.guildMemberId) + ">";
			} else {
				this->data.displayName = guildMemberData.nick;
				this->data.guildMemberMention = "<@" + std::to_string(this->data.guildMemberId) + ">";
			}
			this->data.userName = guildMemberData.userName;
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
	std::mutex DatabaseManagerAgent::workloadMutex{};
	uint64_t DatabaseManagerAgent::botUserId{ 0 };
	int32_t DiscordUser::guildCount{ 0 };

}