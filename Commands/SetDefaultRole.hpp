// SetDefaultRole.hpp - Header for the "add new user role" command.
// Sep 4, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	CoRoutine<void> addNewUserRolesAsync(Guild guild) {
		try {
			co_await NewThreadAwaitable<void>();
			DiscordGuild discordGuild{ guild };
			for (auto& [key, value]: guild.members) {
				DiscordGuildMember guildMember(value);
				if (guildMember.data.previousRoleIds.size() == 0) {
					for (auto& value01: discordGuild.data.defaultRoleIds) {
						bool isItFound = false;
						for (auto& value02: value.roles) {
							if (value02 == value01) {
								isItFound = true;
								break;
							}
						}
						if (!isItFound) {
							Roles::addGuildMemberRoleAsync({ .guildId = guild.id, .userId = value.user.id, .roleId = value01, .reason = "New User." }).get();
						}
					}
				}
			}
			co_return;
		} catch (...) {
			reportException("addNewUserRolesAsync Error: ");
		}
	}

	CoRoutine<void> addNewUserRoleAsync(GuildMember guildMember) {
		co_await NewThreadAwaitable<void>();
		Guild guild = Guilds::getCachedGuildAsync({ .guildId = guildMember.guildId }).get();
		DiscordGuild discordGuild{ guild };

		for (auto& value: discordGuild.data.defaultRoleIds) {
			Roles::addGuildMemberRoleAsync({ .guildId = guild.id, .userId = guildMember.user.id, .roleId = value, .reason = "New User." }).get();
		}
	}

	class SetDefaultRole : public BaseFunction {
	  public:
		SetDefaultRole() {
			this->commandName = "setdefaultrole";
			this->helpDescription = "Sets the default role(s) to add to new users.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription(
				"------\nJust enter /setdefaultrole view to view the current list of default roles!\nEnter /setdefaultrole add, rolename to add a role as a default \
                for when someone new joins the server.\n/setdefaultrole = remove, rolename to remove a role from the list.\n------");
			msgEmbed.setTitle("__**Add New User Role Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<SetDefaultRole>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				DiscordCoreAPI::Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = newArgs.eventData.getAuthorId(), .guildId = newArgs.eventData.getGuildId() })
						.get();

				bool doWeHaveAdminPerms = doWeHaveAdminPermissions(newArgs, newArgs.eventData, discordGuild, channel, guildMember);

				if (!doWeHaveAdminPerms) {
					return;
				}

				std::string whatAreWeDoing;
				std::string roleId;
				if (newArgs.commandData.subCommandName == "view") {
					whatAreWeDoing = "view";
				}
				if (newArgs.commandData.optionsArgs.size() > 0) {
					if (convertToLowerCase(newArgs.commandData.subCommandName) == "add") {
						whatAreWeDoing = "add";
					} else if (convertToLowerCase(newArgs.commandData.subCommandName) == "remove") {
						whatAreWeDoing = "remove";
					}
				}
				if (newArgs.commandData.optionsArgs.size() > 0) {
					roleId = newArgs.commandData.optionsArgs[0];
					std::cout << "THE ROLE ID: " << newArgs.commandData.optionsArgs[0] << std::endl;
				}

				std::vector<Role> roleArray = Roles::getGuildRolesAsync({ .guildId = newArgs.eventData.getGuildId() }).get();

				std::vector<bool> isItFoundReal;
				for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
					bool isItFound = false;
					for (auto& value: roleArray) {
						if (value.id == discordGuild.data.defaultRoleIds[x]) {
							isItFound = true;
							break;
						}
					}
					isItFoundReal.push_back(isItFound);
					bool isItFoundFinal = false;
					for (int32_t y = 0; y < isItFoundReal.size(); y += 1) {
						if (isItFoundReal[y] == true) {
							isItFoundFinal = true;
						}
					}
					if (isItFoundFinal == false) {
						std::cout << "Removing a missing guild role from the list of defaults." << std::endl;
						discordGuild.data.defaultRoleIds.erase(discordGuild.data.defaultRoleIds.begin() + x, discordGuild.data.defaultRoleIds.begin() + x + 1);
						discordGuild.writeDataToDB();
					}
				}

				if (whatAreWeDoing == "view") {
					std::string msgString;

					if (discordGuild.data.defaultRoleIds.size() > 0) {
						msgString = "\n------\n";
						for (auto& value: roleArray) {
							for (auto& value02: discordGuild.data.defaultRoleIds) {
								if (value.id == value02) {
									msgString += "<@&" + value02 + ">\n";
								}
							}
						}
						msgString += "------";
					} else {
						msgString = "------\n__You don't have any default roles!__\n------";
					}

					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Default Roles:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
					return;
				}

				Role currentRole = Roles::getRoleAsync({ .guildId = newArgs.eventData.getGuildId(), .roleId = roleId }).get();

				bool isItFound = false;
				for (auto& value: roleArray) {
					if (roleId == value.id) {
						isItFound = true;
						break;
					}
				}

				if (!isItFound) {
					std::string msgString = "------\n**Sorry, but the role you entered could not be found! Check spelling and case!**\n------";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Role Issue:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
					return;
				}

				if (whatAreWeDoing == "add") {
					for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
						if (currentRole.id == discordGuild.data.defaultRoleIds[x]) {
							std::string msgString = "------\n**Hey! It looks like you've already added that role!**\n------";
							std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
							msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setTitle("__**Role Issue:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(msgString);
							RespondToInputEventData dataPackage(newArgs.eventData);
							dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
							return;
						}
					}

					GuildMember botGuildMember = GuildMembers::getCachedGuildMemberAsync(
						{ .guildMemberId = newArgs.discordCoreClient->getBotUser().id, .guildId = newArgs.eventData.getGuildId() })
													 .get();
					std::vector<Role> highestBotRoles =
						Roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = newArgs.eventData.getGuildId() }).get();
					RoleData highestBotRole;
					int32_t currentPosition = 0;
					for (auto& value: highestBotRoles) {
						if (value.position > currentPosition) {
							highestBotRole = value;
							currentPosition = value.position;
						}
					}

					if (currentRole.position > highestBotRole.position || currentRole.getManaged()) {
						std::string msgString =
							"------\n**Sorry, but that is either a managed role or it is higher than my highest role! I cannot use it!**\n------";
						std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**Role Issue:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.defaultRoleIds.push_back(currentRole.id);
					discordGuild.writeDataToDB();

					std::string msgString = "\n------\n__**Role:**__ <@&" + currentRole.id + "> \n------";
					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**New Default Role Added:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
					return;
				}
				if (whatAreWeDoing == "remove") {
					isItFound = false;
					for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
						if (currentRole.id == discordGuild.data.defaultRoleIds[x]) {
							discordGuild.data.defaultRoleIds.erase(
								discordGuild.data.defaultRoleIds.begin() + x, discordGuild.data.defaultRoleIds.begin() + x + 1);
							discordGuild.writeDataToDB();
							isItFound = true;
						}
					}

					if (!isItFound) {
						std::string msgString = "------\n**Sorry, but the role you entered could not be found! Check spelling and case!**\n------";
						std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
						msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**Missing/Invalud Arguments:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						RespondToInputEventData dataPackage(newArgs.eventData);
						dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
						return;
					}

					std::string msgString = "\n------\n__**Role**__: <@&" + currentRole.id + ">\n------";

					std::unique_ptr<EmbedData> msgEmbed{ std::make_unique<EmbedData>() };
					msgEmbed->setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Default Role removed:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
					return;
				}
			} catch (...) {
				reportException("SetDefaultRole::execute()");
			}
		}
		virtual ~SetDefaultRole(){};
	};

}
