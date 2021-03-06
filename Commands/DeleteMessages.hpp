// DeleteMessages.hpp - Header for the "delete messages" s.
// Aug 6, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

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

		inline static std::mutex theMutex{};

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<SetDeletionStatus>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				std::unique_ptr<Channel> channel{ std::make_unique<Channel>(Channels::getCachedChannelAsync({ newArgs.eventData.getChannelId() }).get()) };

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() }).get();
				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, *channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				std::string whatAreWeDoing;
				std::regex messageCountRegex("\\d{1,18}");
				int32_t howManyBack = 0;
				if (newArgs.subCommandName == "add" && newArgs.optionsArgs.size() < 2) {
					if (!std::regex_search(newArgs.optionsArgs[1].c_str(), messageCountRegex) || std::stoll(newArgs.optionsArgs[1]) < 0 ||
						std::stoll(newArgs.optionsArgs[1]) > 10000) {
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
						auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
				}
				int64_t numberOfMinutesToWait{ 0 };
				try {
					if (newArgs.optionsArgs.size() > 1) {
						if (stoll(newArgs.optionsArgs[1]) <= 0) {
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
							auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
							return;
						} else {
							numberOfMinutesToWait = stoll(newArgs.optionsArgs[1]);
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
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (newArgs.optionsArgs.size() > 1) {
					whatAreWeDoing = convertToLowerCase(newArgs.subCommandName);
					std::cmatch howManyMatch;
					std::regex_search(newArgs.optionsArgs[0].c_str(), howManyMatch, messageCountRegex);
					howManyBack = ( int32_t )std::stoll(howManyMatch.str());
				} else if (newArgs.optionsArgs.size() == 0) {
					whatAreWeDoing = convertToLowerCase(newArgs.subCommandName);
					howManyBack = 0;
				}

				std::unique_ptr<DeletionChannelData> currentDeletionChannel{ std::make_unique<DeletionChannelData>() };
				currentDeletionChannel->numberOfMessagesToSave = howManyBack;
				currentDeletionChannel->channelId = newArgs.eventData.getChannelId();
				currentDeletionChannel->currentlyBeingDeleted = false;
				currentDeletionChannel->deletionMessageId = 0;

				if (whatAreWeDoing == "view") {
					std::string msgString = "\n------\n";
					if (discordGuild.data.deletionChannels.size() > 0) {
						for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
							Channel currentChannel = Channels::getCachedChannelAsync({ .channelId = discordGuild.data.deletionChannels[x].channelId }).get();
							if (currentChannel.id == 0) {
								discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + x);
								continue;
							}
							msgString += "__**Channel:**__ <#" + std::to_string(discordGuild.data.deletionChannels[x].channelId) + ">, __**Messages To Save:**__ " +
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
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
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
						thePtr = InputEvents::respondToInputEventAsync(dataPackage).get();
						InputEvents::deleteInputEventResponseAsync(thePtr, 20000);
						std::unique_ptr<Message> previousMessage{ std::make_unique<Message>(
							Messages::getMessageAsync({ .channelId = newArgs.eventData.getChannelId(), .id = currentDeletionChannel->deletionMessageId }).get()) };
						if (previousMessage->id != 0) {
							Messages::deleteMessageAsync({ .timeStamp = previousMessage->timestamp,
															 .channelId = previousMessage->channelId,
															 .messageId = previousMessage->id,
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
						auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
						*pinMessage = event01.getMessageData();
					} else {
						RespondToInputEventData dataPackage(thePtr);
						dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
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
						auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
					discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + deletionChannelIndex);
					discordGuild.writeDataToDB();

					std::string msgString = "\n------\n__**Channel Name:**__ <#" + std::to_string(currentDeletionChannel->channelId) + "> \n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Disabled Channel Purging:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
				}
				return;
			} catch (...) {
				reportException("SetDeletionStatus::execute()");
			}
		}
		~SetDeletionStatus(){};
	};

	CoRoutine<void> deleteMessagesToBeWrapped(DiscordGuild discordGuild, int32_t channelIndex) {
		co_await NewThreadAwaitable<void>();
		DiscordGuild newDiscordGuild{ discordGuild };
		try {
			const int32_t numberOfMessagesToSave = discordGuild.data.deletionChannels[channelIndex].numberOfMessagesToSave;
			uint64_t channelId = newDiscordGuild.data.deletionChannels[channelIndex].channelId;
			std::unique_ptr<Channel> channel{ std::make_unique<Channel>(Channels::getCachedChannelAsync({ channelId }).get()) };
			if (channel->id == 0) {
				newDiscordGuild.data.deletionChannels.erase(newDiscordGuild.data.deletionChannels.begin() + channelIndex);
				std::cout << shiftToBrightBlue() << "Removing an 'unknown channel' from list of deletion channels!" << reset() << std::endl << std::endl;
				newDiscordGuild.writeDataToDB();
				co_return;
			}

			if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == true) {
				std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
				std::cout << shiftToBrightGreen() << "Nope! Still being deleted! Channel: " + channel->name + " of server " + newDiscordGuild.data.guildName << reset() << std::endl
						  << std::endl;
				co_return;
			}

			{
				std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
				std::cout << shiftToBrightBlue() << "Checking for messages to delete in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName + reset()
						  << std::endl
						  << std::endl;
			}

			newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = true;
			newDiscordGuild.writeDataToDB();
			if (numberOfMessagesToSave > 0) {
				std::unique_ptr<MessageData> startingMessage{ std::make_unique<MessageData>() };
				int64_t totalMessagesReturned{ 1 };
				int64_t messagesSaved{ 0 };
				std::vector<Message> originalMessagesArray{};
				while (totalMessagesReturned > 0) {
					std::vector<Message> arrayOfMessagesToDelete{};
					if (startingMessage->id == 0) {
						arrayOfMessagesToDelete = Messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
					} else {
						arrayOfMessagesToDelete = Messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
					}

					originalMessagesArray.insert(originalMessagesArray.end(), arrayOfMessagesToDelete.begin(), arrayOfMessagesToDelete.end());
					totalMessagesReturned = arrayOfMessagesToDelete.size();
					if (arrayOfMessagesToDelete.size() > 0) {
						*startingMessage = arrayOfMessagesToDelete[arrayOfMessagesToDelete.size() - 1];
					}
				}
				std::vector<Message> theMessagesFinal{};
				theMessagesFinal.insert(theMessagesFinal.begin(), originalMessagesArray.rbegin(), originalMessagesArray.rend());
				std::vector<Message> messagesToDelete{};
				for (uint64_t x = 0; x < originalMessagesArray.size(); x += 1) {
					if (!originalMessagesArray[x].pinned &&
						originalMessagesArray[x].timestamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
						messagesToDelete.push_back(originalMessagesArray[x]);
					} else if (!originalMessagesArray[x].timestamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted) &&
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

				if (messagesToDelete.size() > 1) {
					messagesToDelete.erase(messagesToDelete.begin());
				}
				for (int32_t x = 0; x < messagesToDelete.size(); x += 1) {
					if (messagesToDelete[x].pinned) {
						messagesToDelete.erase(messagesToDelete.begin() + x);
					}
				}

				std::vector<uint64_t> purgeVector{};
				std::vector<Message> deleteVector{};
				int32_t totalMessageCount{ 0 };
				for (int32_t z = 0; z < ( int32_t )messagesToDelete.size(); z += 1) {
					if (!messagesToDelete[z].timestamp.hasTimeElapsed(14, 0, 0) && !messagesToDelete[z].pinned) {
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
				{
					std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
					std::cout << shiftToBrightGreen()
							  << "Total of " + std::to_string(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName << reset()
							  << std::endl
							  << std::endl;
				}

				if (purgeVector.size() >= 2) {
					std::vector<std::vector<uint64_t>> newVector{};
					int32_t secondIndex{ -1 };
					for (int32_t w = 0; w < purgeVector.size(); w += 1) {
						if (w % 100 == 0) {
							newVector.push_back(std::vector<uint64_t>());
							secondIndex += 1;
						}
						newVector[secondIndex].push_back(purgeVector[w]);
					}
					int32_t totalDeletedBefore{ 0 };
					int32_t totalDeletedAfter{ 0 };
					for (auto& value: newVector) {
						totalDeletedAfter += ( int32_t )value.size();
						{
							std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
							std::cout << shiftToBrightBlue()
									  << "Deleting message numbers " + std::to_string(totalDeletedBefore + 1) + " to " + std::to_string(totalDeletedAfter) + ", in channel " +
									channel->name + " of server " + newDiscordGuild.data.guildName
									  << reset() << std::endl
									  << std::endl;
						}

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
						if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
							newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
							newDiscordGuild.writeDataToDB();
							co_return;
						}
						currentValue += 1;
						{
							std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
							std::cout << shiftToBrightBlue()
									  << "Deleting message number " + std::to_string(currentValue) + " of " + std::to_string(deleteVector.size()) + ", in channel " +
									channel->name + " of server " + newDiscordGuild.data.guildName
									  << reset() << std::endl
									  << std::endl;
						}

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
								if (x > 0 && value.timestamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
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
						x = ( int32_t )(arrayOfMessages).size();
						for (auto& value: arrayOfMessages) {
							x = ( int32_t )arrayOfMessages.size();
							if (x > 0 && value.timestamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
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
					newDiscordGuild.getDataFromDB();
					newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
					newDiscordGuild.writeDataToDB();
					{
						std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
						std::cout << shiftToBrightGreen() << "Total of 0 in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName << reset() << std::endl
								  << std::endl;
					}

					co_return;
				}
				std::vector<uint64_t> purgeVector{};
				std::vector<Message> deleteVector{};
				int32_t totalMessageCount{ 0 };
				for (int32_t w = 0; w < ( int32_t )arrayOfMessageArrays.size(); w += 1) {
					for (int32_t z = 0; z < ( int32_t )arrayOfMessageArrays[w].size(); z += 1) {
						if ((!arrayOfMessageArrays[w][z].timestamp.hasTimeElapsed(14, 0, 0) && !arrayOfMessageArrays[w][z].pinned &&
								arrayOfMessageArrays[w][z].timestamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted))) {
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
				{
					std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
					std::cout << shiftToBrightGreen()
							  << "Total of " + std::to_string(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName << reset()
							  << std::endl
							  << std::endl;
				}

				if (purgeVector.size() >= 2) {
					std::vector<std::vector<uint64_t>> newVector{};
					int32_t secondIndex{ -1 };
					for (int32_t w = 0; w < purgeVector.size(); w += 1) {
						if (w % 100 == 0) {
							newVector.push_back(std::vector<uint64_t>());
							secondIndex += 1;
						}
						newVector[secondIndex].push_back(purgeVector[w]);
					}
					int32_t totalDeletedBefore{ 0 };
					int32_t totalDeletedAfter{ 0 };
					for (auto& value: newVector) {
						totalDeletedAfter += ( int32_t )value.size();
						{
							std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
							std::cout << shiftToBrightBlue()
									  << "Deleting message numbers " + std::to_string(totalDeletedBefore + 1) + " to " + std::to_string(totalDeletedAfter) + ", in channel " +
									channel->name + " of server " + newDiscordGuild.data.guildName
									  << reset() << std::endl
									  << std::endl;
						}

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
						newDiscordGuild.getDataFromDB();
						if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
							newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
							newDiscordGuild.writeDataToDB();
							co_return;
						}
						currentValue += 1;
						{
							std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
							std::cout << shiftToBrightBlue()
									  << "Deleting message number " + std::to_string(currentValue) + " of " + std::to_string(deleteVector.size()) + ", in channel " +
									channel->name + " of server " + newDiscordGuild.data.guildName
									  << reset() << std::endl
									  << std::endl;
						}

						DeleteMessageData deleteData{};
						deleteData.channelId = value.channelId;
						deleteData.timeStamp = value.timestamp;
						deleteData.messageId = value.id;
						deleteData.reason = "Purging messages.";
						Messages::deleteMessageAsync(deleteData);
					}
				}
			}
			newDiscordGuild.getDataFromDB();
			newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			newDiscordGuild.writeDataToDB();
			co_return;
		} catch (...) {
			reportException("deleteMessagesToBeWrapped Error: ");
			newDiscordGuild.getDataFromDB();
			newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			newDiscordGuild.writeDataToDB();
		}
		newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
		newDiscordGuild.writeDataToDB();
		co_return;
	}

	void deleteMessages(DiscordCoreAPI::DiscordCoreClient*) {
		try {
			std::vector<GuildData> guildVector = Guilds::getAllGuildsAsync().get();
			std::vector<DiscordGuild> discordGuildVector;

			for (auto& value: guildVector) {
				DiscordGuild discordGuild(value);
				discordGuildVector.push_back(discordGuild);
				for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
					deleteMessagesToBeWrapped(discordGuild, x);
				}
			}
		} catch (...) {
			reportException("deleteMessagesAsync Error: ");
		}
	}

}
