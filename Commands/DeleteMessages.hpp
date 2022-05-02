// DeleteMessages.hpp - Header for the "delete messages" s.
// Aug 6, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	CoRoutine<void> deleteMessagesToBeWrapped(DiscordGuild discordGuild, int32_t channelIndex) {
		co_await NewThreadAwaitable<void>();
		try {
			const int32_t numberOfMessagesToSave = discordGuild.data.deletionChannels[channelIndex].numberOfMessagesToSave;
			std::string channelId = discordGuild.data.deletionChannels[channelIndex].channelId;
			std::unique_ptr<Channel> channel{ std::make_unique<Channel>(Channels::getCachedChannelAsync({ channelId }).get()) };
			if (channel->id == "") {
				discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + channelIndex);
				std::cout << shiftToBrightBlue() << "Removing an 'unknown channel' from list of deletion channels!" << std::endl << std::endl << reset();
				discordGuild.writeDataToDB();
				co_return;
			}

			if (discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == true) {
				std::cout << shiftToBrightGreen() << "Nope! Still being deleted! Channel: " + channel->name + " of server " + discordGuild.data.guildName + "\n"
						  << reset();
				co_return;
			}

			std::cout << shiftToBrightBlue()
					  << "Checking for messages to delete in channel: " + channel->name + " of server " + discordGuild.data.guildName + "\n"
					  << reset();
			discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = true;
			discordGuild.writeDataToDB();
			if (numberOfMessagesToSave > 0) {
				std::unique_ptr<MessageData> startingMessage{ std::make_unique<MessageData>() };
				int64_t totalMessagesReturned{ 1 };
				int64_t messagesSaved{ 0 };
				std::vector<Message> originalMessagesArray{};
				while (totalMessagesReturned > 0) {
					std::vector<Message> arrayOfMessagesToDelete{};
					if (startingMessage->id == "") {
						arrayOfMessagesToDelete = Messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
					} else {
						arrayOfMessagesToDelete =
							Messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
					}

					originalMessagesArray.insert(originalMessagesArray.end(), arrayOfMessagesToDelete.begin(), arrayOfMessagesToDelete.end());
					totalMessagesReturned = arrayOfMessagesToDelete.size();
					if (arrayOfMessagesToDelete.size() > 0) {
						*startingMessage = arrayOfMessagesToDelete[arrayOfMessagesToDelete.size() - 1];
					}
				}
				std::vector<Message> messagesToDelete{};
				for (uint64_t x = 0; x < originalMessagesArray.size(); x += 1) {
					if (!originalMessagesArray[x].pinned &&
						hasTimeElapsed(originalMessagesArray[x].timestamp.getOriginalTimeStamp(), 0, 0,
							discordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
						messagesToDelete.push_back(originalMessagesArray[x]);
					} else if (!hasTimeElapsed(
								   originalMessagesArray[x].timestamp, 0, 0, discordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted) &&
						!originalMessagesArray[x].pinned) {
						messagesSaved += 1;
					} else if (originalMessagesArray[x].pinned) {
						continue;
					}
					if (messagesSaved >= numberOfMessagesToSave) {
						messagesToDelete.insert(messagesToDelete.end(), originalMessagesArray.begin() + x, originalMessagesArray.end());
						break;
					}
				}

				for (int32_t x = 0; x < messagesToDelete.size(); x += 1) {
					if (messagesToDelete[x].pinned) {
						messagesToDelete.erase(messagesToDelete.begin() + x);
					}
				}

				std::vector<std::string> purgeVector{};
				std::vector<Message> deleteVector{};
				int32_t totalMessageCount{ 0 };
				for (int32_t z = 0; z < ( int32_t )messagesToDelete.size(); z += 1) {
					if (!hasTimeElapsed(messagesToDelete[z].timestamp.getOriginalTimeStamp(), 14, 0, 0) && !messagesToDelete[z].pinned) {
						totalMessageCount += 1;
						purgeVector.push_back(messagesToDelete[z].id);
						//deleteVector.push_back(messagesToDelete[z]);
					} else if (!messagesToDelete[z].pinned) {
						totalMessageCount += 1;
						deleteVector.push_back(messagesToDelete[z]);
					}
				}

				purgeVector.shrink_to_fit();
				deleteVector.shrink_to_fit();
				std::cout << shiftToBrightGreen()
						  << "Total of " + std::to_string(totalMessageCount) + " in channel: " + channel->name + " of server " + discordGuild.data.guildName +
						".\n\n"
						  << reset();
				if (purgeVector.size() >= 2) {
					std::vector<std::vector<std::string>> newVector{};
					int32_t secondIndex{ -1 };
					for (int32_t w = 0; w < purgeVector.size(); w += 1) {
						if (w % 100 == 0) {
							newVector.push_back(std::vector<std::string>());
							secondIndex += 1;
						}
						newVector[secondIndex].push_back(purgeVector[w]);
					}
					int32_t totalDeletedBefore{ 0 };
					int32_t totalDeletedAfter{ 0 };
					for (auto& value: newVector) {
						totalDeletedAfter += ( int32_t )value.size();
						std::cout << shiftToBrightBlue()
								  << "Deleting message numbers " + std::to_string(totalDeletedBefore + 1) + " to " + std::to_string(totalDeletedAfter) +
								", in channel " + channel->name + " of server " + discordGuild.data.guildName + ".\n"
								  << reset();
						Messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "Purging the channel!" }).get();
						totalDeletedBefore += ( int32_t )value.size();
					}
				} else {
					for (auto& value: purgeVector) {
						Message message = Messages::getMessageAsync({ .channelId = channelId, .id = value }).get();
						deleteVector.push_back(message);
					}
				}
				if (deleteVector.size() > 0) {
					int32_t currentValue{ 0 };
					for (auto& value: deleteVector) {
						if (discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
							discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
							discordGuild.writeDataToDB();
							co_return;
						}
						currentValue += 1;
						std::cout << shiftToBrightBlue()
								  << "Deleting message number " + std::to_string(currentValue) + " of " + std::to_string(deleteVector.size()) +
								", in channel " + channel->name + " of server " + discordGuild.data.guildName + ".\n"
								  << reset();
						DeleteMessageData deleteData{};
						deleteData.channelId = value.channelId;
						deleteData.timeStamp = value.timestamp;
						deleteData.messageId = value.id;
						deleteData.reason = "Purging messages.";
						Messages::deleteMessageAsync(deleteData);
					}
				}

			} else {
				int32_t x = 1;
				int32_t y = 0;
				std::vector<std::vector<Message>> arrayOfMessageArrays;
				std::unique_ptr<MessageData> startingMessage{ std::make_unique<MessageData>() };
				while (x != 0) {
					std::vector<Message> arrayOfMessages{};
					std::vector<Message> arrayOfMessagesToDelete{};
					if (y == 0) {
						arrayOfMessages = Messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
						if (arrayOfMessages.size() > 0) {
							*startingMessage = arrayOfMessages.at(arrayOfMessages.size() - 1);
							for (auto& value: arrayOfMessages) {
								x = ( int32_t )arrayOfMessages.size();
								if (x > 0 &&
									hasTimeElapsed(value.timestamp.getOriginalTimeStamp(), 0, 0,
										discordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
									arrayOfMessagesToDelete.push_back(value);
									y += 1;
								}
							}
							arrayOfMessageArrays.push_back(arrayOfMessagesToDelete);
						} else {
							x = 0;
						}
					}

					arrayOfMessages = Messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
					arrayOfMessages.shrink_to_fit();
					if (arrayOfMessages.size() > 0) {
						*startingMessage = arrayOfMessages.at(arrayOfMessages.size() - 1);
						x = (int32_t)(arrayOfMessages).size();
						for (auto& value: arrayOfMessages) {
							x = ( int32_t )arrayOfMessages.size();
							if (x > 0 &&
								hasTimeElapsed(
									value.timestamp.getOriginalTimeStamp(), 0, 0, discordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
								arrayOfMessagesToDelete.push_back(value);
								y += 1;
							}
						}
						arrayOfMessageArrays.push_back(arrayOfMessagesToDelete);
					} else {
						x = 0;
					}
				}
				if (arrayOfMessageArrays.size() == 0) {
					discordGuild.getDataFromDB();
					discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
					discordGuild.writeDataToDB();
					std::cout << shiftToBrightGreen() << "Total of 0 in channel: " + channel->name + " of server " + discordGuild.data.guildName + ".\n\n"
							  << reset();
					co_return;
				}
				std::vector<std::string> purgeVector{};
				std::vector<Message> deleteVector{};
				int32_t totalMessageCount{ 0 };
				for (int32_t w = 0; w < ( int32_t )arrayOfMessageArrays.size(); w += 1) {
					for (int32_t z = 0; z < ( int32_t )arrayOfMessageArrays[w].size(); z += 1) {
						if ((!hasTimeElapsed(arrayOfMessageArrays[w][z].timestamp.getOriginalTimeStamp(), 14, 0, 0) && !arrayOfMessageArrays[w][z].pinned &&
								hasTimeElapsed(arrayOfMessageArrays[w][z].timestamp.getOriginalTimeStamp(), 0, 0,
									discordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted))) {
							//deleteVector.push_back(arrayOfMessageArrays[w][z]);
							purgeVector.push_back(arrayOfMessageArrays[w][z].id);
						} else if (!arrayOfMessageArrays[w][z].pinned) {
							totalMessageCount += 1;
							deleteVector.push_back(arrayOfMessageArrays[w][z]);
						}
					}
				}
				purgeVector.shrink_to_fit();
				deleteVector.shrink_to_fit();
				std::cout << shiftToBrightGreen()
						  << "Total of " + std::to_string(totalMessageCount) + " in channel: " + channel->name + " of server " + discordGuild.data.guildName +
						".\n\n"
						  << reset();
				if (purgeVector.size() >= 2) {
					std::vector<std::vector<std::string>> newVector{};
					int32_t secondIndex{ -1 };
					for (int32_t w = 0; w < purgeVector.size(); w += 1) {
						if (w % 100 == 0) {
							newVector.push_back(std::vector<std::string>());
							secondIndex += 1;
						}
						newVector[secondIndex].push_back(purgeVector[w]);
					}
					int32_t totalDeletedBefore{ 0 };
					int32_t totalDeletedAfter{ 0 };
					for (auto& value: newVector) {
						totalDeletedAfter += ( int32_t )value.size();
						std::cout << shiftToBrightBlue()
								  << "Deleting message numbers " + std::to_string(totalDeletedBefore + 1) + " to " + std::to_string(totalDeletedAfter) +
								", in channel " + channel->name + " of server " + discordGuild.data.guildName + ".\n"
								  << reset();
						Messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "Purging the channel!" }).get();
						totalDeletedBefore += ( int32_t )value.size();
					}
				} else {
					for (auto& value: purgeVector) {
						std::unique_ptr<Message> message{ std::make_unique<Message>(Messages::getMessageAsync({ .channelId = channelId, .id = value }).get()) };
						deleteVector.push_back(*message);
					}
				}
				if (deleteVector.size() > 0) {
					int32_t currentValue{ 0 };
					for (auto& value: deleteVector) {
						discordGuild.getDataFromDB();
						if (discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
							discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
							discordGuild.writeDataToDB();
							co_return;
						}
						currentValue += 1;
						std::cout << shiftToBrightBlue()
								  << "Deleting message number " + std::to_string(currentValue) + " of " + std::to_string(deleteVector.size()) +
								", in channel " + channel->name + " of server " + discordGuild.data.guildName + ".\n"
								  << reset();
						DeleteMessageData deleteData{};
						deleteData.channelId = value.channelId;
						deleteData.timeStamp = value.timestamp;
						deleteData.messageId = value.id;
						deleteData.reason = "Purging messages.";
						Messages::deleteMessageAsync(deleteData);
					}
				}
			}
			discordGuild.getDataFromDB();
			discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			discordGuild.writeDataToDB();
			co_return;
		} catch (...) {
			reportException("deleteMessagesToBeWrapped Error: ");
			discordGuild.getDataFromDB();
			discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			discordGuild.writeDataToDB();
		}
		discordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
		discordGuild.writeDataToDB();
		co_return;
	}

	void deleteMessages(DiscordCoreAPI::DiscordCoreClient*) {
		try {
			std::vector<Guild> guildVector = Guilds::getAllGuildsAsync().get();
			std::vector<DiscordGuild> discordGuildVector;

			for (auto& value: guildVector) {
				DiscordGuild discordGuild(value);
				discordGuildVector.push_back(discordGuild);
				for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
					deleteMessagesToBeWrapped(discordGuild, x);
				}
			}

			return;
		} catch (...) {
			reportException("deleteMessagesAsync Error: ");
		}
	}

	class SetDeletionStatus : public BaseFunction {
	  public:
		SetDeletionStatus() {
			this->commandName = "setdeletionstatus";
			this->helpDescription = "Sets the auto-delete status for the current channel.";
			DiscordCoreAPI::EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter this to add/remove message deletion/pruning in a given channel.\nIn the desired channel, type "
									"/setdeletionstatus add/remove, AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOSAVEMESSAGESFOR"
									" enter nothing for AMOUNTOFMESSAGESTOSAVE to save none!\nAlso simply enter /setdeletionstatus view to view the current "
									"list of channels being purged on the current server!.\n------");
			msgEmbed.setTitle("__**Set Deletion Status Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<SetDeletionStatus>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				std::unique_ptr<Channel> channel{ std::make_unique<Channel>(Channels::getCachedChannelAsync({ newArgs.eventData.getChannelId() }).get()) };

				bool areWeInADm = areWeInADM(newArgs.eventData, *channel);

				if (areWeInADm) {
					return;
				}

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() })
						.get();
				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, *channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				std::string whatAreWeDoing;
				std::regex messageCountRegex("\\d{1,18}");
				int32_t howManyBack = 0;
				if (newArgs.commandData.subCommandName == "add" && newArgs.commandData.optionsArgs.size() < 2) {
					if (!std::regex_search(newArgs.commandData.optionsArgs[1].c_str(), messageCountRegex) ||
						std::stoll(newArgs.commandData.optionsArgs[1]) < 0 || std::stoll(newArgs.commandData.optionsArgs[1]) > 10000) {
						std::string msgString = "------\n**Please enter a valid number of messages back to save! (!setdeletionstatus = add/remove, "
												"AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOWAITUNTILDELETED)**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToEvent(dataPackage);
						return;
					}
				}
				int64_t numberOfMinutesToWait{ 0 };
				try {
					if (newArgs.commandData.optionsArgs.size() > 1) {
						if (stoll(newArgs.commandData.optionsArgs[1]) <= 0) {
							std::string msgString = "------\n**Please enter a valid number of minutes to save the messages for! (!setdeletionstatus = "
													"add/remove, AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOWAITUNTILDELETED)**\n------";
							std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
							msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
							RespondToInputEventData dataPackage(newArgs.eventData);
							dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto event01 = InputEvents::respondToEvent(dataPackage);
							return;
						} else {
							numberOfMinutesToWait = stoll(newArgs.commandData.optionsArgs[1]);
						}
					}
				} catch (...) {
					std::string msgString = "------\n**Please enter a valid number of minutes to save the messages for! (!setdeletionstatus = add/remove, "
											"AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOWAITUNTILDELETED)**\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToEvent(dataPackage);
					return;
				}

				if (newArgs.commandData.optionsArgs.size() > 1) {
					whatAreWeDoing = convertToLowerCase(newArgs.commandData.subCommandName);
					std::cmatch howManyMatch;
					std::regex_search(newArgs.commandData.optionsArgs[0].c_str(), howManyMatch, messageCountRegex);
					howManyBack = ( int32_t )std::stoll(howManyMatch.str());
				} else if (newArgs.commandData.optionsArgs.size() == 0) {
					whatAreWeDoing = convertToLowerCase(newArgs.commandData.subCommandName);
					howManyBack = 0;
				}

				std::unique_ptr<DeletionChannelData> currentDeletionChannel{ std::make_unique<DeletionChannelData>() };
				currentDeletionChannel->numberOfMessagesToSave = howManyBack;
				currentDeletionChannel->channelId = newArgs.eventData.getChannelId();
				currentDeletionChannel->currentlyBeingDeleted = false;
				currentDeletionChannel->deletionMessageId = "";

				if (whatAreWeDoing == "view") {
					std::string msgString = "\n------\n";
					if (discordGuild.data.deletionChannels.size() > 0) {
						for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
							Channel currentChannel = Channels::getCachedChannelAsync({ .channelId = discordGuild.data.deletionChannels[x].channelId }).get();
							if (currentChannel.id == "") {
								discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + x);
								continue;
							}
							msgString += "__**Channel:**__ <#" + discordGuild.data.deletionChannels[x].channelId + ">, __**Messages To Save:**__ " +
								std::to_string(discordGuild.data.deletionChannels[x].numberOfMessagesToSave) + "\n";
						}
					} else {
						msgString = "------\n__There's no channels to display, currently!__\n";
					}
					msgString += "------";

					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Current Deletion Channels:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToEvent(dataPackage);
					InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
					return;
				}
				if (whatAreWeDoing == "add") {
					bool isItFound = false;
					int32_t deletionChannelIndex = 0;
					for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
						if (newArgs.eventData.getChannelId() == discordGuild.data.deletionChannels[x].channelId) {
							*currentDeletionChannel = discordGuild.data.deletionChannels[x];
							currentDeletionChannel->currentlyBeingDeleted = false;
							currentDeletionChannel->numberOfMessagesToSave = howManyBack;
							isItFound = true;
							deletionChannelIndex = x;
						}
					}
					InputEventData thePtr{ InputEventData{ newArgs.eventData } };
					if (isItFound == true) {
						std::string msgString = "------\n**This channel has already been added! I will update your number of saved messages though!**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Channel Re-Added:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						thePtr = InputEvents::respondToEvent(dataPackage);
						InputEvents::deleteInputEventResponseAsync(thePtr, 20000);
						std::unique_ptr<Message> previousMessage{ std::make_unique<Message>(
							Messages::getMessageAsync({ .channelId = newArgs.eventData.getChannelId(), .id = currentDeletionChannel->deletionMessageId })
								.get()) };
						if (previousMessage->id != "") {
							Messages::deleteMessageAsync({ .channelId = previousMessage->channelId,
															 .messageId = previousMessage->id,
															 .timeStamp = previousMessage->timestamp,
															 .reason = "Deleting for the next one!" })
								.get();
						}
					}

					currentDeletionChannel->minutesToWaitUntilDeleted = static_cast<int32_t>(numberOfMinutesToWait);
					std::string msgString = "------\n__**Messages beyond message number " + std::to_string(currentDeletionChannel->numberOfMessagesToSave) +
						", or messages older than " + std::to_string(currentDeletionChannel->minutesToWaitUntilDeleted) +
						" minutes are being purged, in this channel.** __\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Channel Message Purging:**__");
					std::unique_ptr<MessageData> pinMessage{ std::make_unique<MessageData>() };
					if (thePtr.responseType == InputEventResponseType::Interaction_Response) {
						RespondToInputEventData dataPackage(thePtr);
						dataPackage.setResponseType(InputEventResponseType::Follow_Up_Message);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToEvent(dataPackage);
						*pinMessage = event01.getMessageData();
					} else {
						RespondToInputEventData dataPackage(thePtr);
						dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToEvent(dataPackage);
						*pinMessage = event01.getMessageData();
					}
					Messages::pinMessageAsync({ .channelId = newArgs.eventData.getChannelId(), .messageId = pinMessage->id }).get();
					currentDeletionChannel->deletionMessageId = pinMessage->id;
					discordGuild.getDataFromDB();
					if (isItFound == true) {
						discordGuild.data.deletionChannels[deletionChannelIndex] = *currentDeletionChannel;
					} else {
						discordGuild.data.deletionChannels.push_back(*currentDeletionChannel);
					}
					discordGuild.writeDataToDB();

					return;
				}
				if (whatAreWeDoing == "remove") {
					bool isItFound = false;
					int32_t deletionChannelIndex = 0;
					for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
						if (newArgs.eventData.getChannelId() == discordGuild.data.deletionChannels[x].channelId) {
							isItFound = true;
							deletionChannelIndex = x;
						}
					}

					if (isItFound == false) {
						std::string msgString = "------\n**Sorry, but this channel could not be found in the list of active deletion channels!**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Channel Issue:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToEvent(dataPackage);
						return;
					}
					discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + deletionChannelIndex);
					discordGuild.writeDataToDB();

					std::string msgString = "\n------\n__**Channel Name:**__ <#" + currentDeletionChannel->channelId + "> \n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Disabled Channel Purging:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToEvent(dataPackage);
					InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
				}
				return;
			} catch (...) {
				reportException("SetDeletionStatus::execute()");
			}
		}
		virtual ~SetDeletionStatus(){};
	};

}
