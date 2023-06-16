// DeleteMessages.hpp - Header for the "delete messages" s.
// Aug 6, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class SetDeletionStatus : public BaseFunction {
	  public:
		SetDeletionStatus() {
			this->commandName	  = "setdeletionstatus";
			this->helpDescription = "Sets the auto-delete status for the current channel.";
			EmbedData msgEmbed{};
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

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetDeletionStatus>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				UniquePtr<ChannelData> channel{ makeUnique<ChannelData>(argsNew.getChannelData()) };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, *channel, guildMember, false);

				if (!doWeHaveAdminPermission) {
					return;
				}

				jsonifier::string whatAreWeDoing;
				int32_t howManyBack = 0;
				int64_t numberOfMinutesToWait{ 0 };
				try {
					if (argsNew.getCommandArguments().values.size() > 1) {
						if (std::stoull(argsNew.getCommandArguments().values["minutestosave"].value) <= 0) {
							jsonifier::string msgString = "------\n**Please enter a valid number of minutes to save the messages for! (!setdeletionstatus = "
													"add/remove, AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOWAITUNTILDELETED)**\n------";
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
							RespondToInputEventData dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
							return;
						} else {
							numberOfMinutesToWait = std::stoull(argsNew.getCommandArguments().values["minutestosave"].value);
						}
					}
				} catch (const std::exception&) {
					jsonifier::string msgString = "------\n**Please enter a valid number of minutes to save the messages for! (!setdeletionstatus = add/remove, "
											"AMOUNTOFMESSAGESTOSAVE, NUMBEROFMINUTESTOWAITUNTILDELETED)**\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (argsNew.getCommandArguments().values.size() > 1) {
					whatAreWeDoing = convertToLowerCase(argsNew.getSubCommandName());
					std::cmatch howManyMatch;
					howManyBack = std::stoull(argsNew.getCommandArguments().values["quantity"].value);
				} else if (argsNew.getCommandArguments().values.size() >= 0) {
					whatAreWeDoing = convertToLowerCase(argsNew.getSubCommandName());
					howManyBack	   = 0;
				}

				UniquePtr<DeletionChannelData> currentDeletionChannel{ makeUnique<DeletionChannelData>() };
				currentDeletionChannel->numberOfMessagesToSave = howManyBack;
				currentDeletionChannel->channelId			   = channel->id;
				currentDeletionChannel->currentlyBeingDeleted  = false;
				currentDeletionChannel->deletionMessageId	   = 0;

				if (whatAreWeDoing == "view") {
					jsonifier::string msgString = "\n------\n";
					if (discordGuild.data.deletionChannels.size() > 0) {
						for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
							ChannelData currentChannel = Channels::getCachedChannel({ .channelId = discordGuild.data.deletionChannels[x].channelId });
							if (currentChannel.id == 0) {
								discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + x);
								continue;
							}
							msgString += "__**ChannelData:**__ <#" + discordGuild.data.deletionChannels[x].channelId + ">, __**Messages To Save:**__ " +
								jsonifier::toString(discordGuild.data.deletionChannels[x].numberOfMessagesToSave) + "\n";
						}
					} else {
						msgString = "------\n__There's no channels to display, currently!__\n";
					}
					msgString += "------";

					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Current Deletion Channels:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
					return;
				}
				if (whatAreWeDoing == "add") {
					bool isItFound				 = false;
					int32_t deletionChannelIndex = 0;
					for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
						if (channel->id == discordGuild.data.deletionChannels[x].channelId) {
							*currentDeletionChannel						   = discordGuild.data.deletionChannels[x];
							currentDeletionChannel->currentlyBeingDeleted  = false;
							currentDeletionChannel->numberOfMessagesToSave = howManyBack;
							isItFound									   = true;
							deletionChannelIndex						   = x;
						}
					}
					InputEventData thePtr{ InputEventData{ argsNew.getInputEventData() } };
					if (isItFound == true) {
						jsonifier::string msgString = "------\n**This channel has already been added! I will update your number of saved messages though!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**ChannelData Re-Added:**__");
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Follow_Up_Message);
						dataPackage.addMessageEmbed(*msgEmbed);
						thePtr = InputEvents::respondToInputEventAsync(dataPackage).get();
						InputEvents::deleteInputEventResponseAsync(thePtr, 20000);
						UniquePtr<MessageData> previousMessage{ makeUnique<MessageData>(
							Messages::getMessageAsync({ .channelId = channel->id, .id = currentDeletionChannel->deletionMessageId }).get()) };
						if (previousMessage->id != 0) {
							MessageData dataNew{};
							dataNew.channelId = previousMessage->channelId;
							dataNew.timeStamp = previousMessage->timeStamp;
							dataNew.id		  = previousMessage->id;
							DeleteMessageData dataPackage{ dataNew };
							dataPackage.reason = "Deleting";
							Messages::deleteMessageAsync(dataPackage).get();
						}
					}

					currentDeletionChannel->minutesToWaitUntilDeleted = static_cast<int32_t>(numberOfMinutesToWait);
					jsonifier::string msgString = "------\n__**Messages beyond message number " + jsonifier::toString(currentDeletionChannel->numberOfMessagesToSave) +
						", or messages older than " + jsonifier::toString(currentDeletionChannel->minutesToWaitUntilDeleted) +
						" minutes are being purged, in this channel.** __\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**ChannelData MessageData Purging:**__");
					UniquePtr<MessageData> pinMessage{ makeUnique<MessageData>() };
					RespondToInputEventData dataPackage(thePtr);
					dataPackage.setResponseType(InputEventResponseType::Follow_Up_Message);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					*pinMessage	 = event01.getMessageData();
					Messages::pinMessageAsync({ .channelId = channel->id, .messageId = pinMessage->id }).get();
					currentDeletionChannel->deletionMessageId = pinMessage->id;
					discordGuild.getDataFromDB(managerAgent);
					if (isItFound == true) {
						discordGuild.data.deletionChannels[deletionChannelIndex] = *currentDeletionChannel;
					} else {
						discordGuild.data.deletionChannels.emplace_back(*currentDeletionChannel);
					}
					discordGuild.writeDataToDB(managerAgent);

					return;
				}
				if (whatAreWeDoing == "remove") {
					bool isItFound				 = false;
					int32_t deletionChannelIndex = 0;
					for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
						if (channel->id == discordGuild.data.deletionChannels[x].channelId) {
							isItFound			 = true;
							deletionChannelIndex = x;
						}
					}
					if (isItFound == false) {
						jsonifier::string msgString = "------\n**Sorry, but this channel could not be found in the list of active deletion channels!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**ChannelData Issue:**__");
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
						InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
						return;
					}
					discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + deletionChannelIndex);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString = "\n------\n__**ChannelData Name:**__ <#" + currentDeletionChannel->channelId + "> \n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Disabled ChannelData Purging:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(event01, 20000).get();
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "SetDeletionStatus::execute()" << error.what() << std::endl;
			}
		}
		~SetDeletionStatus(){};
	};

	CoRoutine<void> deleteMessagesToBeWrapped(DiscordGuild discordGuild, int32_t channelIndex) {
		DiscordGuild newDiscordGuild{ discordGuild };
		try {
			const int32_t numberOfMessagesToSave = discordGuild.data.deletionChannels[channelIndex].numberOfMessagesToSave;
			Snowflake channelId					 = newDiscordGuild.data.deletionChannels[channelIndex].channelId;
			if (channelId != 0) {
				UniquePtr<ChannelData> channel{ makeUnique<ChannelData>(Channels::getCachedChannel({ channelId })) };
				if (channel->id == 0) {
					newDiscordGuild.data.deletionChannels.erase(newDiscordGuild.data.deletionChannels.begin() + channelIndex);
					std::cout << DiscordCoreAPI::shiftToBrightBlue() << "Removing an 'unknown channel' from list of deletion channels!" << jsonifier::string{ DiscordCoreAPI::reset() }
							  << std::endl
							  << std::endl;
					newDiscordGuild.writeDataToDB(managerAgent);
					co_return;
				}

				if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == true) {
					std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
					std::cout << DiscordCoreAPI::shiftToBrightGreen() << "Nope! Still being deleted! ChannelData: " + channel->name + " of server " + newDiscordGuild.data.guildName
							  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
							  << std::endl;
					co_return;
				}

				{
					std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
					std::cout << DiscordCoreAPI::shiftToBrightBlue()
							  << "Checking for messages to delete in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName +
							jsonifier::string{ DiscordCoreAPI::reset() }
							  << std::endl
							  << std::endl;
				}

				newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = true;
				newDiscordGuild.writeDataToDB(managerAgent);
				if (numberOfMessagesToSave > 0) {
					UniquePtr<MessageData> startingMessage{ makeUnique<MessageData>() };
					int64_t totalMessagesReturned{ 1 };
					int64_t messagesSaved{ 0 };
					jsonifier_internal::stop_watch stopWatch{ 1500ms };
					stopWatch.reset();
					jsonifier::vector<MessageData> originalMessagesArray{};
					while (totalMessagesReturned > 0 && !stopWatch.hasTimeElapsed()) {
						jsonifier::vector<MessageData> arrayOfMessagesToDelete{};
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
					jsonifier::vector<MessageData> theMessagesFinal{};
					theMessagesFinal.insert(theMessagesFinal.begin(), originalMessagesArray.rbegin(), originalMessagesArray.rend());
					jsonifier::vector<MessageData> messagesToDelete{};
					stopWatch.reset();
					for (uint64_t x = 0; x < originalMessagesArray.size(); x += 1) {
						if (stopWatch.hasTimeElapsed()) {
							break;
						}
						if (!originalMessagesArray[x].pinned &&
							originalMessagesArray[x].timeStamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
							messagesToDelete.emplace_back(originalMessagesArray[x]);
						} else if (!originalMessagesArray[x].timeStamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted) &&
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
					stopWatch.reset();
					for (int32_t x = 0; x < messagesToDelete.size(); x += 1) {
						if (stopWatch.hasTimeElapsed()) {
							break;
						}
						if (messagesToDelete[x].pinned) {
							messagesToDelete.erase(messagesToDelete.begin() + x);
						}
					}

					jsonifier::vector<Snowflake> purgeVector{};
					jsonifier::vector<MessageData> deleteVector{};
					int32_t totalMessageCount{ 0 };
					stopWatch.reset();
					for (int32_t z = 0; z < ( int32_t )messagesToDelete.size(); z += 1) {
						if (stopWatch.hasTimeElapsed()) {
							break;
						}
						if (!messagesToDelete[z].timeStamp.hasTimeElapsed(14, 0, 0) && !messagesToDelete[z].pinned) {
							totalMessageCount += 1;
							purgeVector.emplace_back(messagesToDelete[z].id);
							// deleteVector.emplace_back(messagesToDelete[z]);
						} else if (!messagesToDelete[z].pinned) {
							totalMessageCount += 1;
							deleteVector.emplace_back(messagesToDelete[z]);
						}
					}

					purgeVector.shrinkToFit();
					deleteVector.shrinkToFit();
					{
						std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
						std::cout << DiscordCoreAPI::shiftToBrightGreen()
								  << "Total of " + jsonifier::toString(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
								  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
								  << std::endl;
					}

					if (purgeVector.size() >= 2) {
						jsonifier::vector<jsonifier::vector<Snowflake>> newVector{};
						int32_t secondIndex{ -1 };
						stopWatch.reset();
						for (int32_t w = 0; w < purgeVector.size(); w += 1) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if (w % 100 == 0) {
								newVector.emplace_back(jsonifier::vector<Snowflake>());
								secondIndex += 1;
							}
							newVector[secondIndex].emplace_back(Snowflake{ purgeVector[w] });
						}
						int32_t totalDeletedBefore{ 0 };
						int32_t totalDeletedAfter{ 0 };
						stopWatch.reset();
						for (auto& value: newVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							totalDeletedAfter += ( int32_t )value.size();
							{
								std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
								std::cout << DiscordCoreAPI::shiftToBrightBlue()
										  << "Deleting message numbers " + jsonifier::toString(totalDeletedBefore + 1) + " to " + jsonifier::toString(totalDeletedAfter) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
										  << std::endl;
							}

							Messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "Purging the channel!" }).get();
							totalDeletedBefore += ( int32_t )value.size();
						}
					} else {
						stopWatch.reset();
						for (auto& value: purgeVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							MessageData message = Messages::getMessageAsync({ .channelId = channelId, .id = value }).get();
							deleteVector.emplace_back(message);
						}
					}
					if (deleteVector.size() > 0) {
						int32_t currentValue{ 0 };
						stopWatch.reset();
						for (auto& value: deleteVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
								newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
								newDiscordGuild.writeDataToDB(managerAgent);
								co_return;
							}
							currentValue += 1;
							{
								std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
								std::cout << DiscordCoreAPI::shiftToBrightBlue()
										  << "Deleting message number " + jsonifier::toString(currentValue) + " of " + jsonifier::toString(deleteVector.size()) + ", in channel " +
										channel->name + " of server 0101 " + newDiscordGuild.data.guildName
										  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
										  << std::endl;
							}

							DeleteMessageData deleteData{ value };
							deleteData.channelId = value.channelId;
							deleteData.timeStamp = value.timeStamp;
							deleteData.messageId = value.id;
							deleteData.reason	 = "Purging messages.";
							Messages::deleteMessageAsync(deleteData);
						}
					}

				} else {
					int32_t x = 1;
					int32_t y = 0;
					jsonifier::vector<jsonifier::vector<MessageData>> arrayOfMessageArrays;
					UniquePtr<MessageData> startingMessage{ makeUnique<MessageData>() };
					jsonifier_internal::stop_watch stopWatch{ 1500ms };
					stopWatch.reset();
					while (x != 0 && !stopWatch.hasTimeElapsed()) {
						jsonifier::vector<MessageData> arrayOfMessages{};
						jsonifier::vector<MessageData> arrayOfMessagesToDelete{};
						if (y == 0) {
							arrayOfMessages = Messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
							if (arrayOfMessages.size() > 0) {
								*startingMessage = arrayOfMessages.at(arrayOfMessages.size() - 1);
								stopWatch.reset();
								for (auto& value: arrayOfMessages) {
									if (stopWatch.hasTimeElapsed()) {
										break;
									}
									x = ( int32_t )arrayOfMessages.size();
									if (x > 0 && value.timeStamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
										arrayOfMessagesToDelete.emplace_back(value);
										y += 1;
									}
								}
								arrayOfMessageArrays.emplace_back(arrayOfMessagesToDelete);
							} else {
								x = 0;
							}
							std::this_thread::sleep_for(1ms);
						}

						arrayOfMessages = Messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
						arrayOfMessages.shrinkToFit();
						if (arrayOfMessages.size() > 0) {
							*startingMessage = arrayOfMessages.at(arrayOfMessages.size() - 1);
							x				 = ( int32_t )(arrayOfMessages).size();
							stopWatch.reset();
							for (auto& value: arrayOfMessages) {
								if (stopWatch.hasTimeElapsed()) {
									break;
								}
								x = ( int32_t )arrayOfMessages.size();
								if (x > 0 && value.timeStamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted)) {
									arrayOfMessagesToDelete.emplace_back(value);
									y += 1;
								}
							}
							arrayOfMessageArrays.emplace_back(arrayOfMessagesToDelete);
						} else {
							x = 0;
						}
					}
					if (arrayOfMessageArrays.size() == 0) {
						newDiscordGuild.getDataFromDB(managerAgent);
						newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
						newDiscordGuild.writeDataToDB(managerAgent);
						{
							std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
							std::cout << DiscordCoreAPI::shiftToBrightGreen() << "Total of 0 in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
									  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
									  << std::endl;
						}

						co_return;
					}
					jsonifier::vector<Snowflake> purgeVector{};
					jsonifier::vector<MessageData> deleteVector{};
					int32_t totalMessageCount{ 0 };
					stopWatch.reset();
					for (int32_t w = 0; w < ( int32_t )arrayOfMessageArrays.size(); w += 1) {
						if (stopWatch.hasTimeElapsed()) {
							break;
						}
						stopWatch.reset();
						for (int32_t z = 0; z < ( int32_t )arrayOfMessageArrays[w].size(); z += 1) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if ((!arrayOfMessageArrays[w][z].timeStamp.hasTimeElapsed(14, 0, 0) && !arrayOfMessageArrays[w][z].pinned &&
									arrayOfMessageArrays[w][z].timeStamp.hasTimeElapsed(0, 0, newDiscordGuild.data.deletionChannels[channelIndex].minutesToWaitUntilDeleted))) {
								// deleteVector.emplace_back(arrayOfMessageArrays[w][z]);
								purgeVector.emplace_back(arrayOfMessageArrays[w][z].id);
							} else if (!arrayOfMessageArrays[w][z].pinned) {
								totalMessageCount += 1;
								deleteVector.emplace_back(arrayOfMessageArrays[w][z]);
							}
						}
					}
					purgeVector.shrinkToFit();
					deleteVector.shrinkToFit();
					{
						std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
						std::cout << DiscordCoreAPI::shiftToBrightGreen()
								  << "Total of " + jsonifier::toString(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
								  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
								  << std::endl;
					}

					if (purgeVector.size() >= 2) {
						jsonifier::vector<jsonifier::vector<Snowflake>> newVector{};
						int32_t secondIndex{ -1 };
						stopWatch.reset();
						for (int32_t w = 0; w < purgeVector.size(); w += 1) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if (w % 2 == 0) {
								newVector.emplace_back(jsonifier::vector<Snowflake>());
								secondIndex += 1;
							}
							newVector[secondIndex].emplace_back(Snowflake{ purgeVector[w] });
						}
						int32_t totalDeletedBefore{ 0 };
						int32_t totalDeletedAfter{ 0 };
						stopWatch.reset();
						for (auto& value: newVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							totalDeletedAfter += ( int32_t )value.size();
							{
								std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
								std::cout << DiscordCoreAPI::shiftToBrightBlue()
										  << "Deleting message numbers " + jsonifier::toString(totalDeletedBefore + 1) + " to " + jsonifier::toString(totalDeletedAfter) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
										  << std::endl;
							}

							Messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "Purging the channel!" }).get();
							totalDeletedBefore += ( int32_t )value.size();
						}
					} else {
						stopWatch.reset();
						for (auto& value: purgeVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							UniquePtr<MessageData> message{ makeUnique<MessageData>(Messages::getMessageAsync({ .channelId = channelId, .id = value }).get()) };
							deleteVector.emplace_back(*message);
						}
					}
					if (deleteVector.size() > 0) {
						int32_t currentValue{ 0 };
						stopWatch.reset();
						for (auto& value: deleteVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							newDiscordGuild.getDataFromDB(managerAgent);
							if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == false) {
								newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
								newDiscordGuild.writeDataToDB(managerAgent);
								co_return;
							}
							currentValue += 1;
							{
								std::unique_lock<std::mutex> theLock{ SetDeletionStatus::theMutex };
								std::cout << DiscordCoreAPI::shiftToBrightBlue()
										  << "Deleting message number " + jsonifier::toString(currentValue) + " of " + jsonifier::toString(deleteVector.size()) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ DiscordCoreAPI::reset() } << std::endl
										  << std::endl;
							}

							DeleteMessageData deleteData{ value };
							deleteData.channelId = value.channelId;
							deleteData.timeStamp = value.timeStamp;
							deleteData.messageId = value.id;
							deleteData.reason	 = "Purging messages.";
							Messages::deleteMessageAsync(deleteData);
						}
					}
				}
				newDiscordGuild.getDataFromDB(managerAgent);
				newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
				newDiscordGuild.writeDataToDB(managerAgent);
				co_return;
			} else {
				discordGuild.data.deletionChannels[channelIndex].numberOfMessagesToSave;
				newDiscordGuild.data.deletionChannels.erase(newDiscordGuild.data.deletionChannels.begin() + channelIndex);
				newDiscordGuild.writeDataToDB(managerAgent);
			}
		} catch (const std::exception& error) {
			std::cout << "deleteMessagesToBeWrapped Error: " << error.what() << std::endl;
			newDiscordGuild.getDataFromDB(managerAgent);
			newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			newDiscordGuild.writeDataToDB(managerAgent);
		}
		newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
		newDiscordGuild.writeDataToDB(managerAgent);
		co_return;
	}

	void deleteMessages(DiscordCoreClient*) {
		try {
			jsonifier::vector<GuildData> guildVector = Guilds::getAllGuildsAsync();
			jsonifier::vector<DiscordGuild> discordGuildVector{};
			jsonifier_internal::stop_watch stopWatch{ 25000ms };
			stopWatch.reset();
			for (auto& value: guildVector) {
				if (stopWatch.hasTimeElapsed()) {
					break;
				}
				DiscordGuild discordGuild(managerAgent, value);
				for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
					deleteMessagesToBeWrapped(discordGuild, x);
				}
			}
		} catch (const std::exception& error) {
			std::cout << "deleteMessagesAsync Error: " << error.what() << std::endl;
		}
	}

}// namespace DiscordCoreAPI
