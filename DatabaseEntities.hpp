// DatabaseEntities.hpp - Database stuff.
// may 24, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

#include <discordcoreapi/Index.hpp>

namespace discord_core_api {

	struct discord_invite_data {
		jsonifier::string inviteCode{};
		int64_t invitesUsed{ 0 };
		int64_t maxInvites{ 0 };
	};

	struct deletion_channel_data {
		int64_t minutesToWaitUntilDeleted{ 0 };
		bool currentlyBeingDeleted{ false };
		int64_t numberOfMessagesToSave{ 0 };
		snowflake deletionMessageId{ 0 };
		snowflake channelId{ 0 };
	};

	struct discord_user_data {
		jsonifier::vector<uint64_t> botCommanders{ 0, 0, 0 };
		jsonifier::string userName{};
		snowflake userId{ 0 };
	};

	struct ban_info_lite {
		jsonifier::string avatarUrl{};
		jsonifier::string userName{};
		jsonifier::string bannedAt{};
		jsonifier::string reason{};
		snowflake userId{ 0 };
	};

	struct user_ban_info {
		jsonifier::vector<ban_info_lite> userBans{};
		snowflake userId{ 0 };
	};

	struct role_manager {
		jsonifier::vector<uint64_t> theRoles{};
		jsonifier::string message{ "test" };
		snowflake channelId{ 0 };
		snowflake messageId{ 0 };
	};

	struct log {
		jsonifier::string loggingChannelName{ "test" };
		snowflake loggingChannelId{};
		jsonifier::string nameSmall{};
		bool enabled{ false };
		jsonifier::string name{};
	};

	struct discord_guild_data {
		discord_guild_data() {
			log log{
				.loggingChannelName = "",
				.loggingChannelId	= snowflake{ 0 },
				.nameSmall			= "",
				.name				= "",
			};
			log.name	  = "guild_data ban add";
			log.nameSmall = "guildbanadd";
			this->logs.emplace_back(log);
			log.name	  = "guild_data ban remove";
			log.nameSmall = "guildbanremove";
			this->logs.emplace_back(log);
			log.name	  = "guild_data member add";
			log.nameSmall = "guildmemberadd";
			this->logs.emplace_back(log);
			log.name	  = "guild_data member remove";
			log.nameSmall = "guildmemberremove";
			this->logs.emplace_back(log);
			log.name	  = "display name change";
			log.nameSmall = "displaynamechange";
			this->logs.emplace_back(log);
			log.name	  = "nickname change";
			log.nameSmall = "nicknamechange";
			this->logs.emplace_back(log);
			log.name	  = "Role_Data add or remove";
			log.nameSmall = "roleaddorremove";
			this->logs.emplace_back(log);
			log.name	  = "invite create";
			log.nameSmall = "invitecreate";
			this->logs.emplace_back(log);
			log.name	  = "message_data delete";
			log.nameSmall = "messagedelete";
			this->logs.emplace_back(log);
			log.name	  = "message_data delete bulk";
			log.nameSmall = "messagedeletebulk";
			this->logs.emplace_back(log);
			log.name	  = "Role_Data create";
			log.nameSmall = "rolecreate";
			this->logs.emplace_back(log);
			log.name	  = "Role_Data delete";
			log.nameSmall = "roledelete";
			this->logs.emplace_back(log);
			log.name	  = "username change";
			log.nameSmall = "usernamechange";
			this->logs.emplace_back(log);
		}
		jsonifier::vector<deletion_channel_data> deletionChannels{};
		snowflake inviteReportingChannelId{ 0 };
		jsonifier::vector<snowflake> defaultRoleIds{};
		jsonifier::vector<user_ban_info> userBanInfo{};
		jsonifier::vector<snowflake> trackedUsers{};
		jsonifier::string borderColor{ "fefefe" };
		jsonifier::vector<snowflake> ghostedIds{};
		int64_t vanityInviteUses{ 0 };
		uint64_t memberCount{ 0 };
		role_manager roleManager{};
		jsonifier::string guildName{};
		jsonifier::vector<log> logs{};
		snowflake guildId{ 0 };
	};

	/// a permission overwrite, for a given channel_data. \brief a permission overwrite, for a given channel_data.
	struct permission_over_write_data {
		permission_overwrites_type type{};///< Role_Data or user_data type.
		snowflake channelId{ 0 };///< channel_data id for which channel_data this overwrite beint64_ts to.
		permissions allow{};///< collection of permissions to allow.
		permissions deny{};///< collection of permissions to deny.
		snowflake id{ 0 };///< snowflake of the permission overwrite.
	};

	struct discord_guild_member_data {
		jsonifier::vector<permission_over_write_data> previousPermissionOverwrites{};
		jsonifier::vector<snowflake> invitedMemberIds{};
		jsonifier::vector<discord_invite_data> invites{};
		jsonifier::vector<uint64_t> previousRoleIds{};
		jsonifier::string guildMemberMention{};
		snowflake guildMemberId{ 0 };
		uint64_t totalInvites{ 0 };
		jsonifier::string displayName{};
		jsonifier::string globalId{};
		jsonifier::string userName{};
		snowflake guildId{ 0 };
	};

	enum class database_workload_type {
		Discord_User_Write		   = 0,
		Discord_User_Read		   = 1,
		Discord_Guild_Write		   = 2,
		Discord_Guild_Read		   = 3,
		Discord_Guild_Member_Write = 4,
		Discord_Guild_Member_Read  = 5
	};

	struct database_workload {
		discord_guild_member_data guildMemberData{};
		database_workload_type workloadType{};
		discord_guild_data guildData{};
		discord_user_data userData{};
	};

	struct database_return_value {
		discord_guild_member_data discordGuildMember{};
		discord_guild_data discordGuild{};
		discord_user_data discordUser{};
	};

	class database_manager_agent {
	  public:
		inline void initialize(snowflake botUserIdNew) {
			database_manager_agent::botUserId	   = botUserIdNew;
			auto newClient					   = database_manager_agent::getClient();
			mongocxx::database newDataBase	   = (*newClient)[database_manager_agent::botUserId.operator jsonifier::string().data()];
			mongocxx::collection newCollection = newDataBase[database_manager_agent::botUserId.operator jsonifier::string().data()];
		}

		inline mongocxx::pool::entry getClient() {
			return database_manager_agent::thePool.acquire();
		}

		inline database_return_value submitWorkloadAndGetResults(database_workload workload) {
			std::lock_guard<std::mutex> workloadLock{ database_manager_agent::workloadMutex01 };
			while (database_manager_agent::botUserId == 0) {
				std::this_thread::sleep_for(1ms);
			}
			database_return_value newData{};
			mongocxx::pool::entry thePtr = database_manager_agent::getClient();
			try {
				auto newDataBase   = (*thePtr)[database_manager_agent::botUserId.operator jsonifier::string().data()];
				auto newCollection = newDataBase[database_manager_agent::botUserId.operator jsonifier::string().data()];
				switch (workload.workloadType) {
					case (database_workload_type::Discord_User_Write): {
						auto doc = database_manager_agent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNew	 = newCollection.find_one(document.view());
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_User_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view()); 
						if (resultNew) {
							discord_user_data userData = database_manager_agent::parseUserData(*resultNew);
							newData.discordUser		 = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Write): {
						auto doc = database_manager_agent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							discord_guild_data guildData = database_manager_agent::parseGuildData(*resultNew);
							newData.discordGuild	   = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Member_Write): {
						auto doc = database_manager_agent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Member_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							discord_guild_member_data guildMemberData = database_manager_agent::parseGuildMemberData(*resultNew);
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
				std::cout << "database_manager_agent::run()" << error.what() << std::endl;
				return newData;
			}
		}

	  protected:
		mongocxx::instance instance{};
		std::mutex workloadMutex01{};
		mongocxx::pool thePool{};
		snowflake botUserId{};

		template<typename value_type, typename value_type_to_search>
		static void getValueIfNotNull(value_type& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind);

		template<typename value_type_to_search>
		static void getValueIfNotNull(jsonifier::string& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = jsonifier::string{ valueToSearch[valueToFind].get_string().value };
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(snowflake& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<jsonifier::concepts::enum_t enum_type, typename value_type_to_search>
		static void getValueIfNotNull(enum_type& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = static_cast<enum_type>(valueToSearch[valueToFind].get_int64().value);
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(permissions& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(int64_t& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(uint64_t& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(int32_t& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(uint32_t& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(bool& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_bool().value;
			}
		}

		static bsoncxx::builder::basic::document convertUserDataToDBDoc(discord_user_data discordUserData) {
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
				std::cout << "database_manager_agent::convertUserDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		}

		static discord_user_data parseUserData(bsoncxx::document::value docValue) {
			discord_user_data userData{};
			try {
				getValueIfNotNull(userData.userName, docValue.view(), "userName");
				getValueIfNotNull(userData.userId, docValue.view(), "userId");

				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				jsonifier::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.emplace_back(static_cast<uint64_t>(value.get_int64().value));
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (const std::exception& error) {
				std::cout << "database_manager_agent::parseUserData()" << error.what() << std::endl;
				return userData;
			}
		}

		static bsoncxx::builder::basic::document convertGuildDataToDBDoc(discord_guild_data discordGuildData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildName", discordGuildData.guildName));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.memberCount))));
				buildDoc.append(kvp("vanityInviteUses", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.vanityInviteUses))));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor));
				buildDoc.append(kvp("roleManager", [discordGuildData](bsoncxx::builder::basic::sub_document sub_document01) {
					sub_document01.append(kvp("theRoles", [discordGuildData](bsoncxx::builder::basic::sub_array sub_array01) {
						for (auto& value: discordGuildData.roleManager.theRoles) {
							sub_array01.append(bsoncxx::types::b_int64(static_cast<int64_t>(value)));
						}
					}));
					auto theValue = static_cast<snowflake>(discordGuildData.roleManager.channelId);
					sub_document01.append(kvp("channelId", bsoncxx::types::b_int64(static_cast<int64_t>(theValue.operator const uint64_t&()))));
					theValue = static_cast<snowflake>(discordGuildData.roleManager.messageId);
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
								kvp("minutesToWaitUntilDeleted", bsoncxx::types::b_int64(static_cast<int64_t>(value.minutesToWaitUntilDeleted))));
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
				std::cout << "database_manager_agent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		static discord_guild_data parseGuildData(bsoncxx::document::value docValue) {
			discord_guild_data guildData{};
			try {
				getValueIfNotNull(guildData.inviteReportingChannelId, docValue.view(), "inviteReportingChannelId");
				if (docValue.view()["ghostedIds"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: docValue.view()["ghostedIds"].get_array().value) {
						guildData.ghostedIds.emplace_back(snowflake{ static_cast<size_t>(value.get_value().get_int64().value) });
					}
				}

				getValueIfNotNull(guildData.borderColor, docValue.view(), "borderColor");
				getValueIfNotNull(guildData.vanityInviteUses, docValue.view(), "vanityInviteUses");
				getValueIfNotNull(guildData.guildName, docValue.view(), "guildName");
				getValueIfNotNull(guildData.guildId, docValue.view(), "guildId");
				getValueIfNotNull(guildData.memberCount, docValue.view(), "memberCount");

				for (auto& value: docValue.view()["deletionChannels"].get_array().value) {
					deletion_channel_data newData;
					getValueIfNotNull(newData.channelId, value.get_document().view(), "channelId");
					getValueIfNotNull(newData.currentlyBeingDeleted, value.get_document().view(), "currentlyBeingDeleted");
					getValueIfNotNull(newData.deletionMessageId, value.get_document().view(), "deletionMessageId");
					getValueIfNotNull(newData.numberOfMessagesToSave, value.get_document().view(), "numberOfMessagesToSave");

					if (value.get_document().view()["minutesToWaitUntilDeleted"].type() != bsoncxx::v_noabi::type::k_null) {
						newData.minutesToWaitUntilDeleted = value.get_document().view()["minutesToWaitUntilDeleted"].get_int64().value;
					}
					guildData.deletionChannels.emplace_back(newData);
				}
				for (auto& value: docValue.view()["userBanInfo"].get_array().value) {
					user_ban_info newData;
					newData.userId = static_cast<uint64_t>(value["userId"].get_int64().value);
					for (auto& value2: value["userBans"].get_array().value) {
						ban_info_lite newData02;
						getValueIfNotNull(newData02.userId, value2, "userId");
						getValueIfNotNull(newData02.userName, value2, "userName");
						getValueIfNotNull(newData02.avatarUrl, value2, "avatarUrl");
						getValueIfNotNull(newData02.bannedAt, value2, "bannedAt");
						getValueIfNotNull(newData02.reason, value2, "reason");
						newData.userBans.emplace_back(newData02);
					}
					guildData.userBanInfo.emplace_back(newData);
				}
				for (auto& value: docValue.view()["trackedUsers"].get_array().value) {
					guildData.trackedUsers.emplace_back(snowflake{ static_cast<size_t>(value.get_int64().value) });
				}
				auto roleManager = docValue.view()["roleManager"].get_document();
				if (roleManager.view()["theRoles"].type() == bsoncxx::v_noabi::type::k_array) {
					for (auto& value: roleManager.view()["theRoles"].get_array().value) {
						guildData.roleManager.theRoles.emplace_back(static_cast<uint64_t>(value.get_int64().value));
					}
				}

				getValueIfNotNull(guildData.roleManager.channelId, roleManager.view(), "channelId");
				getValueIfNotNull(guildData.roleManager.messageId, roleManager.view(), "messageId");
				getValueIfNotNull(guildData.roleManager.message, roleManager.view(), "message");
				for (auto& value: docValue.view()["defaultRoleIds"].get_array().value) {
					guildData.defaultRoleIds.emplace_back(snowflake{ static_cast<uint64_t>(value.get_int64().value) });
				}
				for (auto& value: docValue.view()["logs"].get_array().value) {
					log newData;
					getValueIfNotNull(newData.enabled, value.get_document().value, "enabled");
					getValueIfNotNull(newData.loggingChannelId, value.get_document().value, "loggingChannelId");
					getValueIfNotNull(newData.loggingChannelName, value.get_document().value, "loggingChannelName");
					getValueIfNotNull(newData.name, value.get_document().value, "name");
					getValueIfNotNull(newData.nameSmall, value.get_document().value, "nameSmall");
					for (uint64_t y = 0; y < guildData.logs.size(); y += 1) {
						if (guildData.logs[y].nameSmall == newData.nameSmall) {
							guildData.logs[y] = newData;
						}
					}
				}
				return guildData;
			} catch (const std::exception& error) {
				std::cout << "database_manager_agent::parseGuildData()" << error.what() << std::endl;
				return guildData;
			}
		};

		static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(discord_guild_member_data discordGuildMemberData) {
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
				buildDoc.append(kvp("totalInvites", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildMemberData.totalInvites))));
				buildDoc.append(kvp("previousPermissionOverwrites", [discordGuildMemberData](bsoncxx::builder::basic::sub_array subArray) {
					for (permission_over_write_data value: discordGuildMemberData.previousPermissionOverwrites) {
						subArray.append([value](bsoncxx::builder::basic::sub_document subDocument) {
							subDocument.append(
								kvp("allow", bsoncxx::types::b_int64(static_cast<int64_t>(jsonifier::strToUint64(permissions{ value.deny }.operator jsonifier::string().data())))),
								kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(value.type))),
								kvp("deny", bsoncxx::types::b_int64(static_cast<int64_t>(jsonifier::strToUint64(permissions{ value.deny }.operator jsonifier::string().data())))),
								kvp("id", jsonifier::toString(static_cast<uint64_t>(value.id))));
						});
					}
				}));
				buildDoc.append(kvp("invitedMemberIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildMemberData.invitedMemberIds) {
						subArray.append(value.operator jsonifier::string());
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
							subDocument.append(kvp("inviteCode", value.inviteCode), kvp("invitesUsed", bsoncxx::types::b_int64(static_cast<int64_t>(value.invitesUsed))),
								kvp("maxInvites", bsoncxx::types::b_int64(static_cast<int64_t>(value.maxInvites))));
						});
					}
				}));
				return buildDoc;
			} catch (const std::exception& error) {
				std::cout << "database_manager_agent::convertGuildMemberDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		static discord_guild_member_data parseGuildMemberData(bsoncxx::document::value docValue) {
			discord_guild_member_data guildMemberData{};
			try {
				getValueIfNotNull(guildMemberData.guildMemberMention, docValue.view(), "guildMemberMention");
				getValueIfNotNull(guildMemberData.guildId, docValue.view(), "guildId");
				getValueIfNotNull(guildMemberData.displayName, docValue.view(), "displayName");
				getValueIfNotNull(guildMemberData.globalId, docValue.view(), "globalId");
				getValueIfNotNull(guildMemberData.guildMemberId, docValue.view(), "guildMemberId");
				getValueIfNotNull(guildMemberData.userName, docValue.view(), "userName");
				getValueIfNotNull(guildMemberData.totalInvites, docValue.view(), "totalInvites");
				for (auto& value: docValue.view()["previousRoleIds"].get_array().value) {
					guildMemberData.previousRoleIds.emplace_back(static_cast<uint64_t>(value.get_int64().value));
				}
				for (auto& value: docValue.view()["invitedMemberIds"].get_array().value) {
					guildMemberData.invitedMemberIds.emplace_back(static_cast<jsonifier::string>(value.get_string().value));
				}
				for (auto& value: docValue.view()["previousPermissionOverwrites"].get_array().value) {
					permission_over_write_data newOverWriteData{};
					getValueIfNotNull(newOverWriteData.allow, value.get_document().view(), "allow");
					getValueIfNotNull(newOverWriteData.deny, value.get_document().view(), "deny");
					getValueIfNotNull(newOverWriteData.id, value.get_document().view(), "id");
					getValueIfNotNull(newOverWriteData.type, value.get_document().view(), "type");
					guildMemberData.previousPermissionOverwrites.emplace_back(newOverWriteData);
				}

				for (auto& value: docValue.view()["invites"].get_array().value) {
					discord_invite_data newData{};
					getValueIfNotNull(newData.inviteCode, value.get_document().view(), "inviteCode");
					getValueIfNotNull(newData.invitesUsed, value.get_document().view(), "invitesUsed");
					getValueIfNotNull(newData.maxInvites, value.get_document().view(), "maxInvites");
					guildMemberData.invites.emplace_back(newData);
				}
				return guildMemberData;
			} catch (const std::exception& error) {
				std::cout << "database_manager_agent::parseGuildMemberData()" << error.what() << std::endl;
				return guildMemberData;
			}
		};
	};

	class discord_user {
	  public:
		static int64_t guildCount;

		discord_user_data data{};

		inline discord_user(database_manager_agent& other, jsonifier::string userNameNew, snowflake userIdNew) {
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB(other);
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
		}

		inline void writeDataToDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_User_Write;
			workload.userData	  = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_User_Read;
			workload.userData	  = this->data;
			auto result			  = other.submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != 0) {
				this->data = result.discordUser;
			}
		}
	};

	class discord_guild {
	  public:
		inline discord_guild() noexcept = default;
		discord_guild_data data{};

		inline discord_guild(database_manager_agent& other, guild_data guildData) {
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB(other);
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
		}

		inline void writeDataToDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_Guild_Write;
			workload.guildData	  = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_Guild_Read;
			workload.guildData	  = this->data;
			auto result			  = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class discord_guild_member {
	  public:
		discord_guild_member_data data{};

		inline discord_guild_member() noexcept = default;

		inline discord_guild_member(database_manager_agent& other, guild_member_data guildMemberData) {
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

		inline void writeDataToDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType	 = database_workload_type::Discord_Guild_Member_Write;
			workload.guildMemberData = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(database_manager_agent& other) {
			database_workload workload{};
			workload.workloadType	 = database_workload_type::Discord_Guild_Member_Read;
			workload.guildMemberData = this->data;
			auto result				 = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != "") {
				this->data = result.discordGuildMember;
			}
		}
	};

	inline static database_manager_agent managerAgent{};
}
