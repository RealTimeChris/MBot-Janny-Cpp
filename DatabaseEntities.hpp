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
		jsonifier::string inviteCode{};
		int32_t invitesUsed{ 0 };
		int32_t maxInvites{ 0 };
	};

	struct DeletionChannelData {
		int32_t minutesToWaitUntilDeleted{ 0 };
		bool currentlyBeingDeleted{ false };
		int32_t numberOfMessagesToSave{ 0 };
		Snowflake deletionMessageId{ 0 };
		Snowflake channelId{ 0 };
	};

	struct DiscordUserData {
		jsonifier::vector<uint64_t> botCommanders{ 0, 0, 0 };
		jsonifier::string userName{};
		Snowflake userId{ 0 };
	};

	struct BanInfoLite {
		jsonifier::string avatarUrl{};
		jsonifier::string userName{};
		jsonifier::string bannedAt{};
		jsonifier::string reason{};
		Snowflake userId{ 0 };
	};

	struct UserBanInfo {
		jsonifier::vector<BanInfoLite> userBans{};
		Snowflake userId{ 0 };
	};

	struct RoleManager {
		jsonifier::vector<uint64_t> theRoles{};
		std::string message{};
		Snowflake channelId{ 0 };
		Snowflake messageId{ 0 };
	};

	struct Log {
		jsonifier::string loggingChannelName{};
		Snowflake loggingChannelId{ 0 };
		jsonifier::string nameSmall{};
		jsonifier::string name{};
		bool enabled{ false };
	};

	struct DiscordGuildData {
		DiscordGuildData() {
			Log log{
				.loggingChannelName = "",
				.loggingChannelId	= Snowflake{ 0 },
				.nameSmall			= "",
				.name				= "",
			};
			log.name	  = "GuildData Ban Add";
			log.nameSmall = "guildbanadd";
			this->logs.emplace_back(log);
			log.name	  = "GuildData Ban remove";
			log.nameSmall = "guildbanremove";
			this->logs.emplace_back(log);
			log.name	  = "GuildData Member Add";
			log.nameSmall = "guildmemberadd";
			this->logs.emplace_back(log);
			log.name	  = "GuildData Member remove";
			log.nameSmall = "guildmemberremove";
			this->logs.emplace_back(log);
			log.name	  = "Display Name Change";
			log.nameSmall = "displaynamechange";
			this->logs.emplace_back(log);
			log.name	  = "Nickname Change";
			log.nameSmall = "nicknamechange";
			this->logs.emplace_back(log);
			log.name	  = "RoleData Add Or remove";
			log.nameSmall = "roleaddorremove";
			this->logs.emplace_back(log);
			log.name	  = "Invite Create";
			log.nameSmall = "invitecreate";
			this->logs.emplace_back(log);
			log.name	  = "MessageData Delete";
			log.nameSmall = "messagedelete";
			this->logs.emplace_back(log);
			log.name	  = "MessageData Delete Bulk";
			log.nameSmall = "messagedeletebulk";
			this->logs.emplace_back(log);
			log.name	  = "RoleData Create";
			log.nameSmall = "rolecreate";
			this->logs.emplace_back(log);
			log.name	  = "RoleData Delete";
			log.nameSmall = "roledelete";
			this->logs.emplace_back(log);
			log.name	  = "Username Change";
			log.nameSmall = "usernamechange";
			this->logs.emplace_back(log);
		}
		jsonifier::vector<DeletionChannelData> deletionChannels{};
		Snowflake inviteReportingChannelId{ 0 };
		jsonifier::vector<Snowflake> defaultRoleIds{};
		jsonifier::vector<UserBanInfo> userBanInfo{};
		jsonifier::vector<Snowflake> trackedUsers{};
		jsonifier::string borderColor{ "FEFEFE" };
		jsonifier::vector<Snowflake> ghostedIds{};
		int32_t vanityInviteUses{ 0 };
		uint32_t memberCount{ 0 };
		RoleManager roleManager{};
		jsonifier::string guildName{};
		jsonifier::vector<Log> logs{};
		Snowflake guildId{ 0 };
	};

	/// A Permission overwrite, for a given ChannelData. \brief A Permission overwrite, for a given ChannelData.
	struct PermissionOverWriteData {
		PermissionOverwritesType type{};///< RoleData or UserData type.
		Snowflake channelId{ 0 };///< ChannelData id for which ChannelData this overwrite beint64_ts to.
		Permissions allow{};///< Collection of Permissions to allow.
		Permissions deny{};///< Collection of Permissions to deny.
		Snowflake id{ 0 };///< Snowflake of the permission overwrite.
	};

	struct DiscordGuildMemberData {
		jsonifier::vector<PermissionOverWriteData> previousPermissionOverwrites{};
		jsonifier::vector<jsonifier::string> invitedMemberIds{};
		jsonifier::vector<DiscordInviteData> invites{};
		jsonifier::vector<uint64_t> previousRoleIds{};
		jsonifier::string guildMemberMention{};
		Snowflake guildMemberId{ 0 };
		uint32_t totalInvites{ 0 };
		jsonifier::string displayName{};
		jsonifier::string globalId{};
		jsonifier::string userName{};
		Snowflake guildId{ 0 };
	};

	enum class DatabaseWorkloadType {
		Discord_User_Write		   = 0,
		Discord_User_Read		   = 1,
		Discord_Guild_Write		   = 2,
		Discord_Guild_Read		   = 3,
		Discord_Guild_Member_Write = 4,
		Discord_Guild_Member_Read  = 5
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
		inline void initialize(Snowflake botUserIdNew) {
			DatabaseManagerAgent::botUserId	   = botUserIdNew;
			auto newClient					   = DatabaseManagerAgent::getClient();
			mongocxx::database newDataBase	   = (*newClient)[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
			mongocxx::collection newCollection = newDataBase[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
		}

		inline mongocxx::pool::entry getClient() {
			return DatabaseManagerAgent::thePool.acquire();
		}

		inline DatabaseReturnValue submitWorkloadAndGetResults(DatabaseWorkload workload) {
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex01 };
			while (DatabaseManagerAgent::botUserId == 0) {
				std::this_thread::sleep_for(1ms);
			}
			DatabaseReturnValue newData{};
			mongocxx::pool::entry thePtr = DatabaseManagerAgent::getClient();
			try {
				auto newDataBase   = (*thePtr)[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
				auto newCollection = newDataBase[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
				switch (workload.workloadType) {
					case (DatabaseWorkloadType::Discord_User_Write): {
						auto doc = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNew	 = newCollection.find_one(document.view());
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_User_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordUserData userData = DatabaseManagerAgent::parseUserData(*resultNew);
							newData.discordUser		 = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Write): {
						auto doc = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordGuildData guildData = DatabaseManagerAgent::parseGuildData(*resultNew);
							newData.discordGuild	   = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Member_Write): {
						auto doc = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Member_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordGuildMemberData guildMemberData = DatabaseManagerAgent::parseGuildMemberData(*resultNew);
							newData.discordGuildMember			   = guildMemberData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
				}
				return newData;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::run()" << error.what() << std::endl;
				return newData;
			}
		}

	  protected:
		mongocxx::instance instance{};
		std::mutex workloadMutex01{};
		mongocxx::pool thePool{};
		Snowflake botUserId{};

		static bsoncxx::builder::basic::document convertUserDataToDBDoc(DiscordUserData discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(discordUserData.userId.operator const uint64_t&()))));
				buildDoc.append(kvp("userId", bsoncxx::types::b_int64(static_cast<int64_t>(discordUserData.userId.operator const uint64_t&()))));
				buildDoc.append(kvp("userName", discordUserData.userName));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value)));
					}
				}));
				return buildDoc;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::convertUserDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		}

		static DiscordUserData parseUserData(bsoncxx::document::value docValue) {
			DiscordUserData userData{};
			try {
				userData.userName		= docValue.view()["userName"].get_utf8().value;
				userData.userId			= static_cast<uint64_t>(docValue.view()["userId"].get_int64().value);
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				jsonifier::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.emplace_back(static_cast<uint64_t>(value.get_int64().value));
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::parseUserData()" << error.what() << std::endl;
				return userData;
			}
		}

		static bsoncxx::builder::basic::document convertGuildDataToDBDoc(DiscordGuildData discordGuildData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildName", discordGuildData.guildName));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int32(static_cast<int32_t>(discordGuildData.memberCount))));
				buildDoc.append(kvp("vanityInviteUses", bsoncxx::types::b_int32(static_cast<int32_t>(discordGuildData.vanityInviteUses))));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor));
				buildDoc.append(kvp("roleManager", [discordGuildData](bsoncxx::builder::basic::sub_document sub_document01) {
					sub_document01.append(kvp("theRoles", [discordGuildData](bsoncxx::builder::basic::sub_array sub_array01) {
						for (auto& value: discordGuildData.roleManager.theRoles) {
							sub_array01.append(bsoncxx::types::b_int64(static_cast<int64_t>(value)));
						}
					}));
					auto theValue = static_cast<Snowflake>(discordGuildData.roleManager.channelId);
					sub_document01.append(kvp("channelId", bsoncxx::types::b_int64(static_cast<int64_t>(theValue.operator const uint64_t&()))));
					theValue = static_cast<Snowflake>(discordGuildData.roleManager.messageId);
					sub_document01.append(kvp("messageId", bsoncxx::types::b_int64(static_cast<int64_t>(theValue.operator const uint64_t&()))));

					sub_document01.append(kvp("message", bsoncxx::types::b_utf8{ discordGuildData.roleManager.message }));
				}));
				buildDoc.append(kvp("ghostedIds", [discordGuildData](bsoncxx::builder::basic::sub_array subArray01) {
					for (auto& value02: discordGuildData.ghostedIds) {
						subArray01.append(bsoncxx::types::b_int64(static_cast<int64_t>(value02.operator const uint64_t&())));
					}
				}));
				buildDoc.append(
					kvp("inviteReportingChannelId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.inviteReportingChannelId.operator const uint64_t&()))));
				buildDoc.append(kvp("defaultRoleIds",
					[discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
						for (auto& value: discordGuildData.defaultRoleIds) {
							subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value.operator const uint64_t&())));
						}
					}

					));
				buildDoc.append(kvp("deletionChannels", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.deletionChannels) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("channelId", bsoncxx::types::b_int64(static_cast<int64_t>(value.channelId.operator const uint64_t&()))),
								kvp("currentlyBeingDeleted", bsoncxx::types::b_bool(static_cast<uint64_t>(value.currentlyBeingDeleted))),
								kvp("deletionMessageId", bsoncxx::types::b_int64(static_cast<int64_t>(value.deletionMessageId.operator const uint64_t&()))),
								kvp("numberOfMessagesToSave", value.numberOfMessagesToSave),
								kvp("minutesToWaitUntilDeleted", bsoncxx::types::b_int32(static_cast<int32_t>(value.minutesToWaitUntilDeleted))));
						});
					}
				}));
				buildDoc.append(kvp("userBanInfo", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.userBanInfo) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("userId", bsoncxx::types::b_int64(static_cast<int64_t>(value.userId.operator const uint64_t&()))));
							subDocument.append(kvp("userBans", [value](bsoncxx::builder::basic::sub_array subArray02) {
								for (auto& value02: value.userBans) {
									subArray02.append([=](bsoncxx::builder::basic::sub_document subDocument02) {
										subDocument02.append(kvp("reason", bsoncxx::types::b_utf8{ value02.reason }));
										subDocument02.append(kvp("userName", bsoncxx::types::b_utf8{ value02.userName }));
										subDocument02.append(kvp("userId", bsoncxx::types::b_int64(static_cast<int64_t>(value02.userId.operator const uint64_t&()))));
										subDocument02.append(kvp("bannedAt", bsoncxx::types::b_utf8{ value02.bannedAt }));
										subDocument02.append(kvp("avatarUrl", bsoncxx::types::b_utf8{ value02.avatarUrl }));
									});
								}
							}));
						});
					}
				}));
				buildDoc.append(kvp("logs", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.logs) {
						subArray.append([=](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("enabled", bsoncxx::types::b_bool(value.enabled)),
								kvp("loggingChannelId", bsoncxx::types::b_int64(static_cast<int64_t>(value.loggingChannelId.operator const uint64_t&()))),
								kvp("loggingChannelName", bsoncxx::types::b_utf8{ value.loggingChannelName }), kvp("name", bsoncxx::types::b_utf8{ value.name }),
								kvp("nameSmall", bsoncxx::types::b_utf8{ value.nameSmall }));
						});
					}
				}));

				buildDoc.append(kvp("trackedUsers", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.trackedUsers) {
						subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value.operator const uint64_t&())));
					}
				}));
				return buildDoc;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		static DiscordGuildData parseGuildData(bsoncxx::document::value docValue) {
			DiscordGuildData guildData{};
			try {
				guildData.inviteReportingChannelId = static_cast<uint64_t>(docValue.view()["inviteReportingChannelId"].get_int64().value);
				if (docValue.view()["ghostedIds"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: docValue.view()["ghostedIds"].get_array().value) {
						guildData.ghostedIds.emplace_back(Snowflake{ static_cast<size_t>(value.get_value().get_int64().value) });
					}
				}

				guildData.borderColor	   = docValue.view()["borderColor"].get_utf8().value;
				guildData.vanityInviteUses = docValue.view()["vanityInviteUses"].get_int32().value;
				guildData.guildName		   = docValue.view()["guildName"].get_utf8().value;
				guildData.guildId		   = static_cast<uint64_t>(docValue.view()["guildId"].get_int64().value);
				guildData.memberCount	   = static_cast<uint32_t>(docValue.view()["memberCount"].get_int32().value);
				for (auto& value: docValue.view()["deletionChannels"].get_array().value) {
					DeletionChannelData newData;
					newData.channelId			   = static_cast<uint64_t>(value.get_document().view()["channelId"].get_int64().value);
					newData.currentlyBeingDeleted  = value.get_document().view()["currentlyBeingDeleted"].get_bool().value;
					newData.deletionMessageId	   = static_cast<uint64_t>(value.get_document().view()["deletionMessageId"].get_int64().value);
					newData.numberOfMessagesToSave = value.get_document().view()["numberOfMessagesToSave"].get_int32().value;
					if (value.get_document().view()["minutesToWaitUntilDeleted"].type() != bsoncxx::v_noabi::type::k_null) {
						newData.minutesToWaitUntilDeleted = value.get_document().view()["minutesToWaitUntilDeleted"].get_int32().value;
					}
					guildData.deletionChannels.emplace_back(newData);
				}
				for (auto& value: docValue.view()["userBanInfo"].get_array().value) {
					UserBanInfo newData;
					newData.userId = static_cast<uint64_t>(value["userId"].get_int64().value);
					for (auto& value2: value["userBans"].get_array().value) {
						BanInfoLite newData02;
						newData02.userId	= static_cast<uint64_t>(value2["userId"].get_int64().value);
						newData02.userName	= value2["userName"].get_utf8().value;
						newData02.avatarUrl = value2["avatarUrl"].get_utf8().value;
						newData02.bannedAt	= value2["bannedAt"].get_utf8().value;
						newData02.reason	= value2["reason"].get_utf8().value;
						newData.userBans.emplace_back(newData02);
					}
					guildData.userBanInfo.emplace_back(newData);
				}
				for (auto& value: docValue.view()["trackedUsers"].get_array().value) {
					guildData.trackedUsers.emplace_back(Snowflake{ static_cast<size_t>(value.get_int64().value) });
				}
				auto roleManager = docValue.view()["roleManager"].get_document();
				if (roleManager.view()["theRoles"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: roleManager.view()["theRoles"].get_array().value) {
						guildData.roleManager.theRoles.emplace_back(static_cast<uint64_t>(value.get_int64().value));
					}
				}

				guildData.roleManager.channelId = static_cast<uint64_t>(roleManager.view()["channelId"].get_int64().value);
				guildData.roleManager.messageId = static_cast<uint64_t>(roleManager.view()["messageId"].get_int64().value);
				//guildData.roleManager.message	= roleManager.view()["message"].get_utf8().value;
				for (auto& value: docValue.view()["defaultRoleIds"].get_array().value) {
					guildData.defaultRoleIds.emplace_back(Snowflake{ static_cast<uint64_t>(value.get_int64().value) });
				}
				for (auto& value: docValue.view()["logs"].get_array().value) {
					Log newData;
					newData.enabled			   = value.get_document().value["enabled"].get_bool().value;
					newData.loggingChannelId   = static_cast<uint64_t>(value.get_document().value["loggingChannelId"].get_int64().value);
					newData.loggingChannelName = value.get_document().value["loggingChannelName"].get_utf8().value;
					newData.name			   = value.get_document().value["name"].get_utf8().value;
					newData.nameSmall		   = value.get_document().value["nameSmall"].get_utf8().value;
					for (uint32_t y = 0; y < guildData.logs.size(); y += 1) {
						if (guildData.logs[y].nameSmall == newData.nameSmall) {
							guildData.logs[y] = newData;
						}
					}
				}
				return guildData;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::parseGuildData()" << error.what() << std::endl;
				return guildData;
			}
		};

		static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(DiscordGuildMemberData discordGuildMemberData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("guildMemberMention", discordGuildMemberData.guildMemberMention));
				buildDoc.append(kvp("_id", discordGuildMemberData.globalId));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildMemberData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildMemberId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildMemberData.guildMemberId.operator const uint64_t&()))));
				buildDoc.append(kvp("globalId", discordGuildMemberData.globalId));
				buildDoc.append(kvp("userName", discordGuildMemberData.userName));
				buildDoc.append(kvp("displayName", discordGuildMemberData.displayName));
				buildDoc.append(kvp("totalInvites", bsoncxx::types::b_int32(static_cast<int32_t>(discordGuildMemberData.totalInvites))));
				buildDoc.append(kvp("previousPermissionOverwrites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (PermissionOverWriteData value: discordGuildMemberData.previousPermissionOverwrites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(
								kvp("allow", bsoncxx::types::b_int64(static_cast<int64_t>(std::stoull(Permissions{ value.deny }.operator jsonifier::string().data())))),
								kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(value.type))),
								kvp("deny", bsoncxx::types::b_int64(static_cast<int64_t>(std::stoull(Permissions{ value.deny }.operator jsonifier::string().data())))),
								kvp("id", jsonifier::toString(static_cast<uint64_t>(value.id))));
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
						subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value)));
					}
				}));
				buildDoc.append(kvp("invites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.invites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(kvp("inviteCode", value.inviteCode), kvp("invitesUsed", bsoncxx::types::b_int32(static_cast<int32_t>(value.invitesUsed))),
								kvp("maxInvites", bsoncxx::types::b_int32(static_cast<int32_t>(value.maxInvites))));
						});
					}
				}));
				return buildDoc;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::convertGuildMemberDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		static DiscordGuildMemberData parseGuildMemberData(bsoncxx::document::value docValue) {
			DiscordGuildMemberData guildMemberData{};
			try {
				guildMemberData.guildMemberMention = docValue.view()["guildMemberMention"].get_utf8().value;
				guildMemberData.guildId			   = static_cast<uint64_t>(docValue.view()["guildId"].get_int64().value);
				guildMemberData.displayName		   = docValue.view()["displayName"].get_utf8().value;
				guildMemberData.globalId		   = docValue.view()["globalId"].get_utf8().value;
				guildMemberData.guildMemberId	   = static_cast<uint64_t>(docValue.view()["guildMemberId"].get_int64().value);
				guildMemberData.userName		   = docValue.view()["userName"].get_utf8().value;
				guildMemberData.totalInvites	   = static_cast<uint32_t>(docValue.view()["totalInvites"].get_int32().value);
				for (auto& value: docValue.view()["previousRoleIds"].get_array().value) {
					guildMemberData.previousRoleIds.emplace_back(static_cast<uint64_t>(value.get_int64().value));
				}
				for (auto& value: docValue.view()["invitedMemberIds"].get_array().value) {
					guildMemberData.invitedMemberIds.emplace_back(static_cast<jsonifier::string>(value.get_utf8().value));
				}
				for (auto& value: docValue.view()["previousPermissionOverwrites"].get_array().value) {
					PermissionOverWriteData newOverWriteData{};
					newOverWriteData.allow = static_cast<uint64_t>(value.get_document().view()["allow"].get_int64().value);
					newOverWriteData.deny  = static_cast<uint64_t>(value.get_document().view()["deny"].get_int64().value);
					newOverWriteData.id	   = std::stoull(jsonifier::string{ value.get_document().view()["id"].get_utf8().value }.data());
					newOverWriteData.type  = static_cast<PermissionOverwritesType>(value.get_document().view()["type"].get_int32().value);
					guildMemberData.previousPermissionOverwrites.emplace_back(newOverWriteData);
				}

				for (auto& value: docValue.view()["invites"].get_array().value) {
					DiscordInviteData newData{};
					newData.inviteCode	= value.get_document().view()["inviteCode"].get_utf8().value;
					newData.invitesUsed = value.get_document().view()["invitesUsed"].get_int32().value;
					newData.maxInvites	= value.get_document().view()["maxInvites"].get_int32().value;
					guildMemberData.invites.emplace_back(newData);
				}
				return guildMemberData;
			} catch (const std::exception& error) {
				std::cout << "DatabaseManagerAgent::parseGuildMemberData()" << error.what() << std::endl;
				return guildMemberData;
			}
		};
	};

	class DiscordUser {
	  public:
		static int32_t guildCount;

		DiscordUserData data{};

		inline DiscordUser(DatabaseManagerAgent& other, jsonifier::string userNameNew, Snowflake userIdNew) {
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB(other);
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
		}

		inline void writeDataToDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_User_Write;
			workload.userData	  = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_User_Read;
			workload.userData	  = this->data;
			auto result			  = other.submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != 0) {
				this->data = result.discordUser;
			}
		}
	};

	class DiscordGuild {
	  public:
		inline DiscordGuild() noexcept = default;
		DiscordGuildData data{};

		inline DiscordGuild(DatabaseManagerAgent& other, GuildCacheData guildData) {
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB(other);
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
		}

		inline void writeDataToDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Write;
			workload.guildData	  = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Read;
			workload.guildData	  = this->data;
			auto result			  = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class DiscordGuildMember {
	  public:
		DiscordGuildMemberData data{};

		inline DiscordGuildMember(DatabaseManagerAgent& other, GuildMemberCacheData guildMemberData) {
			this->data.guildMemberId = guildMemberData.user.id;
			this->data.guildId		 = guildMemberData.guildId.operator const uint64_t&();
			this->data.globalId		 = this->data.guildId.operator jsonifier::string() + " + " + this->data.guildMemberId;
			this->getDataFromDB(other);
			if (guildMemberData.nick == "") {
				this->data.displayName		  = guildMemberData.getUserData().userName;
				this->data.guildMemberMention = "<@" + this->data.guildMemberId + ">";
			} else {
				this->data.displayName		  = guildMemberData.nick;
				this->data.guildMemberMention = "<@!" + this->data.guildMemberId + ">";
			}
			this->data.userName = guildMemberData.getUserData().userName;
		}

		inline void writeDataToDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType	 = DatabaseWorkloadType::Discord_Guild_Member_Write;
			workload.guildMemberData = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType	 = DatabaseWorkloadType::Discord_Guild_Member_Read;
			workload.guildMemberData = this->data;
			auto result				 = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != "") {
				this->data = result.discordGuildMember;
			}
		}
	};

	inline static DatabaseManagerAgent managerAgent{};
}
