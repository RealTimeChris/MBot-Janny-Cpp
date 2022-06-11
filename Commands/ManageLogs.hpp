// ManageLogs.hpp - Header for the "manage logs" command.
// Aug 10, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class ManageLogs : public BaseFunction {
	  public:
		ManageLogs() {
			this->commandName = "managelogs";
			this->helpDescription = "Views the status for the given possible logs.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /managelogs, to view an enabled/disabled list of possible logs.\n------");
			msgEmbed.setTitle("__**Manage Logs Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<ManageLogs>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();

				DiscordGuild discordGuild{ guild };

				GuildMember guildMember = GuildMembers::getCachedGuildMemberAsync({
																					  .guildMemberId = newArgs.eventData.getAuthorId(),
																					  .guildId = newArgs.eventData.getGuildId(),
																				  })
											  .get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				if (newArgs.commandData.optionsArgs.size() > 0) {
					if (newArgs.commandData.optionsArgs[0] == "true") {
						newArgs.commandData.optionsArgs[0] = "enable";
					} else if (newArgs.commandData.optionsArgs[0] == "false") {
						newArgs.commandData.optionsArgs[0] = "disable";
					}
				}

				if (newArgs.commandData.optionsArgs.size() == 0) {
					std::vector<EmbedFieldData> fields;
					for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
						if (Channels::getCachedChannelAsync({ .channelId = discordGuild.data.logs[x].loggingChannelId }).get().name == "") {
							discordGuild.data.logs[x].loggingChannelName = "";
							discordGuild.data.logs[x].loggingChannelId = 0;
							discordGuild.data.logs[x].enabled = false;
						}
						if (discordGuild.data.logs[x].enabled == false) {
							EmbedFieldData field = { .Inline = true, .value = "__Enabled:__ ❌", .name = "__**" + discordGuild.data.logs[x].name + "**__" };
							fields.push_back(field);
						} else if (discordGuild.data.logs[x].enabled == true) {
							EmbedFieldData field = { .Inline = true,
								.value = "__Enabled:__ ✅\n__Logging Channel:__ <#" + std::to_string(discordGuild.data.logs[x].loggingChannelId) + ">",
								.name = "__**" + discordGuild.data.logs[x].name + "**__" };
							fields.push_back(field);
						}
					}

					std::string msgString = "**To enable/disable a given log, enter within the text channel where you would like it to be logged: !managelogs "
											"= <enable/disable>, <logname>\nFor example, '!managelogs = "
											"enable, guildbanadd'.**'";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					*msgEmbed = EmbedData()
									.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl())
									.setColor(discordGuild.data.borderColor)
									.setDescription(msgString)
									.setTimeStamp(getTimeAndDate())
									.setTitle("__**Manage Logs:**__");
					for (auto value: fields) {
						msgEmbed->addField(value.name, value.value, value.Inline);
					}
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				else if (convertToLowerCase(newArgs.commandData.optionsArgs[0]) != "enable" && convertToLowerCase(newArgs.commandData.optionsArgs[0]) != "disable") {
					std::string msgString = "------\n**Please, enter enable or disable for the first argument of this command! (!managelogs = "
											"<enable/disable>, <logname>)**\n------";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (newArgs.commandData.optionsArgs[1] == "") {
					std::string msgString = "------\n**Please, enter a log name to disable or enable as the second argument of this command!";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					if (convertToLowerCase(newArgs.commandData.optionsArgs[0]) == "enable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(newArgs.commandData.optionsArgs[1]) == discordGuild.data.logs[x].nameSmall) {
								isItFound = true;
								Channel channelNew = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();
								discordGuild.data.logs[x].loggingChannelId = channelNew.id;
								discordGuild.data.logs[x].loggingChannelName = channelNew.name;
								discordGuild.data.logs[x].enabled = true;
								discordGuild.writeDataToDB();
								std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
								std::string msgString = "------\n**Nicely done! You've enabled logging for " + discordGuild.data.logs[x].name + ".\nIn channel <#" +
									std::to_string(discordGuild.data.logs[x].loggingChannelId) + ">.** \n------";
								msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
								msgEmbed->setColor(discordGuild.data.borderColor);
								msgEmbed->setDescription(msgString);
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**Manage Logs Enabled:**__");
								RespondToInputEventData dataPackage(newArgs.eventData);
								dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								InputEvents::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							std::string msgString = "Please enter a proper log name!";
							std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
							msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Manage Logs Issue:**__");
							RespondToInputEventData dataPackage(newArgs.eventData);
							dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							InputEvents::respondToInputEventAsync(dataPackage).get();
						}
					} else if (convertToLowerCase(newArgs.commandData.optionsArgs[0]) == "disable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(newArgs.commandData.optionsArgs[1]) == discordGuild.data.logs[x].nameSmall) {
								isItFound = true;
								Channel channelNew = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();
								discordGuild.data.logs[x].loggingChannelId = 0;
								discordGuild.data.logs[x].loggingChannelName = "";
								discordGuild.data.logs[x].enabled = false;
								discordGuild.writeDataToDB();
								std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
								std::string msgString = "------\n**Nicely done! You've disabled logging for " + discordGuild.data.logs[x].name + ".**\n------";
								msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
								msgEmbed->setColor(discordGuild.data.borderColor);
								msgEmbed->setDescription(msgString);
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**Manage Logs Disabled:**__");
								RespondToInputEventData dataPackage(newArgs.eventData);
								dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								InputEvents::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							std::string msgString = "Please enter a proper log name!";
							std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
							msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Manage Logs Issue:**__");
							RespondToInputEventData dataPackage(newArgs.eventData);
							dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							InputEvents::respondToInputEventAsync(dataPackage).get();
						}
					}
				}
				return;
			} catch (...) {
				reportException("ManageLogs::execute()");
			}
		}
		~ManageLogs(){};
	};

}
