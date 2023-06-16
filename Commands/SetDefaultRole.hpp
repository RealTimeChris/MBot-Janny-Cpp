// SetDefaultRole.hpp - Header for the "add new user role" command.
// Sep 4, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	void addNewUserRolesAsync(GuildData guild) {
		try {
			DiscordGuild discordGuild{ managerAgent, guild };
			for (auto& value: guild.members) {
				GetGuildMemberData theData{};
				theData.guildId		  = guild.id;
				theData.guildMemberId = value.user.id;
				auto guildMemberNew	  = GuildMembers::getCachedGuildMember(theData);
				DiscordGuildMember guildMember(managerAgent, guildMemberNew);
				if (guildMember.data.previousRoleIds.size() == 0) {
					for (auto& value01: discordGuild.data.defaultRoleIds) {
						bool isItFound = false;
						for (auto& value02: guildMemberNew.roles) {
							if (value02 == value01) {
								isItFound = true;
								break;
							}
						}
						if (!isItFound) {
							Roles::addGuildMemberRoleAsync({ .guildId = guild.id, .reason = "New User.", .userId = value.user.id.operator const uint64_t&(), .roleId = value01 })
								.get();
						}
					}
				}
			}
			return;
		} catch (const std::exception& error) {
			std::cout << "addNewUserRolesAsync Error: " << error.what() << std::endl;
		}
	}

	CoRoutine<void> addNewUserRoleAsync(const GuildMemberData guildMember) {
		co_await NewThreadAwaitable<void>();
		GuildData guild = Guilds::getCachedGuild({ .guildId = guildMember.guildId });
		DiscordGuild discordGuild{ managerAgent, guild };

		for (auto& value: discordGuild.data.defaultRoleIds) {
			Roles::addGuildMemberRoleAsync({ .guildId = guild.id, .reason = "New User.", .userId = guildMember.user.id, .roleId = value }).get();
		}
	}

	class SetDefaultRole : public BaseFunction {
	  public:
		SetDefaultRole() {
			this->commandName	  = "setdefaultrole";
			this->helpDescription = "Sets the default role(s) to add to new users.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription(
				"------\nJust enter /setdefaultrole view to view the current list of default roles!\nEnter /setdefaultrole add, rolename to add a role as a default \
                for when someone new joins the server.\n/setdefaultrole = remove, rolename to remove a role from the list.\n------");
			msgEmbed.setTitle("__**Add New UserData RoleData Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetDefaultRole>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };

				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, true);

				if (!doWeHaveAdminPermission) {
					return;
				}

				jsonifier::string whatAreWeDoing{};
				Snowflake roleId{};
				if (argsNew.getSubCommandName() == "view") {
					whatAreWeDoing = "view";
				}
				if (argsNew.getCommandArguments().values.size() > 0) {
					if (convertToLowerCase(argsNew.getSubCommandName()) == "add") {
						whatAreWeDoing = "add";
					} else if (convertToLowerCase(argsNew.getSubCommandName()) == "remove") {
						whatAreWeDoing = "remove";
					}
				}
				if (argsNew.getCommandArguments().values.size() > 0) {
					roleId = std::stoull(argsNew.getCommandArguments().values["role"].value.operator jsonifier::string().data());
				}

				jsonifier::vector<RoleData> roleArray = Roles::getGuildRolesAsync({ .guildId = guild.id }).get();

				jsonifier::vector<bool> isItFoundReal;
				for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
					bool isItFound = false;
					for (auto& value: roleArray) {
						if (value.id == discordGuild.data.defaultRoleIds[x]) {
							isItFound = true;
							break;
						}
					}
					isItFoundReal.emplace_back(isItFound);
					bool isItFoundFinal = false;
					for (int32_t y = 0; y < isItFoundReal.size(); y += 1) {
						if (isItFoundReal[y] == true) {
							isItFoundFinal = true;
						}
					}
					if (isItFoundFinal == false) {
						std::cout << "Removing a missing guild role from the list of defaults." << std::endl;
						discordGuild.data.defaultRoleIds.erase(discordGuild.data.defaultRoleIds.begin() + x);
						discordGuild.writeDataToDB(managerAgent);
					}
				}

				if (whatAreWeDoing == "view") {
					jsonifier::string msgString;

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

					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Default Roles:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				RoleData currentRole = Roles::getRoleAsync({ .guildId = guild.id, .roleId = roleId }).get();

				bool isItFound = false;
				for (auto& value: roleArray) {
					if (roleId == value.id) {
						isItFound = true;
						break;
					}
				}

				if (!isItFound) {
					jsonifier::string msgString = "------\n**Sorry, but the role you entered could not be found! Check spelling and case!**\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**RoleData Issue:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (whatAreWeDoing == "add") {
					for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
						if (currentRole.id == discordGuild.data.defaultRoleIds[x]) {
							jsonifier::string msgString = "------\n**Hey! It looks like you've already added that role!**\n------";
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setTitle("__**RoleData Issue:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(msgString);
							RespondToInputEventData dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
							return;
						}
					}

					GuildMemberData botGuildMember = GuildMembers::getCachedGuildMember({ .guildMemberId = DiscordCoreClient::getInstance()->getBotUser().id, .guildId = guild.id });
					jsonifier::vector<RoleData> highestBotRoles = Roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = guild.id }).get();
					RoleData highestBotRole;
					int32_t currentPosition = 0;
					for (auto& value: highestBotRoles) {
						if (value.position > currentPosition) {
							highestBotRole	= value;
							currentPosition = value.position;
						}
					}

					if (currentRole.position > highestBotRole.position || currentRole.getFlagValue(RoleFlags::Managed)) {
						jsonifier::string msgString = "------\n**Sorry, but that is either a managed role or it is higher than my highest role! I cannot use it!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**RoleData Issue:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.defaultRoleIds.emplace_back(currentRole.id);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString = "\n------\n__**RoleData:**__ <@&" + currentRole.id + "> \n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**New Default RoleData Added:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (whatAreWeDoing == "remove") {
					isItFound = false;
					for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
						if (currentRole.id == discordGuild.data.defaultRoleIds[x]) {
							discordGuild.data.defaultRoleIds.erase(discordGuild.data.defaultRoleIds.begin() + x);
							discordGuild.writeDataToDB(managerAgent);
							isItFound = true;
						}
					}

					if (!isItFound) {
						jsonifier::string msgString = "------\n**Sorry, but the role you entered could not be found! Check spelling and case!**\n------";
						UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed->setColor(discordGuild.data.borderColor);
						msgEmbed->setTitle("__**Missing/Invalud Arguments:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
						return;
					}

					jsonifier::string msgString = "\n------\n__**RoleData**__: <@&" + currentRole.id + ">\n------";

					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setTitle("__**Default RoleData removed:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
			} catch (const std::exception& error) {
				std::cout << "SetDefaultRole::execute()" << error.what() << std::endl;
			}
		}
		~SetDefaultRole(){};
	};

}// namespace DiscordCoreAPI
