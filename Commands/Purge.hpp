// Purge.hpp - Header for the "purge" command.
// jul 31, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class purge : public base_function {
	  public:
		purge() {
			this->commandName	  = "purge";
			this->helpDescription = "purges messages from the current channel.";
			embed_data msgEmbed{};
			msgEmbed.setDescription(
				"------\nEnter /purge #ofmessagestodelete, @usermention, true/false, where @usermention is optional - select it to only delete messages from that particular user. \
				also, true / false is used to set whether or not pinned messages are deleted, and it is also therefore optional.\n------");
			msgEmbed.setTitle("__**purge usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<purge>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember	 = guild_members::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });
				auto inputEventData			  = argsNew.getInputEventData();
				bool doWeHaveAdminPermission	 = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, false);

				if (!doWeHaveAdminPermission) {
					return;
				}
				bool deletePinned{ false };
				bool doWeHaveUser{ false };
				uint64_t userId{};
				if (argsNew.getCommandArguments().values.contains("user_id")) {
					userId = argsNew.getCommandArguments().values["user_id"].value.operator size_t();
					user_data user{};
					try {
						user = users::getCachedUser({ .userId = userId });
					} catch (const std::exception&) {
					}

					if (user.userName != jsonifier::string{ "" }) {
						doWeHaveUser = true;
					} else {
						jsonifier::string msgString = "------\n**please, enter a proper user id for deletion! (17-20 digits) (/purge = #ofmessagestodelete, @usermention, "
												"USER_ID"
												"true/false or "
												"/purge #ofmessagestodelete, @usermention, "
												"true/false, where @usermention is optional - select it to only delete messages from that particular user.**\n------";
						embed_data msgEmbed{};
						msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
						msgEmbed.setColor("fefefe");
						msgEmbed.setDescription(msgString);
						msgEmbed.setTimeStamp(getTimeAndDate());
						msgEmbed.setTitle("__**missing or invalid arguments:**__");
						respond_to_input_event_data dataPackage(inputEventData);
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						auto eventNew = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					}
				}
				if (argsNew.getCommandArguments().values.contains("user")) {
					doWeHaveUser = true;
					userId		 = argsNew.getCommandArguments().values["user"].value.operator size_t();
				}

				if (argsNew.getCommandArguments().values.contains("deletepinned")) {
					bool newString = argsNew.getCommandArguments().values["deletepinned"].value.operator bool();
					if (newString ) {
						deletePinned = true;
					} else {
						deletePinned = false;
					}
				}
				input_event_data newEvent01{ argsNew.getInputEventData() };

				if (argsNew.getCommandArguments().values.size() > 1) {
					if (argsNew.getCommandArguments().values["user"].value.operator size_t() != 0) {
						userId = argsNew.getCommandArguments().values["user"].value.operator size_t();
					} else {
						userId = argsNew.getCommandArguments().values["user_id"].value.operator size_t();
					}
				}

				uint32_t messageLimit = argsNew.getCommandArguments().values["amount"].value.operator size_t();
				jsonifier::string msgString;
				if (!doWeHaveUser) {
					msgString = "------\n**deleting " + jsonifier::toString(messageLimit) + " messages.**\n------";
				} else {
					msgString = "------\n**deleting " + jsonifier::toString(messageLimit) + " messages, from user <@" + userId + ">.**\n------";
				}

				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**purging messages:**__");
				input_event_data argsNewer{};
				respond_to_input_event_data dataPackage02(newEvent01);
				dataPackage02.setResponseType(input_event_response_type::Edit_Interaction_Response);
				dataPackage02.addMessageEmbed(msgEmbed);
				argsNewer = input_events::respondToInputEventAsync(dataPackage02).get();
				jsonifier::string msgString2{};
				snowflake currentMessageId = argsNewer.getMessageData().id;

				jsonifier::vector<message_data> messageArray{};
				jsonifier::vector<snowflake> messageIdsToPurgeFinal{};
				jsonifier::vector<snowflake> messageIdsToDeleteFinal{};

				int64_t messageCollectedCount{ 0 };
				do {
					try {
						messageArray = messages::getMessagesAsync({
																	  .beforeThisId = currentMessageId,
																	  .channelId	= channel.id,
																	  .limit		= 100,
																  })
										   .get();
					} catch (const std::exception&) {
					}

					if (!doWeHaveUser) {
						for (size_t x = 0; x < messageArray.size(); ++x) {
							if (messageArray.size() == 0 || messageCollectedCount >= messageLimit) {
								break;
							}
							if (messageArray[x].timeStamp.hasTimeElapsed(14, 0, 0)) {
								if (deletePinned) {
									messageIdsToDeleteFinal.emplace_back(messageArray[x].id);
									++messageCollectedCount;
								} else {
									if (!messageArray[x].pinned) {
										messageIdsToDeleteFinal.emplace_back(messageArray[x].id);
										++messageCollectedCount;
									}
								}
							} else {
								if (deletePinned) {
									messageIdsToPurgeFinal.emplace_back(messageArray[x].id);
									++messageCollectedCount;
								} else {
									if (!messageArray[x].pinned) {
										messageIdsToPurgeFinal.emplace_back(messageArray[x].id);
										++messageCollectedCount;
									}
								}
							}
						}

					} else {
						for (size_t x = 0; x < messageArray.size(); ++x) {
							if (messageArray.size() == 0 || messageCollectedCount >= messageLimit) {
								break;
							}
							if (messageArray[x].timeStamp.hasTimeElapsed(14, 0, 0)) {
								if (userId == messageArray[x].author.id) {
									if (deletePinned) {
										messageIdsToDeleteFinal.emplace_back(messageArray[x].id);
										++messageCollectedCount;
									} else {
										if (!messageArray[x].pinned) {
											messageIdsToDeleteFinal.emplace_back(messageArray[x].id);
											++messageCollectedCount;
										}
									}
								}
							} else {
								if (userId == messageArray[x].author.id) {
									if (deletePinned) {
										messageIdsToPurgeFinal.emplace_back(messageArray[x].id);
										++messageCollectedCount;
									} else {
										if (!messageArray[x].pinned) {
											messageIdsToPurgeFinal.emplace_back(messageArray[x].id);
											++messageCollectedCount;
										}
									}
								}
							}
						}
					}
					if (messageArray.size() > 0) {
						currentMessageId = messageArray.back().id;
					}
				} while (messageCollectedCount < messageLimit && messageArray.size() > 0);
				if (messageCollectedCount == 0) {
					msgString2 = "------\n**deleted " + jsonifier::toString(0) + " messages.**\n------";
				}
				while (messageCollectedCount > 0) {
					if (!doWeHaveUser) {
						if (messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size() < 1) {
							msgString2 = "------\n**deleted " + jsonifier::toString(0) + " messages.**\n------";
						} else {
							msgString2 = "------\n**deleted " + jsonifier::toString(messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size()) + " messages.**\n------";
						}
					} else {
						if (messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size() < 1) {
							msgString2 = "------\n**deleted " + jsonifier::toString(0) + " messages, from the user <@" + userId + ">.**\n------";
						} else {
							msgString2 = "------\n**deleted " + jsonifier::toString(messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size()) + " messages, from the user <@" +
								userId + ">.**\n------";
						}
					}
					if (messageIdsToPurgeFinal.size() >= 100) {
						jsonifier::vector<snowflake> newMessageIdsToPurgeFinal{};
						for (size_t x = 0; x < 99; ++x) {
							newMessageIdsToPurgeFinal.emplace_back(messageIdsToPurgeFinal[x]);
						}
						messageIdsToPurgeFinal.erase(messageIdsToPurgeFinal.begin() + 100);
						messageCollectedCount -= 100;
						messages::deleteMessagesBulkAsync(
							{ .messageIds = jsonifier::vector<snowflake>{ newMessageIdsToPurgeFinal }, .channelId = channel.id, .reason = "deleting" })
							.get();
					} else if (messageIdsToPurgeFinal.size() >= 2) {
						messageCollectedCount -= messageIdsToPurgeFinal.size();
						messages::deleteMessagesBulkAsync({ .messageIds = messageIdsToPurgeFinal, .channelId = channel.id }).get();
					} else if (messageIdsToDeleteFinal.size() > 0) {
						for (auto& value: messageIdsToDeleteFinal) {
							message_data dataNew{};
							dataNew.channelId = channel.id;
							dataNew.timeStamp = value.getCreatedAtTimeStamp();
							dataNew.id		  = value;
							delete_message_data dataPackage{ dataNew };
							dataPackage.reason = "deleting";
							messages::deleteMessageAsync(dataPackage).get();
						}
						messageCollectedCount -= messageIdsToDeleteFinal.size();
					} else {
						break;
					}
				}

				embed_data msgEmbed2{};

				msgEmbed2.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed2.setColor("fefefe");
				msgEmbed2.setDescription(msgString2);
				msgEmbed2.setTimeStamp(getTimeAndDate());
				msgEmbed2.setTitle("__**purging messages:**__");
				respond_to_input_event_data dataPackage03(argsNewer);
				dataPackage03.setResponseType(input_event_response_type::Edit_Interaction_Response);
				dataPackage03.addMessageEmbed(msgEmbed2);
				argsNewer = input_events::respondToInputEventAsync(dataPackage03).get();
				input_events::deleteInputEventResponseAsync(argsNewer, 20000);
				return;
			} catch (const std::exception& error) {
				std::cout << "purge::execute()" << error.what() << std::endl;
			}
		}
		~purge(){};
	};

}// namespace