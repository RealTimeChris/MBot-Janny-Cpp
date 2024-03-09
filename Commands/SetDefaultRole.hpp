// SetDefaultRole.hpp - Header for the "add new user role" command.
// sep 4, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	void addNewUserRolesAsync(guild_data guild) {
		try {
			discord_guild discordGuild{ managerAgent, guild };
			for (auto& value: guild.members) {
				get_guild_member_data theData{};
				theData.guildId		  = guild.id;
				theData.guildMemberId = value.user.id;
				auto guildMemberNew	  = guild_members::getCachedGuildMember(theData);
				discord_guild_member guildMember(managerAgent, guildMemberNew);
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
							roles::addGuildMemberRoleAsync({ .guildId = guild.id, .reason = "new user.", .userId = value.user.id.operator const uint64_t&(), .roleId = value01 })
								.get();
						}
					}
				}
			}
			return;
		} catch (const std::exception& error) {
			std::cout << "addNewUserRolesAsync error: " << error.what() << std::endl;
		}
	}

	co_routine<void> addNewUserRoleAsync(const guild_member_data guildMember) {
		co_await newThreadAwaitable<void>();
		guild_data guild = guilds::getCachedGuild({ .guildId = guildMember.guildId });
		discord_guild discordGuild{ managerAgent, guild };

		for (auto& value: discordGuild.data.defaultRoleIds) {
			roles::addGuildMemberRoleAsync({ .guildId = guild.id, .reason = "new user.", .userId = guildMember.user.id, .roleId = value }).get();
		}
	}

	class set_default_role : public base_function {
	  public:
		set_default_role() {
			this->commandName	  = "setdefaultrole";
			this->helpDescription = "sets the default role(s) to add to new users.";
			embed_data msgEmbed{};
			msgEmbed.setDescription(
				"------\nJust enter /setdefaultrole view to view the current list of default roles!\nEnter /setdefaultrole add, rolename to add a role as a default \
                for when someone new joins the server.\n/setdefaultrole = remove, rolename to remove a role from the list.\n------");
			msgEmbed.setTitle("__**add new user_data Role_Data usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<set_default_role>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, true);

				if (!doWeHaveAdminPermission) {
					return;
				}

				jsonifier::string whatAreWeDoing{};
				snowflake roleId{};
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
					roleId = argsNew.getCommandArguments().values["role"].operator size_t();
				}

				jsonifier::vector<role_data> roleArray = roles::getGuildRolesAsync({ .guildId = guild.id }).get();

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
						std::cout << "removing a missing guild role from the list of defaults." << std::endl;
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

					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**default roles:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				role_data currentRole = roles::getRoleAsync({ .guildId = guild.id, .roleId = roleId }).get();

				bool isItFound = false;
				for (auto& value: roleArray) {
					if (roleId == value.id) {
						isItFound = true;
						break;
					}
				}

				if (!isItFound) {
					jsonifier::string msgString = "------\n**sorry, but the role you entered could not be found! check spelling and case!**\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**Role_Data issue:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (whatAreWeDoing == "add") {
					for (int32_t x = 0; x < discordGuild.data.defaultRoleIds.size(); x += 1) {
						if (currentRole.id == discordGuild.data.defaultRoleIds[x]) {
							jsonifier::string msgString = "------\n**hey! it looks like you've already added that role!**\n------";
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setColor("fefefe");
							msgEmbed->setTitle("__**Role_Data issue:**__");
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setDescription(msgString);
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
							return;
						}
					}

					guild_member_data botGuildMember =
						guild_members::getCachedGuildMember({ .guildMemberId = discord_core_client::getInstance()->getBotUser().id, .guildId = guild.id });
					jsonifier::vector<role_data> highestBotRoles = roles::getGuildMemberRolesAsync({ .guildMember = botGuildMember, .guildId = guild.id }).get();
					role_data highestBotRole;
					int32_t currentPosition = 0;
					for (auto& value: highestBotRoles) {
						if (value.position > currentPosition) {
							highestBotRole	= value;
							currentPosition = value.position;
						}
					}

					if (currentRole.position > highestBotRole.position || currentRole.getFlagValue(role_flags::managed)) {
						jsonifier::string msgString = "------\n**sorry, but that is either a managed role or it is higher than my highest role! i cannot use it!**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setColor("fefefe");
						msgEmbed->setTitle("__**Role_Data issue:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.defaultRoleIds.emplace_back(currentRole.id);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString = "\n------\n__**Role_Data:**__ <@&" + currentRole.id + "> \n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**new default Role_Data added:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
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
						jsonifier::string msgString = "------\n**sorry, but the role you entered could not be found! check spelling and case!**\n------";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setColor("fefefe");
						msgEmbed->setTitle("__**missing/invalud arguments:**__");
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setDescription(msgString);
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					}

					jsonifier::string msgString = "\n------\n__**Role_Data**__: <@&" + currentRole.id + ">\n------";

					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setTitle("__**default Role_Data removed:**__");
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setDescription(msgString);
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					auto argsNewer = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
			} catch (const std::exception& error) {
				std::cout << "set_default_role::execute()" << error.what() << std::endl;
			}
		}
		~set_default_role(){};
	};

}// namespace discord_core_api
