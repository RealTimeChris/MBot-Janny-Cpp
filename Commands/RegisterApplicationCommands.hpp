// RegisterApplicationCommands.hpp - Registers the slash commands of this bot.
// May 27, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class RegisterApplicationCommands : public BaseFunction {
	  public:
		RegisterApplicationCommands() {
			this->commandName = "registerapplicationcommands";
			this->helpDescription = "Register some application commands.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /RegisterApplicationCommands\n------");
			msgEmbed.setTitle("__**Register Slash Commands Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<RegisterApplicationCommands>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				InputEvents::deleteInputEventResponseAsync(newArgs.eventData);
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };
				
				CreateGlobalApplicationCommandData reactionRoleData{};
				reactionRoleData.dmPermission = false;
				reactionRoleData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				reactionRoleData.defaultPermission = true;
				reactionRoleData.description = "Setup or instantiate the 'reaction role' menu.";
				reactionRoleData.name = "reactionrole";
				reactionRoleData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData reactionRoleDataOptionOh;
				reactionRoleDataOptionOh.type = ApplicationCommandOptionType::Sub_Command;
				reactionRoleDataOptionOh.name = "add";
				reactionRoleDataOptionOh.description = "Adds a role to the current list of roles.";
				ApplicationCommandOptionData reactionRoleDataOptionOneOne;
				reactionRoleDataOptionOneOne.type = ApplicationCommandOptionType::Role;
				reactionRoleDataOptionOneOne.description = "Which role to add.";
				reactionRoleDataOptionOneOne.name = "role";
				reactionRoleDataOptionOneOne.required = true;
				reactionRoleDataOptionOh.options.push_back(reactionRoleDataOptionOneOne);
				reactionRoleData.options.push_back(reactionRoleDataOptionOh);
				ApplicationCommandOptionData reactionRoleDataOptionTwo;
				reactionRoleDataOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				reactionRoleDataOptionTwo.name = "remove";
				reactionRoleDataOptionTwo.description = "Removes a role from the current list of roles.";
				ApplicationCommandOptionData reactionRoleDataOptionTwoOne;
				reactionRoleDataOptionTwoOne.type = ApplicationCommandOptionType::Role;
				reactionRoleDataOptionTwoOne.description = "Which role to remove.";
				reactionRoleDataOptionTwoOne.name = "role";
				reactionRoleDataOptionTwoOne.required = true;
				reactionRoleDataOptionTwo.options.push_back(reactionRoleDataOptionTwoOne);
				reactionRoleData.options.push_back(reactionRoleDataOptionTwo);
				ApplicationCommandOptionData reactionRoleDataOptionThree;
				reactionRoleDataOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				reactionRoleDataOptionThree.name = "instantiate";
				reactionRoleDataOptionThree.description = "Posts the 'react-to-be-roled' message in the current channel along with a provided message.";
				ApplicationCommandOptionData reactionRoleDataOptionThreeOne;
				reactionRoleDataOptionThreeOne.type = ApplicationCommandOptionType::String;
				reactionRoleDataOptionThreeOne.description = "Which message to provide on the 'React-to-be-roled' menu.";
				reactionRoleDataOptionThreeOne.name = "message";
				reactionRoleDataOptionThreeOne.required = true;
				reactionRoleDataOptionThree.options.push_back(reactionRoleDataOptionThreeOne);
				reactionRoleData.options.push_back(reactionRoleDataOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(reactionRoleData).get();

				CreateGlobalApplicationCommandData registerSetDefaultRoleIdsData{};
				registerSetDefaultRoleIdsData.dmPermission = false;
				registerSetDefaultRoleIdsData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				registerSetDefaultRoleIdsData.defaultPermission = true;
				registerSetDefaultRoleIdsData.description = "Sets a default role to be added to a new user, or removes one.";
				registerSetDefaultRoleIdsData.name = "setdefaultrole";
				registerSetDefaultRoleIdsData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData registerSetDefaultRoleIdsDataOptionOh;
				registerSetDefaultRoleIdsDataOptionOh.type = ApplicationCommandOptionType::Sub_Command;
				registerSetDefaultRoleIdsDataOptionOh.name = "view";
				registerSetDefaultRoleIdsDataOptionOh.description = "View the list of default roles.";
				registerSetDefaultRoleIdsData.options.push_back(registerSetDefaultRoleIdsDataOptionOh);
				ApplicationCommandOptionData registerSetDefaultRoleIdsDataOptionOne;
				registerSetDefaultRoleIdsDataOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				registerSetDefaultRoleIdsDataOptionOne.name = "add";
				registerSetDefaultRoleIdsDataOptionOne.description = "Add a new default role to the list.";
				ApplicationCommandOptionData registerSetDefaultRoleIdsDataOptionOneTwo;
				registerSetDefaultRoleIdsDataOptionOneTwo.type = ApplicationCommandOptionType::Role;
				registerSetDefaultRoleIdsDataOptionOneTwo.description = "Which role to add.";
				registerSetDefaultRoleIdsDataOptionOneTwo.name = "role";
				registerSetDefaultRoleIdsDataOptionOneTwo.required = true;
				registerSetDefaultRoleIdsDataOptionOne.options.push_back(registerSetDefaultRoleIdsDataOptionOneTwo);
				registerSetDefaultRoleIdsData.options.push_back(registerSetDefaultRoleIdsDataOptionOne);
				ApplicationCommandOptionData registerSetDefaultRoleIdsDataOptionTwo;
				registerSetDefaultRoleIdsDataOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				registerSetDefaultRoleIdsDataOptionTwo.name = "remove";
				registerSetDefaultRoleIdsDataOptionTwo.description = "remove a default role from the list.";
				ApplicationCommandOptionData registerSetDefaultRoleIdsDataOptionTwoTwo;
				registerSetDefaultRoleIdsDataOptionTwoTwo.type = ApplicationCommandOptionType::Role;
				registerSetDefaultRoleIdsDataOptionTwoTwo.description = "Which role to remove.";
				registerSetDefaultRoleIdsDataOptionTwoTwo.name = "role";
				registerSetDefaultRoleIdsDataOptionTwoTwo.required = true;
				registerSetDefaultRoleIdsDataOptionTwo.options.push_back(registerSetDefaultRoleIdsDataOptionTwoTwo);
				registerSetDefaultRoleIdsData.options.push_back(registerSetDefaultRoleIdsDataOptionTwo);
				ApplicationCommands::createGlobalApplicationCommandAsync(registerSetDefaultRoleIdsData).get();

				CreateGlobalApplicationCommandData createBotInfoCommandData{};
				createBotInfoCommandData.dmPermission = true;
				createBotInfoCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createBotInfoCommandData.defaultPermission = true;
				createBotInfoCommandData.description = "Displays info about the current bot.";
				createBotInfoCommandData.name = "botinfo";
				createBotInfoCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommands::createGlobalApplicationCommandAsync(createBotInfoCommandData).get();

				CreateGlobalApplicationCommandData createDisplayGuildsDataCommandData{};
				createDisplayGuildsDataCommandData.dmPermission = true;
				createDisplayGuildsDataCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createDisplayGuildsDataCommandData.defaultPermission = true;
				createDisplayGuildsDataCommandData.description = "View the list of servers that this bot is in.";
				createDisplayGuildsDataCommandData.name = "displayguildsdata";
				createDisplayGuildsDataCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommands::createGlobalApplicationCommandAsync(createDisplayGuildsDataCommandData).get();

				CreateGlobalApplicationCommandData registerGhostCommandData{};
				registerGhostCommandData.dmPermission = false;
				registerGhostCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				registerGhostCommandData.defaultPermission = true;
				registerGhostCommandData.description = "Mutes/silences a server-member.";
				registerGhostCommandData.name = "ghost";
				registerGhostCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createGhostOptionOne;
				createGhostOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createGhostOptionOne.name = "view";
				createGhostOptionOne.description = "View the currently ghosted members.";
				registerGhostCommandData.options.push_back(createGhostOptionOne);
				ApplicationCommandOptionData createGhostOptionTwo;
				createGhostOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				createGhostOptionTwo.name = "add";
				createGhostOptionTwo.description = "Ghosts a new server member.";
				ApplicationCommandOptionData createGhostOptionTwoTwo;
				createGhostOptionTwoTwo.name = "user";
				createGhostOptionTwoTwo.description = "Chooses which member to ghost.";
				createGhostOptionTwoTwo.required = true;
				createGhostOptionTwoTwo.type = ApplicationCommandOptionType::User;
				createGhostOptionTwo.options.push_back(createGhostOptionTwoTwo);
				ApplicationCommandOptionData createGhostOptionTwoThree;
				createGhostOptionTwoThree.name = "reason";
				createGhostOptionTwoThree.description = "Specify a reason for the ghosting.";
				createGhostOptionTwoThree.required = true;
				createGhostOptionTwoThree.type = ApplicationCommandOptionType::String;
				createGhostOptionTwo.options.push_back(createGhostOptionTwoThree);
				registerGhostCommandData.options.push_back(createGhostOptionTwo);
				ApplicationCommandOptionData createGhostOptionThree;
				createGhostOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				createGhostOptionThree.name = "remove";
				createGhostOptionThree.description = "Un-Ghosts a server member.";
				ApplicationCommandOptionData createGhostOptionThreeTwo;
				createGhostOptionThreeTwo.name = "user";
				createGhostOptionThreeTwo.description = "Chooses which member to un-ghost.";
				createGhostOptionThreeTwo.required = true;
				createGhostOptionThreeTwo.type = ApplicationCommandOptionType::User;
				createGhostOptionThree.options.push_back(createGhostOptionThreeTwo);
				registerGhostCommandData.options.push_back(createGhostOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(registerGhostCommandData).get();
				
				CreateGlobalApplicationCommandData registerPurgeCommandData{};
				registerPurgeCommandData.dmPermission = false;
				registerPurgeCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				registerPurgeCommandData.defaultPermission = true;
				registerPurgeCommandData.description = "Purges a bulk of messages, possibly from a particular user.";
				registerPurgeCommandData.name = "purge";
				registerPurgeCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createPurgeOptionOne;
				createPurgeOptionOne.type = ApplicationCommandOptionType::Integer;
				createPurgeOptionOne.name = "amount";
				createPurgeOptionOne.minValue = 2;
				createPurgeOptionOne.maxValue = 100;
				createPurgeOptionOne.required = true;
				createPurgeOptionOne.description = "The quantity of messages to delete.";
				registerPurgeCommandData.options.push_back(createPurgeOptionOne);
				ApplicationCommandOptionData createPurgeOptionTwo;
				createPurgeOptionTwo.type = ApplicationCommandOptionType::User;
				createPurgeOptionTwo.name = "user";
				createPurgeOptionTwo.required = false;
				createPurgeOptionTwo.description = "An optional user, who's messages will be deleted.";
				registerPurgeCommandData.options.push_back(createPurgeOptionTwo);
				ApplicationCommandOptionData createPurgeOptionThree;
				createPurgeOptionThree.type = ApplicationCommandOptionType::Boolean;
				createPurgeOptionThree.name = "deletepinned";
				createPurgeOptionThree.required = false;
				createPurgeOptionThree.description = "Do we delete pinned messages?";
				registerPurgeCommandData.options.push_back(createPurgeOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(registerPurgeCommandData).get();
				
				CreateGlobalApplicationCommandData registerServerInfoCommandData{};
				registerServerInfoCommandData.dmPermission = false;
				registerServerInfoCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				registerServerInfoCommandData.defaultPermission = true;
				registerServerInfoCommandData.description = "Displays info about the current server.";
				registerServerInfoCommandData.name = "serverinfo";
				registerServerInfoCommandData.type = ApplicationCommandType::Chat_Input;
				auto theResult = ApplicationCommands::createGlobalApplicationCommandAsync(registerServerInfoCommandData).get();

				CreateGlobalApplicationCommandData createSetBorderColorCommandData{};
				createSetBorderColorCommandData.dmPermission = false;
				createSetBorderColorCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSetBorderColorCommandData.defaultPermission = true;
				createSetBorderColorCommandData.description = "Set the default color of borders.";
				createSetBorderColorCommandData.name = "setbordercolor";
				createSetBorderColorCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createSetBorderColoreOptionOne;
				createSetBorderColoreOptionOne.type = ApplicationCommandOptionType::String;
				createSetBorderColoreOptionOne.name = "botname";
				createSetBorderColoreOptionOne.description = "Which of the bots to change the setting on.";
				createSetBorderColoreOptionOne.required = true;
				ApplicationCommandOptionChoiceData setBorderColorchoiceOne;
				setBorderColorchoiceOne.name = "janny";
				setBorderColorchoiceOne = "janny";
				createSetBorderColoreOptionOne.choices.push_back(setBorderColorchoiceOne);
				createSetBorderColorCommandData.options.push_back(createSetBorderColoreOptionOne);
				ApplicationCommandOptionData createSetBorderColorOptionTwo;
				createSetBorderColorOptionTwo.type = ApplicationCommandOptionType::String;
				createSetBorderColorOptionTwo.name = "hexcolorvalue";
				createSetBorderColorOptionTwo.required = true;
				createSetBorderColorOptionTwo.description = "The hex-color-value to set the borders to.";
				createSetBorderColorCommandData.options.push_back(createSetBorderColorOptionTwo);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSetBorderColorCommandData).get();

				CreateGlobalApplicationCommandData createUserInfoCommandData{};
				createUserInfoCommandData.dmPermission = false;
				createUserInfoCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createUserInfoCommandData.defaultPermission = true;
				createUserInfoCommandData.type = ApplicationCommandType::Chat_Input;
				createUserInfoCommandData.description = "Displays info about a chosen user.";
				createUserInfoCommandData.name = "userinfo";
				ApplicationCommandOptionData createUserInfoOptionOne;
				createUserInfoOptionOne.type = ApplicationCommandOptionType::User;
				createUserInfoOptionOne.name = "user";
				createUserInfoOptionOne.description = "Which of the users to view info about.";
				createUserInfoOptionOne.required = true;
				createUserInfoCommandData.options.push_back(createUserInfoOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(createUserInfoCommandData).get();


				CreateGlobalApplicationCommandData createSetInvitesChannelCommandData{};
				createSetInvitesChannelCommandData.dmPermission = false;
				createSetInvitesChannelCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSetInvitesChannelCommandData.defaultPermission = true;
				createSetInvitesChannelCommandData.description = "Set the invite tracking channel and enables, or disables it.";
				createSetInvitesChannelCommandData.name = "setinviteschannel";
				createSetInvitesChannelCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createSetInvitesChannelOptionOne;
				createSetInvitesChannelOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createSetInvitesChannelOptionOne.name = "add";
				createSetInvitesChannelOptionOne.description = "Add the current channel as the invites-tracking channel, and activate it.";
				ApplicationCommandOptionData createSetInvitesChannelOptionOneOne;
				createSetInvitesChannelOptionOneOne.type = ApplicationCommandOptionType::String;
				createSetInvitesChannelOptionOneOne.name = "add";
				createSetInvitesChannelOptionOneOne.description = "Enable the invite-tracking.";
				createSetInvitesChannelOptionOneOne.required = true;
				ApplicationCommandOptionChoiceData createSetInvitesChoiceOne;
				createSetInvitesChoiceOne.name = "add";
				createSetInvitesChoiceOne = "add";
				createSetInvitesChannelOptionOneOne.choices.push_back(createSetInvitesChoiceOne);
				createSetInvitesChannelOptionOne.options.push_back(createSetInvitesChannelOptionOneOne);
				createSetInvitesChannelCommandData.options.push_back(createSetInvitesChannelOptionOne);
				ApplicationCommandOptionData createSetInvitesChannelOptionTwo;
				createSetInvitesChannelOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				createSetInvitesChannelOptionTwo.name = "remove";
				createSetInvitesChannelOptionTwo.description = "remove the current channel as the invites-tracking channel, and deactivate it.";
				ApplicationCommandOptionData createSetInvitesChannelOptionTwoOne;
				createSetInvitesChannelOptionTwoOne.type = ApplicationCommandOptionType::String;
				createSetInvitesChannelOptionTwoOne.name = "remove";
				createSetInvitesChannelOptionTwoOne.description = "Cancel the invite tracking.";
				createSetInvitesChannelOptionTwoOne.required = true;
				ApplicationCommandOptionChoiceData createSetInvitesChoiceTwo;
				createSetInvitesChoiceTwo.name = "remove";
				createSetInvitesChoiceTwo = "remove";
				createSetInvitesChannelOptionTwoOne.choices.push_back(createSetInvitesChoiceTwo);
				createSetInvitesChannelOptionTwo.options.push_back(createSetInvitesChannelOptionTwoOne);
				createSetInvitesChannelCommandData.options.push_back(createSetInvitesChannelOptionTwo);
				ApplicationCommandOptionData createSetInvitesChannelOptionThree;
				createSetInvitesChannelOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				createSetInvitesChannelOptionThree.name = "view";
				createSetInvitesChannelOptionThree.description = "Displays the current list of members and their respective invite-counts.";
				ApplicationCommandOptionData createSetInvitesChannelOptionThreeOne;
				createSetInvitesChannelOptionThreeOne.type = ApplicationCommandOptionType::String;
				createSetInvitesChannelOptionThreeOne.name = "view";
				createSetInvitesChannelOptionThreeOne.description = "Displays the current invite-counts.";
				createSetInvitesChannelOptionThreeOne.required = true;
				ApplicationCommandOptionChoiceData createSetInvitesChoiceThree;
				createSetInvitesChoiceThree.name = "view";
				createSetInvitesChoiceThree = "view";
				createSetInvitesChannelOptionThreeOne.choices.push_back(createSetInvitesChoiceThree);
				createSetInvitesChannelOptionThree.options.push_back(createSetInvitesChannelOptionThreeOne);
				createSetInvitesChannelCommandData.options.push_back(createSetInvitesChannelOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSetInvitesChannelCommandData).get();
				
				CreateGlobalApplicationCommandData createBanCommandData{};
				createBanCommandData.dmPermission = false;
				createBanCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createBanCommandData.defaultPermission = true;
				createBanCommandData.description = "Ban a user or look at the banner's leaderboard.";
				createBanCommandData.name = "ban";
				createBanCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createBanOptionThree;
				createBanOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				createBanOptionThree.name = "view";
				createBanOptionThree.description = "Displays the current leaderboard of banners.";
				createBanCommandData.options.push_back(createBanOptionThree);
				ApplicationCommandOptionData createBanOptionOne;
				createBanOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createBanOptionOne.name = "add";
				createBanOptionOne.description = "Adds a user to the banned list.";
				ApplicationCommandOptionData createBanOptionOneOne;
				createBanOptionOneOne.type = ApplicationCommandOptionType::User;
				createBanOptionOneOne.name = "user";
				createBanOptionOneOne.description = "The user to ban.";
				createBanOptionOneOne.required = true;
				createBanOptionOne.options.push_back(createBanOptionOneOne);
				ApplicationCommandOptionData createBanOptionOneTwo;
				createBanOptionOneTwo.type = ApplicationCommandOptionType::String;
				createBanOptionOneTwo.name = "reason";
				createBanOptionOneTwo.description = "The reason for the ban.";
				createBanOptionOneTwo.required = true;
				createBanOptionOne.options.push_back(createBanOptionOneTwo);
				ApplicationCommandOptionData createBanOptionOneThree;
				createBanOptionOneThree.type = ApplicationCommandOptionType::Integer;
				createBanOptionOneThree.minValue = 1;
				createBanOptionOneThree.maxValue = 7;
				createBanOptionOneThree.name = "numberofdaystopurge";
				createBanOptionOneThree.description = "The number of days of the user's messages to purge.";
				createBanOptionOneThree.required = false;
				createBanOptionOne.options.push_back(createBanOptionOneThree);
				createBanCommandData.options.push_back(createBanOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(createBanCommandData).get();

				CreateGlobalApplicationCommandData createSetDeletionStatusCommandData{};
				createSetDeletionStatusCommandData.dmPermission = false;
				createSetDeletionStatusCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSetDeletionStatusCommandData.defaultPermission = true;
				createSetDeletionStatusCommandData.description = "Enables or disables message-purging in a given channel.";
				createSetDeletionStatusCommandData.name = "setdeletionstatus";
				createSetDeletionStatusCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createSetDeletionStatusOptionOne;
				createSetDeletionStatusOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createSetDeletionStatusOptionOne.name = "add";
				createSetDeletionStatusOptionOne.description = "Add the current channel as a message-purging channel.";

				ApplicationCommandOptionData createSetDeletionStatusOptionOneTwo;
				createSetDeletionStatusOptionOneTwo.type = ApplicationCommandOptionType::Integer;
				createSetDeletionStatusOptionOneTwo.name = "quantity";
				createSetDeletionStatusOptionOneTwo.minValue = 0;
				createSetDeletionStatusOptionOneTwo.maxValue = 100000;
				createSetDeletionStatusOptionOneTwo.description = "The number of messages to save in the channel.";
				createSetDeletionStatusOptionOneTwo.required = true;
				createSetDeletionStatusOptionOne.options.push_back(createSetDeletionStatusOptionOneTwo);

				ApplicationCommandOptionData createSetDeletionStatusOptionOneThree;
				createSetDeletionStatusOptionOneThree.type = ApplicationCommandOptionType::Integer;
				createSetDeletionStatusOptionOneThree.minValue = 0;
				createSetDeletionStatusOptionOneThree.maxValue = 100000;
				createSetDeletionStatusOptionOneThree.name = "minutestosave";
				createSetDeletionStatusOptionOneThree.description = "The number of minutes to save the messages before deleting them.";
				createSetDeletionStatusOptionOneThree.required = true;
				createSetDeletionStatusOptionOne.options.push_back(createSetDeletionStatusOptionOneThree);
				createSetDeletionStatusCommandData.options.push_back(createSetDeletionStatusOptionOne);

				ApplicationCommandOptionData createSetDeletionStatusOptionTwo;
				createSetDeletionStatusOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				createSetDeletionStatusOptionTwo.name = "remove";
				createSetDeletionStatusOptionTwo.description = "remove the current channel as a message-purging channel.";

				createSetDeletionStatusCommandData.options.push_back(createSetDeletionStatusOptionTwo);
				ApplicationCommandOptionData createSetDeletionStatusOptionThree;
				createSetDeletionStatusOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				createSetDeletionStatusOptionThree.name = "view";
				createSetDeletionStatusOptionThree.description = "Displays the current list of message-purging channels.";

				createSetDeletionStatusCommandData.options.push_back(createSetDeletionStatusOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSetDeletionStatusCommandData).get();
				
				CreateGlobalApplicationCommandData createUserInfoData{};
				createUserInfoData.dmPermission = false;
				createUserInfoData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createUserInfoData.type = ApplicationCommandType::User;
				createUserInfoData.name = "User Info";
				createUserInfoData.defaultPermission = true;
				ApplicationCommands::createGlobalApplicationCommandAsync(createUserInfoData).get();

				CreateGlobalApplicationCommandData RegisterApplicationCommandsCommandData{};
				RegisterApplicationCommandsCommandData.dmPermission = false;
				RegisterApplicationCommandsCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				RegisterApplicationCommandsCommandData.type = ApplicationCommandType::Chat_Input;
				RegisterApplicationCommandsCommandData.defaultPermission = true;
				RegisterApplicationCommandsCommandData.description = "Register the programmatically designated slash commands.";
				RegisterApplicationCommandsCommandData.name = "registerapplicationcommands";
				auto theResult02 = ApplicationCommands::createGlobalApplicationCommandAsync(RegisterApplicationCommandsCommandData).get();
				
				CreateGlobalApplicationCommandData createTestData{};
				createTestData.dmPermission = true;
				createTestData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createTestData.type = ApplicationCommandType::Chat_Input;
				createTestData.name = "test";
				createTestData.defaultPermission = true;
				createTestData.description = "Test command.";
				ApplicationCommandOptionData testOptionOne{};
				testOptionOne.type = ApplicationCommandOptionType::Attachment;
				testOptionOne.name = "attachment";
				testOptionOne.required = false;
				testOptionOne.description = "Test Attachment!";
				createTestData.options.push_back(testOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(createTestData).get();
				
				CreateGlobalApplicationCommandData createManageLogsData{};
				createManageLogsData.dmPermission = false;
				createManageLogsData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createManageLogsData.type = ApplicationCommandType::Chat_Input;
				createManageLogsData.name = "managelogs";
				createManageLogsData.defaultPermission = true;
				createManageLogsData.description = "Check the status and/or set the status of various logs.";
				ApplicationCommandOptionData createManageLogsOptionOh;
				createManageLogsOptionOh.type = ApplicationCommandOptionType::Sub_Command;
				createManageLogsOptionOh.description = "View the currently enabled/disabled logs.";
				createManageLogsOptionOh.name = "view";
				createManageLogsData.options.push_back(createManageLogsOptionOh);
				ApplicationCommandOptionData createManageLogsOptionOne;
				createManageLogsOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createManageLogsOptionOne.description = "Group 1 of the potential logs to set.";
				createManageLogsOptionOne.name = "group1";
				ApplicationCommandOptionData createManageLogsOptionOhOne;
				createManageLogsOptionOhOne.name = "settowhat";
				createManageLogsOptionOhOne.description = "Enable or disable the currently selected log?";
				createManageLogsOptionOhOne.required = true;
				createManageLogsOptionOhOne.type = ApplicationCommandOptionType::Boolean;
				createManageLogsOptionOne.options.push_back(createManageLogsOptionOhOne);
				ApplicationCommandOptionData createManageLogsOptionOneOne;
				createManageLogsOptionOneOne.name = "group1option";
				createManageLogsOptionOneOne.description = "Select which log you would like to set.";
				createManageLogsOptionOneOne.required = true;
				createManageLogsOptionOneOne.type = ApplicationCommandOptionType::String;
				for (int32_t x = 0; x < 10; x += 1) {
					ApplicationCommandOptionChoiceData choiceDataOne;
					choiceDataOne.name = discordGuild.data.logs[x].name;
					choiceDataOne = discordGuild.data.logs[x].nameSmall;
					createManageLogsOptionOneOne.choices.push_back(choiceDataOne);
				}
				createManageLogsOptionOne.options.push_back(createManageLogsOptionOneOne);
				createManageLogsData.options.push_back(createManageLogsOptionOne);
				ApplicationCommandOptionData createManageLogsOptionTwo;
				createManageLogsOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				createManageLogsOptionTwo.description = "Group 2 of the potential logs to set.";
				createManageLogsOptionTwo.name = "group2";
				ApplicationCommandOptionData createManageLogsOptionOhTwo;
				createManageLogsOptionOhTwo.name = "settowhat";
				createManageLogsOptionOhTwo.description = "Enable or disable the currently selected log?";
				createManageLogsOptionOhTwo.required = true;
				createManageLogsOptionOhTwo.type = ApplicationCommandOptionType::Boolean;
				createManageLogsOptionTwo.options.push_back(createManageLogsOptionOhTwo);
				ApplicationCommandOptionData createManageLogsOptionTwoOne;
				createManageLogsOptionTwoOne.name = "group2option";
				createManageLogsOptionTwoOne.description = "Select which log you would like to set.";
				createManageLogsOptionTwoOne.required = true;
				createManageLogsOptionTwoOne.type = ApplicationCommandOptionType::String;
				for (int32_t x = 10; x < discordGuild.data.logs.size(); x += 1) {
					ApplicationCommandOptionChoiceData choiceDataOne;
					choiceDataOne.name = discordGuild.data.logs[x].name;
					choiceDataOne = discordGuild.data.logs[x].nameSmall;
					createManageLogsOptionTwoOne.choices.push_back(choiceDataOne);
				}
				createManageLogsOptionTwo.options.push_back(createManageLogsOptionTwoOne);
				createManageLogsData.options.push_back(createManageLogsOptionTwo);
				ApplicationCommands::createGlobalApplicationCommandAsync(createManageLogsData).get();

				CreateGlobalApplicationCommandData createHelpData{};
				createHelpData.dmPermission = true;
				createHelpData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createHelpData.type = ApplicationCommandType::Chat_Input;
				createHelpData.name = "help";
				createHelpData.defaultPermission = true;
				createHelpData.description = "A help command for this bot.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createHelpData).get();

				CreateGlobalApplicationCommandData createAvatarData{};
				createAvatarData.dmPermission = false;
				createAvatarData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createAvatarData.defaultPermission = true;
				createAvatarData.type = ApplicationCommandType::Chat_Input;
				createAvatarData.description = "Displays the avatar of a chosen user.";
				createAvatarData.name = "avatar";
				ApplicationCommandOptionData createAvatarOptionOne{};
				createAvatarOptionOne.type = ApplicationCommandOptionType::User;
				createAvatarOptionOne.name = "user";
				createAvatarOptionOne.description = "Which of the users to view the avatar of.";
				createAvatarOptionOne.required = true;
				createAvatarData.options.push_back(createAvatarOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(createAvatarData).get();
				
				CreateGlobalApplicationCommandData pushmecommandData{};
				pushmecommandData.dmPermission = false;
				pushmecommandData.name = "pushme";
				pushmecommandData.type = ApplicationCommandType::Chat_Input;
				pushmecommandData.description = "Push me to find out!";
				pushmecommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				ApplicationCommands::createGlobalApplicationCommandAsync(pushmecommandData).get();
				
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor("FeFeFe");
				msgEmbed.setDescription("------\nNicely done, you've registered some commands!\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Register Application Commands Complete:**__");
				RespondToInputEventData responseData(newEvent);
				responseData.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				responseData.addMessageEmbed(msgEmbed);
				auto event = InputEvents::respondToInputEventAsync(responseData).get();
				return;
			} catch (...) {
				reportException("RegisterApplicationCommands::execute()");
			}
		}
		virtual ~RegisterApplicationCommands(){};
	};

}
