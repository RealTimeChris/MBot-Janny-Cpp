// ManageLogs.hpp - Header for the "manage logs" command.
// aug 10, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class manage_logs : public base_function {
	  public:
		manage_logs() {
			this->commandName	  = "managelogs";
			this->helpDescription = "views the status for the given possible logs.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /managelogs, to view an enabled/disabled list of possible logs.\n------");
			msgEmbed.setTitle("__**manage logs usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<manage_logs>();
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
				jsonifier::string setToWhat{};
				if (argsNew.getCommandArguments().values.size() > 0) {
					if (argsNew.getCommandArguments().values["settowhat"].operator bool() == true) {
						setToWhat = "enable";
					} else if (argsNew.getCommandArguments().values["settowhat"].operator bool() == false) {
						setToWhat = "disable";
					}
				}

				if (argsNew.getCommandArguments().values.size() == 0) {
					jsonifier::vector<embed_field_data> fields;
					for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
						if (channels::getCachedChannel({ .channelId = discordGuild.data.logs[x].loggingChannelId }).name == "") {
							discordGuild.data.logs[x].loggingChannelName = "";
							discordGuild.data.logs[x].loggingChannelId	 = 0;
							discordGuild.data.logs[x].enabled			 = false;
						}
						if (discordGuild.data.logs[x].enabled == false) {
							embed_field_data field{};
							field.Inline = true;
							field.value	 = "__Enabled:__ ❌";
							field.name	 = "__**" + discordGuild.data.logs[x].name + "**__";
							fields.emplace_back(field);
						} else if (discordGuild.data.logs[x].enabled == true) {
							embed_field_data field{};
							field.Inline = true;
							field.value	 = "__Enabled:__ ✅\n__Logging channel_data:__ <#" + discordGuild.data.logs[x].loggingChannelId + ">";
							field.name	 = "__**" + discordGuild.data.logs[x].name + "**__";
							fields.emplace_back(field);
						}
					}

					jsonifier::string msgString = "**to enable/disable a given log, enter within the text channel where you would like it to be logged: !managelogs "
											"= <enable/disable>, <logname>\nFor example, '!managelogs = "
											"enable, guildbanadd'.**'";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					*msgEmbed = embed_data()
									.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>())
									.setColor("fefefe")
									.setDescription(msgString)
									.setTimeStamp(getTimeAndDate())
									.setTitle("__**manage logs:**__");
					for (auto& value: fields) {
						msgEmbed->addField(value.name, value.value, value.Inline);
					}
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				else if (setToWhat != "enable" && setToWhat != "disable") {
					jsonifier::string msgString = "------\n**please, enter enable or disable for the first argument of this command! (!managelogs = "
											"<enable/disable>, <logname>)**\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**missing or invalid arguments:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getCommandArguments().values["group1option"].operator jsonifier::string() == "") {
					jsonifier::string msgString = "------\n**please, enter a log name to disable or enable as the second argument of this command!";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**missing or invalid arguments:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					input_events::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					if (setToWhat == "enable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(argsNew.getCommandArguments().values["group1option"].operator jsonifier::string()) == discordGuild.data.logs[x].nameSmall) {
								isItFound									 = true;
								channel_data channelNew						 = channels::getCachedChannel({ .channelId = channel.id });
								discordGuild.data.logs[x].loggingChannelId	 = channelNew.id;
								discordGuild.data.logs[x].loggingChannelName = channelNew.name;
								discordGuild.data.logs[x].enabled			 = true;
								discordGuild.writeDataToDB(managerAgent);
								unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
								jsonifier::string msgString = "------\n**nicely done! you've enabled logging for " + jsonifier::string{ discordGuild.data.logs[x].name } +
									jsonifier::string{ ".\nIn channel <#" } + jsonifier::string{ discordGuild.data.logs[x].loggingChannelId.operator jsonifier::string() } +
									">.** \n------";
								msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
								msgEmbed->setColor("fefefe");
								msgEmbed->setDescription(jsonifier::string{ msgString });
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**manage logs enabled:**__");
								respond_to_input_event_data dataPackage(argsNew.getInputEventData());
								dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								input_events::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							jsonifier::string msgString = "please enter a proper log name!";
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setColor("fefefe");
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**manage logs issue:**__");
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							input_events::respondToInputEventAsync(dataPackage).get();
						}
					} else if (setToWhat == "disable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(argsNew.getCommandArguments().values["group1option"].operator jsonifier::string()) ==
								discordGuild.data.logs[x].nameSmall) {
								isItFound									 = true;
								channel_data channelNew						 = channels::getCachedChannel({ .channelId = argsNew.getChannelData().id });
								discordGuild.data.logs[x].loggingChannelId	 = 0;
								discordGuild.data.logs[x].loggingChannelName = "";
								discordGuild.data.logs[x].enabled			 = false;
								discordGuild.writeDataToDB(managerAgent);
								unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
								jsonifier::string msgString =
									jsonifier::string{ "------\n**nicely done! you've disabled logging for " } + discordGuild.data.logs[x].name + ".**\n------";
								msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
								msgEmbed->setColor("fefefe");
								msgEmbed->setDescription(jsonifier::string{ msgString });
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**manage logs disabled:**__");
								respond_to_input_event_data dataPackage(argsNew.getInputEventData());
								dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								input_events::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							jsonifier::string msgString = "please enter a proper log name!";
							unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
							msgEmbed->setColor("fefefe");
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**manage logs issue:**__");
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							input_events::respondToInputEventAsync(dataPackage).get();
						}
					}
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "manage_logs::execute()" << error.what() << std::endl;
			}
		}
		~manage_logs(){};
	};

}// namespace discord_core_api
