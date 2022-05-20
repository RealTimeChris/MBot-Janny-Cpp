// ReactionRole.hpp - Allows an individual to select roles from an optinos menu.
// Mar 16, 2022
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	CoRoutine<void> startupTheMessagePerGuild(DiscordCoreAPI::DiscordGuild* discordGuild, BotUser botUser, std::string theMessage, InputEventData& inputData);

	void startupToWrapTwo(DiscordCoreAPI::DiscordCoreClient* theClient);

	CoRoutine<void> theLoop(DiscordGuild* discordGuild, DiscordCoreAPI::DiscordCoreClient* theClient);

	class ReactionRole : public BaseFunction {
	  public:
		ReactionRole() {
			this->commandName = "reactionrole";
			this->helpDescription = "Setup or utilize a select-menu for individuals to select roles from.";
			DiscordCoreAPI::EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /reactionrole add/remove/instantiate OR /reactionrole add OR /reactionrole remove OR /rectionrole instantiate------");
			msgEmbed.setTitle("__**Reaction Role Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<ReactionRole>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				std::unique_ptr<Channel> channel{ std::make_unique<Channel>(Channels::getCachedChannelAsync({ newArgs.eventData.getChannelId() }).get()) };

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				std::unique_ptr<Guild> guild{ std::make_unique<Guild>(Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get()) };
				std::unique_ptr<DiscordGuild> discordGuild(std::make_unique<DiscordGuild>(*guild));

				GuildMember sendingGuildMember =
					GuildMembers::getGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() }).get();
				GuildMember botMember =
					GuildMembers::getGuildMemberAsync({ .guildMemberId = newArgs.discordCoreClient->getBotUser().id, .guildId = newArgs.eventData.getGuildId() }).get();
				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, *discordGuild, *channel, sendingGuildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				std::string whatAreWeDoing{ "" };
				uint64_t roleId{ 0 };
				std::regex roleIdRegex{ "\\d{18}" };
				std::string theMessage{ "" };
				if (newArgs.commandData.subCommandName == "add") {
					whatAreWeDoing = "add";
					std::cmatch roleIdMatch{};
					roleId = stoull(newArgs.commandData.optionsArgs[0]);
				} else if (newArgs.commandData.subCommandName == "remove") {
					whatAreWeDoing = "remove";
					std::cmatch roleIdMatch{};
					roleId = stoull(newArgs.commandData.optionsArgs[0]);
				} else if (newArgs.commandData.subCommandName == "instantiate") {
					whatAreWeDoing = "instantiate";
					theMessage = newArgs.commandData.optionsArgs[0];
				}
				if (whatAreWeDoing == "add") {
					bool isItFound{ false };
					for (auto& value: discordGuild->data.roleManager.theRoles) {
						if (roleId == value) {
							isItFound = true;
							break;
						}
					}
					if (isItFound) {
						std::string msgString = "------\n**Sorry, but that role is already added!**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(botMember.userName, botMember.userAvatar);
						msgEmbed->setColor(discordGuild->data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Adding Role Issue:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
					auto botRoles = Roles::getGuildMemberRolesAsync({ .guildMember = botMember, .guildId = newArgs.eventData.getGuildId() }).get();
					std::vector<Role> theRolesNew{};
					Role highestRole{};
					for (auto& value: botRoles) {
						if (value.position > highestRole.position) {
							highestRole = value;
						}
					}
					Role newRole = Roles::getRoleAsync({ .guildId = newArgs.eventData.getGuildId(), .roleId = roleId }).get();
					if (newRole.position > highestRole.position) {
						std::string msgString = "------\n**Sorry, but that role needs to be lowered beneath mine in the list of roles so that I can apply it!**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(botMember.userName, botMember.userAvatar);
						msgEmbed->setColor(discordGuild->data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Adding Role Issue:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
					std::string msgString = "------\n**Nicely done! You've added the role <@&" + std::to_string(newRole.id) + "> to your list of roles to select from!**\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild->data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**New Role Added:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					discordGuild->data.roleManager.theRoles.push_back(newRole.id);
					discordGuild->writeDataToDB();
				} else if (whatAreWeDoing == "remove") {
					bool isItFound{ false };
					for (auto& value: discordGuild->data.roleManager.theRoles) {
						if (roleId == value) {
							isItFound = true;
							break;
						}
					}
					if (!isItFound) {
						std::string msgString = "------\n**Sorry, but that role is not already added!**\n------";
						std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
						msgEmbed->setAuthor(botMember.userName, botMember.userAvatar);
						msgEmbed->setColor(discordGuild->data.borderColor);
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**Removing Role Issue:**__");
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}
					Role newRole = Roles::getRoleAsync({ .guildId = newArgs.eventData.getGuildId(), .roleId = roleId }).get();
					for (uint32_t x = 0; x < discordGuild->data.roleManager.theRoles.size(); x += 1) {
						if (discordGuild->data.roleManager.theRoles[x] == newRole.id) {
							discordGuild->data.roleManager.theRoles.erase(discordGuild->data.roleManager.theRoles.begin() + x);
						}
					}
					if (discordGuild->data.roleManager.theRoles.size() == 0) {
						std::unique_ptr<Message> messageNew{ std::make_unique<Message>(
							Messages::getMessageAsync({ .channelId = discordGuild->data.roleManager.channelId, .id = discordGuild->data.roleManager.messageId }).get()) };
						Messages::deleteMessageAsync({ .timeStamp = messageNew->timestamp, .channelId = messageNew->channelId, .messageId = messageNew->id, .reason = "Deleting!" })
							.get();
						discordGuild->data.roleManager.channelId = 0;
						discordGuild->data.roleManager.messageId = 0;
						discordGuild->data.roleManager.message = "";
					}
					discordGuild->writeDataToDB();
					std::string msgString =
						"------\n**Nicely done! You've removed the role <@&" + std::to_string(newRole.id) + "> from your list of roles to select from!**\n------";
					std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed02{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					msgEmbed02->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed02->setColor(discordGuild->data.borderColor);
					msgEmbed02->setDescription(msgString);
					msgEmbed02->setTimeStamp(getTimeAndDate());
					msgEmbed02->setTitle("__**Role Removed :**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed02);
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;

				} else if (whatAreWeDoing == "instantiate") {
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addContent("TEST");
					auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(eventNew).get();
					if (discordGuild->data.roleManager.theRoles.size() == 0) {
						discordGuild->data.roleManager.channelId = 0;
						discordGuild->data.roleManager.messageId = 0;
						discordGuild->data.roleManager.message = "";
						discordGuild->writeDataToDB();
						return;
					}
					discordGuild->data.roleManager.channelId = newArgs.eventData.getChannelId();
					discordGuild->data.roleManager.message = theMessage;
					discordGuild->writeDataToDB();
					theLoop(discordGuild.get(), newArgs.discordCoreClient);
				}
			} catch (...) {
				reportException("ReactionRole::execute()");
			}
		}
		~ReactionRole(){};
	};

	CoRoutine<void> theLoop(DiscordGuild* discordGuild, DiscordCoreAPI::DiscordCoreClient* theClient) {
		auto botUser = theClient->getBotUser();
		co_await NewThreadAwaitable<void>();
		discordGuild->getDataFromDB();
		std::unique_ptr<Message> message{ std::make_unique<Message>(
			Messages::getMessageAsync({ .channelId = discordGuild->data.roleManager.channelId, .id = discordGuild->data.roleManager.messageId }).get()) };

		if (message->id != 0) {
			Messages::deleteMessageAsync({ .timeStamp = message->timestamp, .channelId = message->channelId, .messageId = message->id, .reason = "Deleting!" }).get();
		}
		std::string messageString = "------\n__**Hello! Press start to begin selecting your roles!**__\n------";
		std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed02{ std::make_unique<DiscordCoreAPI::EmbedData>() };
		msgEmbed02->setAuthor(botUser.userName, botUser.avatar);
		msgEmbed02->setColor(discordGuild->data.borderColor);
		msgEmbed02->setDescription(messageString);
		msgEmbed02->setTimeStamp(getTimeAndDate());
		msgEmbed02->setTitle("__**Add New Roles:**__");
		CreateMessageData dataPackage02{};
		dataPackage02.channelId = discordGuild->data.roleManager.channelId;
		Guild guildNew = Guilds::getGuildAsync({ .guildId = discordGuild->data.guildId }).get();
		dataPackage02.addButton(false, "start", "Start", ButtonStyle::Success, "check", 687509905208508640);
		dataPackage02.addMessageEmbed(*msgEmbed02);
		std::unique_ptr<Message> newMessage{ std::make_unique<Message>(Messages::createMessageAsync(dataPackage02).get()) };
		std::unique_ptr<InteractionData> interaction{ std::make_unique<InteractionData>() };
		InputEventData currentEvent = InputEventData{ *interaction };
		int32_t counter{ 0 };

		while (true) {
			discordGuild->getDataFromDB();
			if (counter > 0) {
				*newMessage = Messages::createMessageAsync(dataPackage02).get();
				discordGuild->data.roleManager.messageId = newMessage->id;
				discordGuild->data.roleManager.channelId = newMessage->channelId;
				discordGuild->writeDataToDB();

				currentEvent = InputEventData{ *interaction };

				std::unique_ptr<ButtonCollector> buttonCollector{ std::make_unique<ButtonCollector>(currentEvent) };
				auto resultValue = buttonCollector->collectButtonData(true, INT32_MAX, 1, 0).get();
				InputEventData inputData = InputEventData{ *resultValue[0].interactionData };
				startupTheMessagePerGuild(discordGuild, botUser, discordGuild->data.roleManager.message, inputData);

				Messages::deleteMessageAsync({ .timeStamp = newMessage->timestamp, .channelId = newMessage->channelId, .messageId = newMessage->id, .reason = "Deleting!" }).get();
			} else {
				discordGuild->data.roleManager.messageId = newMessage->id;
				discordGuild->data.roleManager.channelId = newMessage->channelId;
				discordGuild->writeDataToDB();

				std::unique_ptr<ButtonCollector> buttonCollector{ std::make_unique<ButtonCollector>(currentEvent) };
				auto resultValue = buttonCollector->collectButtonData(true, INT32_MAX, 1, 0).get();
				InputEventData inputData = InputEventData{ *resultValue[0].interactionData };
				startupTheMessagePerGuild(discordGuild, botUser, discordGuild->data.roleManager.message, inputData);

				Messages::deleteMessageAsync({ .timeStamp = newMessage->timestamp, .channelId = newMessage->channelId, .messageId = newMessage->id, .reason = "Deleting!" }).get();
			}


			counter += 1;
		}
		co_return;
	}

	void startupToWrapTwo(DiscordCoreAPI::DiscordCoreClient* theClient) {
		auto guilds = Guilds::getAllGuildsAsync().get();
		for (auto& value: guilds) {
			DiscordCoreAPI::DiscordGuild discordGuild{ value };
			if (discordGuild.data.roleManager.messageId == 0 || discordGuild.data.roleManager.theRoles.size() == 0) {
				continue;
			}
			theLoop(&discordGuild, theClient);
		}
	}

	CoRoutine<void> startupTheMessagePerGuild(DiscordCoreAPI::DiscordGuild* discordGuild, BotUser botUser, std::string theMessage, InputEventData& inputData) {
		co_await NewThreadAwaitable<void>();
		if (discordGuild->data.roleManager.theRoles.size() != 0) {
			std::vector<RespondToInputEventData> dataPackages{};
			std::vector<EmbedData> messageEmbeds{};
			int32_t currentPageIndex{ 0 };
			std::vector<SelectOptionData> theOptions{};
			for (int32_t x = 0; x < discordGuild->data.roleManager.theRoles.size(); x += 1) {
				if (x % 25 == 0) {
					if (x > 0) {
						currentPageIndex += 1;
					}
					messageEmbeds.push_back(EmbedData{});
					dataPackages.push_back(inputData);

					Role roleNew{};
					for (int32_t y = currentPageIndex * 25; y < currentPageIndex * 25 + 25 && y < discordGuild->data.roleManager.theRoles.size(); y += 1) {
						std::unique_ptr<SelectOptionData> newOption{ std::make_unique<SelectOptionData>() };
						roleNew = Roles::getRoleAsync({ .guildId = discordGuild->data.guildId, .roleId = discordGuild->data.roleManager.theRoles[y] }).get();
						newOption->emoji.name = roleNew.unicodeEmoji;
						newOption->description = roleNew.name;
						newOption->label = roleNew.name;
						newOption->value = roleNew.id;
						if (roleNew.name != "") {
							theOptions.push_back(*newOption);
						}
					}

					if (theOptions.size() == 0) {
						std::unique_ptr<SelectOptionData> newOption{ std::make_unique<SelectOptionData>() };
						newOption->emoji.name = "❌";
						newOption->description = "There's no roles to display.";
						newOption->label = "Empty";
						newOption->value = "empty";
						theOptions.push_back(*newOption);
					} else {
						std::unique_ptr<SelectOptionData> newOption{ std::make_unique<SelectOptionData>() };
						newOption->emoji.name = "❌";
						newOption->description = "Exit this menu.";
						newOption->label = "Exit";
						newOption->value = "exit";
						theOptions.push_back(*newOption);
					}
					messageEmbeds[currentPageIndex]
						.setDescription(discordGuild->data.roleManager.message)
						.setAuthor(botUser.userName, botUser.avatar)
						.setColor(discordGuild->data.borderColor)
						.setTitle("__**Get Your Roles (" + std::to_string(currentPageIndex + 1) + " of " + std::to_string(discordGuild->data.roleManager.theRoles.size() / 25 + 1) +
							") :**__")
						.setTimeStamp(getTimeAndDate());
					dataPackages[currentPageIndex].setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackages[currentPageIndex].addMessageEmbed(messageEmbeds[currentPageIndex]);
					dataPackages[currentPageIndex].addSelectMenu(false, "role_selection_" + currentPageIndex, theOptions, "Select Roles", static_cast<int32_t>(theOptions.size()),
						0);
				}
			}
			InputEventData newEvent{ inputData };
			newEvent.responseType = InputEventResponseType::Ephemeral_Interaction_Response;
			bool isItFirst{ true };
			while (true) {
				bool doWeQuit{ false };
				if (isItFirst) {
					RespondToInputEventData dataPackage02(newEvent);
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(messageEmbeds[0]);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();
					isItFirst = false;
				} else {
					dataPackages[0] = newEvent;
					dataPackages[0].setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					dataPackages[0].addMessageEmbed(messageEmbeds[currentPageIndex]);
					dataPackages[0].addSelectMenu(false, "role_selection_" + currentPageIndex, theOptions, "Select Roles", static_cast<int32_t>(theOptions.size()), 0);
				}
				auto newResult = moveThroughMessagePages(std::to_string(inputData.getAuthorId()), InputEventData(newEvent), currentPageIndex, messageEmbeds, false, 120000, true);
				std::vector<Role> theRoles{};
				std::vector<SelectMenuResponseData> returnData{};

				std::unique_ptr<SelectMenuCollector> collector{ std::make_unique<SelectMenuCollector>(newEvent) };
				if (newResult.buttonId == "empty" || newResult.buttonId == "exit") {
					goto exit;
				}

				dataPackages[newResult.currentPageIndex].setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
				newEvent = InputEvents::respondToInputEventAsync(dataPackages[newResult.currentPageIndex]).get();
				collector = std::make_unique<SelectMenuCollector>(newEvent);
				returnData = collector->collectSelectMenuData(true, 120000, 1, 0).get();

				if (returnData.size() > 0 && returnData[0].values[0] != "empty") {
					InteractionData eventNew = *returnData[0].interactionData;
					GuildMember guildMember =
						GuildMembers::getGuildMemberAsync({ .guildMemberId = returnData[0].interactionData->member.id, .guildId = discordGuild->data.guildId }).get();
					theRoles = Roles::getGuildMemberRolesAsync({ .guildMember = guildMember, .guildId = discordGuild->data.guildId }).get();

					for (auto value: returnData[0].values) {
						bool isItFound{ false };
						if (value == "exit") {
							doWeQuit = true;
							RespondToInputEventData dataPackage03(newEvent);
							dataPackage03.setResponseType(InputEventResponseType::Edit_Follow_Up_Message);
							dataPackage03.addMessageEmbed(messageEmbeds[currentPageIndex]);
							InputEvents::respondToInputEventAsync(dataPackage03).get();
							continue;
						}
						for (auto& value02: theRoles) {
							if (value02.id == stoull(value)) {
								isItFound = true;
								break;
							}
						}

						if (isItFound) {
							std::string msgString = "------\n**Sorry, but you already have a role called <@&" + value + ">!**\n------";
							std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
							msgEmbed->setAuthor(inputData.getUserName(), inputData.getAvatarUrl());
							msgEmbed->setColor(discordGuild->data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Role Granting Issue:**__");
							RespondToInputEventData dataPackage02(eventNew);
							dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
							dataPackage02.addMessageEmbed(*msgEmbed);
							eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get().getInteractionData();
						} else {
							Roles::addGuildMemberRoleAsync(
								{ .guildId = inputData.getGuildId(), .userId = guildMember.id, .roleId = stoull(value), .reason = "Role-granting." })
								.get();
							std::string msgString = "------\n**Nicely done! You've added the <@&" + value + "> role to yourself!**\n------";
							std::unique_ptr<DiscordCoreAPI::EmbedData> msgEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
							msgEmbed->setAuthor(inputData.getUserName(), inputData.getAvatarUrl());
							msgEmbed->setColor(discordGuild->data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**New Role Granted:**__");
							RespondToInputEventData dataPackage02(eventNew);
							dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
							dataPackage02.addMessageEmbed(*msgEmbed);
							eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get().getInteractionData();
						}
					}
					if (doWeQuit) {
						co_return;
					} else {
						continue;
					}
				}
				if (returnData[0].values[0] == "empty") {
				exit:
					co_return;
				}
			}


		} else {
			co_return;
		}
	};
}