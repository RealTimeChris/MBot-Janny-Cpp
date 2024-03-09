// DeleteMessages.hpp - Header for the "delete messages" s.
// aug 6, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class set_deletion_status : public base_function {
	  public:
		set_deletion_status() {
			this->commandName	  = "setdeletionstatus";
			this->helpDescription = "sets the auto-delete status for the current channel.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter this to add/remove message deletion/pruning in a given channel.\nIn the desired channel, type "
									"/setdeletionstatus add/remove, amountofmessagestosave, numberofminutestosavemessagesfor"
									" enter nothing for amountofmessagestosave to save none!\nAlso simply enter /setdeletionstatus view to view the current "
									"list of channels being purged on the current server!.\n------");
			msgEmbed.setTitle("__**set deletion status usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		inline static std::mutex theMutex{};

		unique_ptr<base_function> create() {
			return makeUnique<set_deletion_status>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				unique_ptr<channel_data> channel{ makeUnique<channel_data>(argsNew.getChannelData()) };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember{ argsNew.getGuildMemberData() };
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
						if (argsNew.getCommandArguments().values["minutestosave"].operator std::streamoff() <= 0) {
							jsonifier::string msgString = "------\n**please enter a valid number of minutes to save the messages for! (!setdeletionstatus = "
													"add/remove, amountofmessagestosave, numberofminutestowaituntildeleted)**\n------";
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setColor("fefefe");
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**missing or invalid arguments:**__");
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
							return;
						} else {
							numberOfMinutesToWait = argsNew.getCommandArguments().values["minutestosave"].operator size_t();
						}
					}
				} catch (const std::exception&) {
					jsonifier::string msgString = "------\n**please enter a valid number of minutes to save the messages for! (!setdeletionstatus = add/remove, "
											"amountofmessagestosave, numberofminutestowaituntildeleted)**\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**missing or invalid arguments:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (argsNew.getCommandArguments().values.size() > 1) {
					whatAreWeDoing = convertToLowerCase(argsNew.getSubCommandName());
					std::cmatch howManyMatch;
					howManyBack = argsNew.getCommandArguments().values["quantity"].operator size_t();
				} else if (argsNew.getCommandArguments().values.size() >= 0) {
					whatAreWeDoing = convertToLowerCase(argsNew.getSubCommandName());
					howManyBack	   = 0;
				}

				unique_ptr<deletion_channel_data> currentDeletionChannel{ makeUnique<deletion_channel_data>() };
				currentDeletionChannel->numberOfMessagesToSave = howManyBack;
				currentDeletionChannel->channelId			   = channel->id;
				currentDeletionChannel->currentlyBeingDeleted  = false;
				currentDeletionChannel->deletionMessageId	   = 0;

				if (whatAreWeDoing == "view") {
					jsonifier::string msgString = "\n------\n";
					if (discordGuild.data.deletionChannels.size() > 0) {
						for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
							channel_data currentChannel = channels::getCachedChannel({ .channelId = discordGuild.data.deletionChannels[x].channelId });
							if (currentChannel.id == 0) {
								discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + x);
								continue;
							}
							msgString += "__**channel_data:**__ <#" + discordGuild.data.deletionChannels[x].channelId + ">, __**messages to save:**__ " +
								jsonifier::toString(discordGuild.data.deletionChannels[x].numberOfMessagesToSave) + "\n";
						}
					} else {
						msgString = "------\n__There's no channels to display, currently!__\n";
					}
					msgString += "------";

					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**current deletion channels:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
					input_events::deleteInputEventResponseAsync(event01, 20000).get();
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
					input_event_data thePtr{ input_event_data{ argsNew.getInputEventData() } };
					if (isItFound == true) {
						jsonifier::string msgString = "------\n**this channel has already been added! i will update your number of saved messages though!**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setColor("fefefe");
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**channel_data re-added:**__");
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Follow_Up_Message);
						dataPackage.addMessageEmbed(*msgEmbed);
						thePtr = input_events::respondToInputEventAsync(dataPackage).get();
						input_events::deleteInputEventResponseAsync(thePtr, 20000);
						unique_ptr<message_data> previousMessage{ makeUnique<message_data>(
							messages::getMessageAsync({ .channelId = channel->id, .id = currentDeletionChannel->deletionMessageId }).get()) };
						if (previousMessage->id != 0) {
							message_data dataNew{};
							dataNew.channelId = previousMessage->channelId;
							dataNew.timeStamp = previousMessage->timeStamp;
							dataNew.id		  = previousMessage->id;
							delete_message_data dataPackage{ dataNew };
							dataPackage.reason = "deleting";
							messages::deleteMessageAsync(dataPackage).get();
						}
					}

					currentDeletionChannel->minutesToWaitUntilDeleted = static_cast<int32_t>(numberOfMinutesToWait);
					jsonifier::string msgString = "------\n__**messages beyond message number " + jsonifier::toString(currentDeletionChannel->numberOfMessagesToSave) +
						", or messages older than " + jsonifier::toString(currentDeletionChannel->minutesToWaitUntilDeleted) +
						" minutes are being purged, in this channel.** __\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**channel_data message_data purging:**__");
					unique_ptr<message_data> pinMessage{ makeUnique<message_data>() };
					respond_to_input_event_data dataPackage(thePtr);
					dataPackage.setResponseType(input_event_response_type::Follow_Up_Message);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
					*pinMessage	 = event01.getMessageData();
					messages::pinMessageAsync({ .channelId = channel->id, .messageId = pinMessage->id }).get();
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
						jsonifier::string msgString = "------\n**sorry, but this channel could not be found in the list of active deletion channels!**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setColor("fefefe");
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**channel_data issue:**__");
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
						input_events::deleteInputEventResponseAsync(event01, 20000).get();
						return;
					}
					discordGuild.data.deletionChannels.erase(discordGuild.data.deletionChannels.begin() + deletionChannelIndex);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString = "\n------\n__**channel_data name:**__ <#" + currentDeletionChannel->channelId + "> \n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**disabled channel_data purging:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto event01 = input_events::respondToInputEventAsync(dataPackage).get();
					input_events::deleteInputEventResponseAsync(event01, 20000).get();
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "set_deletion_status::execute()" << error.what() << std::endl;
			}
		}
		~set_deletion_status(){};
	};

	co_routine<void> deleteMessagesToBeWrapped(discord_guild discordGuild, int32_t channelIndex) {
		discord_guild newDiscordGuild{ discordGuild };
		try {
			const int32_t numberOfMessagesToSave = discordGuild.data.deletionChannels[channelIndex].numberOfMessagesToSave;
			snowflake channelId					 = newDiscordGuild.data.deletionChannels[channelIndex].channelId;
			if (channelId != 0) {
				unique_ptr<channel_data> channel{ makeUnique<channel_data>(channels::getCachedChannel({ channelId })) };
				if (channel->id == 0) {
					newDiscordGuild.data.deletionChannels.erase(newDiscordGuild.data.deletionChannels.begin() + channelIndex);
					std::cout << discord_core_api::shiftToBrightBlue() << "removing an 'unknown channel' from list of deletion channels!" << jsonifier::string{ discord_core_api::reset() }
							  << std::endl
							  << std::endl;
					newDiscordGuild.writeDataToDB(managerAgent);
					co_return;
				}

				if (newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted == true) {
					std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
					std::cout << discord_core_api::shiftToBrightGreen() << "nope! still being deleted! channel_data: " + channel->name + " of server " + newDiscordGuild.data.guildName
							  << jsonifier::string{ discord_core_api::reset() } << std::endl
							  << std::endl;
					co_return;
				}

				{
					std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
					std::cout << discord_core_api::shiftToBrightBlue()
							  << "checking for messages to delete in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName +
							jsonifier::string{ discord_core_api::reset() }
							  << std::endl
							  << std::endl;
				}

				newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = true;
				newDiscordGuild.writeDataToDB(managerAgent);
				if (numberOfMessagesToSave > 0) {
					unique_ptr<message_data> startingMessage{ makeUnique<message_data>() };
					int64_t totalMessagesReturned{ 1 };
					int64_t messagesSaved{ 0 };
					jsonifier_internal::stop_watch stopWatch{ 1500ms };
					stopWatch.reset();
					jsonifier::vector<message_data> originalMessagesArray{};
					while (totalMessagesReturned > 0 && !stopWatch.hasTimeElapsed()) {
						jsonifier::vector<message_data> arrayOfMessagesToDelete{};
						if (startingMessage->id == 0) {
							arrayOfMessagesToDelete = messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
						} else {
							arrayOfMessagesToDelete = messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
						}

						originalMessagesArray.insert(originalMessagesArray.end(), arrayOfMessagesToDelete.begin(), arrayOfMessagesToDelete.end());
						totalMessagesReturned = arrayOfMessagesToDelete.size();
						if (arrayOfMessagesToDelete.size() > 0) {
							*startingMessage = arrayOfMessagesToDelete[arrayOfMessagesToDelete.size() - 1];
						}
					}
					jsonifier::vector<message_data> theMessagesFinal{};
					theMessagesFinal.insert(theMessagesFinal.begin(), originalMessagesArray.rbegin(), originalMessagesArray.rend());
					jsonifier::vector<message_data> messagesToDelete{};
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

					jsonifier::vector<snowflake> purgeVector{};
					jsonifier::vector<message_data> deleteVector{};
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
						std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
						std::cout << discord_core_api::shiftToBrightGreen()
								  << "total of " + jsonifier::toString(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
								  << jsonifier::string{ discord_core_api::reset() } << std::endl
								  << std::endl;
					}

					if (purgeVector.size() >= 2) {
						jsonifier::vector<jsonifier::vector<snowflake>> newVector{};
						int32_t secondIndex{ -1 };
						stopWatch.reset();
						for (int32_t w = 0; w < purgeVector.size(); w += 1) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if (w % 100 == 0) {
								newVector.emplace_back(jsonifier::vector<snowflake>());
								secondIndex += 1;
							}
							newVector[secondIndex].emplace_back(snowflake{ purgeVector[w] });
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
								std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
								std::cout << discord_core_api::shiftToBrightBlue()
										  << "deleting message numbers " + jsonifier::toString(totalDeletedBefore + 1) + " to " + jsonifier::toString(totalDeletedAfter) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ discord_core_api::reset() } << std::endl
										  << std::endl;
							}

							messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "purging the channel!" }).get();
							totalDeletedBefore += ( int32_t )value.size();
						}
					} else {
						stopWatch.reset();
						for (auto& value: purgeVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							message_data message = messages::getMessageAsync({ .channelId = channelId, .id = value }).get();
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
								std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
								std::cout << discord_core_api::shiftToBrightBlue()
										  << "deleting message number " + jsonifier::toString(currentValue) + " of " + jsonifier::toString(deleteVector.size()) + ", in channel " +
										channel->name + " of server 0101 " + newDiscordGuild.data.guildName
										  << jsonifier::string{ discord_core_api::reset() } << std::endl
										  << std::endl;
							}

							delete_message_data deleteData{ value };
							deleteData.channelId = value.channelId;
							deleteData.timeStamp = value.timeStamp;
							deleteData.messageId = value.id;
							deleteData.reason	 = "purging messages.";
							messages::deleteMessageAsync(deleteData);
						}
					}

				} else {
					int32_t x = 1;
					int32_t y = 0;
					jsonifier::vector<jsonifier::vector<message_data>> arrayOfMessageArrays;
					unique_ptr<message_data> startingMessage{ makeUnique<message_data>() };
					jsonifier_internal::stop_watch stopWatch{ 1500ms };
					stopWatch.reset();
					while (x != 0 && !stopWatch.hasTimeElapsed()) {
						jsonifier::vector<message_data> arrayOfMessages{};
						jsonifier::vector<message_data> arrayOfMessagesToDelete{};
						if (y == 0) {
							arrayOfMessages = messages::getMessagesAsync({ .channelId = channelId, .limit = 100 }).get();
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

						arrayOfMessages = messages::getMessagesAsync({ .beforeThisId = startingMessage->id, .channelId = channelId, .limit = 100 }).get();
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
							std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
							std::cout << discord_core_api::shiftToBrightGreen() << "total of 0 in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
									  << jsonifier::string{ discord_core_api::reset() } << std::endl
									  << std::endl;
						}

						co_return;
					}
					jsonifier::vector<snowflake> purgeVector{};
					jsonifier::vector<message_data> deleteVector{};
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
						std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
						std::cout << discord_core_api::shiftToBrightGreen()
								  << "total of " + jsonifier::toString(totalMessageCount) + " in channel: " + channel->name + " of server " + newDiscordGuild.data.guildName
								  << jsonifier::string{ discord_core_api::reset() } << std::endl
								  << std::endl;
					}

					if (purgeVector.size() >= 2) {
						jsonifier::vector<jsonifier::vector<snowflake>> newVector{};
						int32_t secondIndex{ -1 };
						stopWatch.reset();
						for (int32_t w = 0; w < purgeVector.size(); w += 1) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							if (w % 2 == 0) {
								newVector.emplace_back(jsonifier::vector<snowflake>());
								secondIndex += 1;
							}
							newVector[secondIndex].emplace_back(snowflake{ purgeVector[w] });
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
								std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
								std::cout << discord_core_api::shiftToBrightBlue()
										  << "deleting message numbers " + jsonifier::toString(totalDeletedBefore + 1) + " to " + jsonifier::toString(totalDeletedAfter) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ discord_core_api::reset() } << std::endl
										  << std::endl;
							}

							messages::deleteMessagesBulkAsync({ .messageIds = value, .channelId = channelId, .reason = "purging the channel!" }).get();
							totalDeletedBefore += ( int32_t )value.size();
						}
					} else {
						stopWatch.reset();
						for (auto& value: purgeVector) {
							if (stopWatch.hasTimeElapsed()) {
								break;
							}
							unique_ptr<message_data> message{ makeUnique<message_data>(messages::getMessageAsync({ .channelId = channelId, .id = value }).get()) };
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
								std::unique_lock<std::mutex> theLock{ set_deletion_status::theMutex };
								std::cout << discord_core_api::shiftToBrightBlue()
										  << "deleting message number " + jsonifier::toString(currentValue) + " of " + jsonifier::toString(deleteVector.size()) + ", in channel " +
										channel->name + " of server " + newDiscordGuild.data.guildName
										  << jsonifier::string{ discord_core_api::reset() } << std::endl
										  << std::endl;
							}

							delete_message_data deleteData{ value };
							deleteData.channelId = value.channelId;
							deleteData.timeStamp = value.timeStamp;
							deleteData.messageId = value.id;
							deleteData.reason	 = "purging messages.";
							messages::deleteMessageAsync(deleteData);
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
			std::cout << "deleteMessagesToBeWrapped error: " << error.what() << std::endl;
			newDiscordGuild.getDataFromDB(managerAgent);
			newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
			newDiscordGuild.writeDataToDB(managerAgent);
		}
		newDiscordGuild.data.deletionChannels[channelIndex].currentlyBeingDeleted = false;
		newDiscordGuild.writeDataToDB(managerAgent);
		co_return;
	}

	void deleteMessages(discord_core_client*) {
		try {
			jsonifier::vector<guild_data> guildVector = guilds::getAllGuildsAsync();
			jsonifier::vector<discord_guild> discordGuildVector{};
			jsonifier_internal::stop_watch stopWatch{ 25000ms };
			stopWatch.reset();
			for (auto& value: guildVector) {
				if (stopWatch.hasTimeElapsed()) {
					break;
				}
				discord_guild discordGuild(managerAgent, value);
				for (int32_t x = 0; x < discordGuild.data.deletionChannels.size(); x += 1) {
					deleteMessagesToBeWrapped(discordGuild, x);
				}
			}
		} catch (const std::exception& error) {
			std::cout << "deleteMessagesAsync error: " << error.what() << std::endl;
		}
	}

}// namespace discord_core_api
