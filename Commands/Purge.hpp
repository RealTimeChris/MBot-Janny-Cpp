// Purge.hpp - Header for the "purge" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Purge : public BaseFunction {
	  public:
		Purge() {
			this->commandName	  = "purge";
			this->helpDescription = "Purges messages from the current channel.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription(
				"------\nEnter /purge #OFMESSAGESTODELETE, @USERMENTION, TRUE/FALSE, where @USERMENTION is optional - select it to only delete messages from that particular user. \
				Also, TRUE / FALSE is used to set whether or not pinned messages are deleted, and it is also therefore optional.\n------");
			msgEmbed.setTitle("__**Purge Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Purge>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember	 = GuildMembers::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, false);

				if (!doWeHaveAdminPermission) {
					return;
				}
				bool deletePinned{ false };
				bool doWeHaveUser{ false };
				jsonifier::string userId{};
				if (argsNew.getCommandArguments().values.contains("user_id")) {
					userId = argsNew.getCommandArguments().values["user_id"].value;
					UserData user{};
					try {
						user = Users::getCachedUser({ .userId = userId });
					} catch (const std::exception&) {
					}

					if (user.userName != jsonifier::string{ "" }) {
						doWeHaveUser = true;
					} else {
						jsonifier::string msgString = "------\n**Please, enter a proper user id for deletion! (17-20 digits) (/purge = #OFMESSAGESTODELETE, @USERMENTION, "
												"USER_ID"
												"TRUE/FALSE or "
												"/purge #OFMESSAGESTODELETE, @USERMENTION, "
												"TRUE/FALSE, where @USERMENTION is optional - select it to only delete messages from that particular user.**\n------";
						EmbedData msgEmbed{};
						msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed.setColor(discordGuild.data.borderColor);
						msgEmbed.setDescription(msgString);
						msgEmbed.setTimeStamp(getTimeAndDate());
						msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
						RespondToInputEventData dataPackage(inputEventData);
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
				}
				if (argsNew.getCommandArguments().values.contains("user")) {
					doWeHaveUser = true;
					userId		 = argsNew.getCommandArguments().values["user"].value;
				}

				if (argsNew.getCommandArguments().values.contains("deletepinned")) {
					jsonifier::string newString = argsNew.getCommandArguments().values["deletepinned"].value;
					if (newString == "true") {
						deletePinned = true;
					} else if (newString == "false") {
						deletePinned = false;
					}
				}
				InputEventData newEvent01{ argsNew.getInputEventData() };

				if (argsNew.getCommandArguments().values.size() > 1) {
					if (argsNew.getCommandArguments().values["user"].value.operator jsonifier::string() != "") {
						userId = argsNew.getCommandArguments().values["user"].value;
					} else {
						userId = argsNew.getCommandArguments().values["user_id"].value;
					}
				}

				uint32_t messageLimit = std::stoull(argsNew.getCommandArguments().values["amount"].value.operator jsonifier::string().data());
				jsonifier::string msgString;
				if (!doWeHaveUser) {
					msgString = "------\n**Deleting " + jsonifier::toString(messageLimit) + " messages.**\n------";
				} else {
					msgString = "------\n**Deleting " + jsonifier::toString(messageLimit) + " messages, from user <@" + userId + ">.**\n------";
				}

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Purging Messages:**__");
				InputEventData argsNewer{};
				RespondToInputEventData dataPackage02(newEvent01);
				dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage02.addMessageEmbed(msgEmbed);
				argsNewer = InputEvents::respondToInputEventAsync(dataPackage02).get();
				jsonifier::string msgString2{};
				Snowflake currentMessageId = argsNewer.getMessageData().id;

				jsonifier::vector<MessageData> messageArray{};
				jsonifier::vector<Snowflake> messageIdsToPurgeFinal{};
				jsonifier::vector<Snowflake> messageIdsToDeleteFinal{};

				int64_t messageCollectedCount{ 0 };
				do {
					try {
						messageArray = Messages::getMessagesAsync({
																	  .beforeThisId = currentMessageId.operator const uint64_t&(),
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
					msgString2 = "------\n**Deleted " + jsonifier::toString(0) + " messages.**\n------";
				}
				while (messageCollectedCount > 0) {
					if (!doWeHaveUser) {
						if (messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size() < 1) {
							msgString2 = "------\n**Deleted " + jsonifier::toString(0) + " messages.**\n------";
						} else {
							msgString2 = "------\n**Deleted " + jsonifier::toString(messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size()) + " messages.**\n------";
						}
					} else {
						if (messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size() < 1) {
							msgString2 = "------\n**Deleted " + jsonifier::toString(0) + " messages, from the user <@" + userId + ">.**\n------";
						} else {
							msgString2 = "------\n**Deleted " + jsonifier::toString(messageIdsToPurgeFinal.size() + messageIdsToDeleteFinal.size()) + " messages, from the user <@" +
								userId + ">.**\n------";
						}
					}
					if (messageIdsToPurgeFinal.size() >= 100) {
						jsonifier::vector<Snowflake> newMessageIdsToPurgeFinal{};
						for (size_t x = 0; x < 99; ++x) {
							newMessageIdsToPurgeFinal.emplace_back(messageIdsToPurgeFinal[x]);
						}
						messageIdsToPurgeFinal.erase(messageIdsToPurgeFinal.begin() + 100);
						messageCollectedCount -= 100;
						Messages::deleteMessagesBulkAsync(
							{ .messageIds = jsonifier::vector<Snowflake>{ newMessageIdsToPurgeFinal }, .channelId = channel.id, .reason = "Deleting" })
							.get();
					} else if (messageIdsToPurgeFinal.size() >= 2) {
						messageCollectedCount -= messageIdsToPurgeFinal.size();
						Messages::deleteMessagesBulkAsync({ .messageIds = messageIdsToPurgeFinal, .channelId = channel.id }).get();
					} else if (messageIdsToDeleteFinal.size() > 0) {
						for (auto& value: messageIdsToDeleteFinal) {
							MessageData dataNew{};
							dataNew.channelId = channel.id;
							dataNew.timeStamp = value.getCreatedAtTimeStamp();
							dataNew.id		  = value;
							DeleteMessageData dataPackage{ dataNew };
							dataPackage.reason = "Deleting";
							Messages::deleteMessageAsync(dataPackage).get();
						}
						messageCollectedCount -= messageIdsToDeleteFinal.size();
					} else {
						break;
					}
				}

				EmbedData msgEmbed2{};

				msgEmbed2.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed2.setColor(discordGuild.data.borderColor);
				msgEmbed2.setDescription(msgString2);
				msgEmbed2.setTimeStamp(getTimeAndDate());
				msgEmbed2.setTitle("__**Purging Messages:**__");
				RespondToInputEventData dataPackage03(argsNewer);
				dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage03.addMessageEmbed(msgEmbed2);
				argsNewer = InputEvents::respondToInputEventAsync(dataPackage03).get();
				InputEvents::deleteInputEventResponseAsync(argsNewer, 20000);
				return;
			} catch (const std::exception& error) {
				std::cout << "Purge::execute()" << error.what() << std::endl;
			}
		}
		~Purge(){};
	};

}// namespace