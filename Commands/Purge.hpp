// Purge.hpp - Header for the "purge" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Purge : public BaseFunction {
	  public:
		Purge() {
			this->commandName = "purge";
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

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Purge>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() }).get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}
				bool deletePinned = false;
				std::string userId;
				std::regex digitRegex("\\d{1,3}");
				std::regex userIdRegex("\\d{18}");
				std::cmatch digitMatch;
				std::regex_search(newArgs.commandData.optionsArgs[0].c_str(), digitMatch, digitRegex);
				std::string digitString = digitMatch.str();
				if (stol(digitString) < 2 || stol(digitString) > 100) {
					std::string msgString = "------\n**Please, enter a proper amount of messages to delete! (2-100) (/purge = #OFMESSAGESTODELETE, @USERMENTION, TRUE/FALSE or "
											"/purge #OFMESSAGESTODELETE, @USERMENTION, "
											"TRUE/FALSE, where @USERMENTION is optional - select it to only delete messages from that particular user.**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (newArgs.commandData.optionsArgs.size() > 1 && !std::regex_search(newArgs.commandData.optionsArgs[1], userIdRegex)) {
					std::string msgString = "------\n**Please, enter a proper user mention! (/purge = #OFMESSAGESTODELETE, @USERMENTION, TRUE/FALSE or /purge "
											"#OFMESSAGESTODELETE, @USERMENTION, TRUE/FALSE, where "
											"@USERMENTION is optional - select it to only delete messages from that particular user.**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (newArgs.commandData.optionsArgs.size() > 2) {
					std::string newString;
					nlohmann::json newJson = nlohmann::json::parse(newArgs.commandData.optionsArgs[2]);
					if (newJson.is_boolean()) {
						bool newBool = newJson.get<bool>();
						if (newBool) {
							newString = "true";
						} else {
							newString = "false";
						}
					} else if (newJson.is_number()) {
						bool newBool = newJson.get<int32_t>();
						if (newBool) {
							newString = "true";
						} else {
							newString = "false";
						}
					} else {
						newString = newJson.get<std::string>();
					}

					if (newString == "true") {
						deletePinned = true;
					} else if (newString == "false") {
						deletePinned = false;
					}
				}
				InputEventData newEvent01{};
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
				newEvent01 = InputEvents::respondToInputEventAsync(dataPackage).get();

				if (newArgs.commandData.optionsArgs.size() > 1) {
					std::cmatch userIdMatch;
					std::regex_search(newArgs.commandData.optionsArgs[1].c_str(), userIdMatch, userIdRegex);
					userId = userIdMatch.str();
				}

				std::string msgString;
				if (newArgs.commandData.optionsArgs.size() < 2) {
					msgString = "------\n**Deleting " + std::to_string(stol(digitString)) + " messages.**\n------";
				} else if (newArgs.commandData.optionsArgs.size() >= 2) {
					msgString = "------\n**Deleting " + std::to_string(stol(digitString)) + " messages, from user <@" + userId + ">.**\n------";
				}

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Purging Messages:**__");
				InputEventData newEvent{};
				RespondToInputEventData dataPackage02(newEvent01);
				dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage02.addMessageEmbed(msgEmbed);
				newEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();
				std::string msgString2;
				uint32_t messageLimit = ( uint32_t )std::stoll(digitString);
				std::vector<uint64_t> messageIdsToDelete;
				uint64_t currentMessageId = newEvent.getMessageId();
				if (deletePinned) {
					if (newArgs.commandData.optionsArgs.size() < 2) {
						while (messageIdsToDelete.size() < messageLimit) {
							std::vector<Message> messageArray = Messages::getMessagesAsync({
																							   .beforeThisId = currentMessageId,
																							   .channelId = newArgs.eventData.getChannelId(),
																							   .limit = 100,
																						   })
																	.get();
					
							if (messageArray.size() == 0) {
								break;
							}
							currentMessageId = messageArray.at(messageArray.size() - 1).id;
							for (auto& value: messageArray) {

								if (!value.timestamp.hasTimeElapsed(14)) {
									messageIdsToDelete.push_back(value.id);
								}
								if (messageIdsToDelete.size() >= messageLimit) {
									break;
								}
							}
						}
					} else if (newArgs.commandData.optionsArgs.size() >= 2) {
						while (messageIdsToDelete.size() < messageLimit) {
							std::vector<Message>messageArray =
								Messages::getMessagesAsync({ .beforeThisId = currentMessageId, .channelId = newArgs.eventData.getChannelId(), .limit = 100 }).get();
							if (messageArray.size() == 0) {
								break;
							}
							currentMessageId = messageArray[messageArray.size() - 1].id;
							for (auto& value: messageArray) {
								if (!value.timestamp.hasTimeElapsed(14) && value.author.id == stoull(userId)) {
									messageIdsToDelete.push_back(value.id);
								}
								if (messageIdsToDelete.size() >= messageLimit) {
									break;
								}
							}
						}
					}
				} else {
					if (newArgs.commandData.optionsArgs.size() < 2) {
						while (messageIdsToDelete.size() < messageLimit) {
							std::vector<Message>messageArray = Messages::getMessagesAsync({
																						.beforeThisId = currentMessageId,
																						.channelId = newArgs.eventData.getChannelId(),
																						.limit = 100,
																					})
															 .get();

							if (messageArray.size() == 0) {
								break;
							}
							currentMessageId = messageArray[messageArray.size() - 1].id;
							for (auto& value: messageArray) {
								if (!value.timestamp.hasTimeElapsed(14) && !value.pinned) {
									messageIdsToDelete.push_back(value.id);
								}
								if (messageIdsToDelete.size() >= messageLimit) {
									break;
								}
							}
						}
					} else if (newArgs.commandData.optionsArgs.size() >= 2) {
						while (messageIdsToDelete.size() < messageLimit) {
							std::vector<Message>messageArray = Messages::getMessagesAsync({
																											  .beforeThisId = currentMessageId,
																											  .channelId = newArgs.eventData.getChannelId(),
																											  .limit = 100,
																										  })
																				   .get();
							if (messageArray.size() == 0) {
								break;
							}
							currentMessageId = messageArray[messageArray.size() - 1].id;
							for (auto& value: messageArray) {
								if (!value.timestamp.hasTimeElapsed(14) && std::to_string(value.author.id) == userId && !value.pinned) {
									messageIdsToDelete.push_back(value.id);
								}
								if (messageIdsToDelete.size() >= messageLimit) {
									break;
								}
							}
						}
					}
				}
				if (newArgs.commandData.optionsArgs.size() < 2) {
					if (messageIdsToDelete.size() < 2) {
						msgString2 = "------\n**Deleted " + std::to_string(0) + " messages.**\n------";
					} else {
						msgString2 = "------\n**Deleted " + std::to_string(messageIdsToDelete.size()) + " messages.**\n------";
					}
				} else if (newArgs.commandData.optionsArgs.size() >= 2) {
					if (messageIdsToDelete.size() < 2) {
						msgString2 = "------\n**Deleted " + std::to_string(0) + " messages, from the user <@" + userId + ">.**\n------";
					} else {
						msgString2 = "------\n**Deleted " + std::to_string(messageIdsToDelete.size()) + " messages, from the user <@" + userId + ">.**\n------";
					}
				}

				Messages::deleteMessagesBulkAsync({ .messageIds = messageIdsToDelete, .channelId = newArgs.eventData.getChannelId() }).get();
				EmbedData msgEmbed2;

				msgEmbed2.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed2.setColor(discordGuild.data.borderColor);
				msgEmbed2.setDescription(msgString2);
				msgEmbed2.setTimeStamp(getTimeAndDate());
				msgEmbed2.setTitle("__**Purging Messages:**__");
				RespondToInputEventData dataPackage03(newEvent);
				dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage03.addMessageEmbed(msgEmbed2);
				newEvent = InputEvents::respondToInputEventAsync(dataPackage03).get();
				InputEvents::deleteInputEventResponseAsync(newEvent, 20000);
				return;
			} catch (...) {
				reportException("Purge::execute()");
			}
		}
		~Purge(){};
	};

}