// ManageLogs.hpp - Header for the "manage logs" command.
// Aug 10, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class ManageLogs : public BaseFunction {
	  public:
		ManageLogs() {
			this->commandName	  = "managelogs";
			this->helpDescription = "Views the status for the given possible logs.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /managelogs, to view an enabled/disabled list of possible logs.\n------");
			msgEmbed.setTitle("__**Manage Logs Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<ManageLogs>();
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
				jsonifier::string setToWhat{};
				if (argsNew.getCommandArguments().values.size() > 0) {
					if (static_cast<bool>(std::stoull(argsNew.getCommandArguments().values["settowhat"].value)) == true) {
						setToWhat = "enable";
					} else if (static_cast<bool>(std::stoull(argsNew.getCommandArguments().values["settowhat"].value)) == false) {
						setToWhat = "disable";
					}
				}

				if (argsNew.getCommandArguments().values.size() == 0) {
					jsonifier::vector<EmbedFieldData> fields;
					for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
						if (Channels::getCachedChannel({ .channelId = discordGuild.data.logs[x].loggingChannelId }).name == "") {
							discordGuild.data.logs[x].loggingChannelName = "";
							discordGuild.data.logs[x].loggingChannelId	 = 0;
							discordGuild.data.logs[x].enabled			 = false;
						}
						if (discordGuild.data.logs[x].enabled == false) {
							EmbedFieldData field{};
							field.Inline = true;
							field.value	 = "__Enabled:__ ❌";
							field.name	 = "__**" + discordGuild.data.logs[x].name + "**__";
							fields.emplace_back(field);
						} else if (discordGuild.data.logs[x].enabled == true) {
							EmbedFieldData field{};
							field.Inline = true;
							field.value	 = "__Enabled:__ ✅\n__Logging ChannelData:__ <#" + discordGuild.data.logs[x].loggingChannelId + ">";
							field.name	 = "__**" + discordGuild.data.logs[x].name + "**__";
							fields.emplace_back(field);
						}
					}

					jsonifier::string msgString = "**To enable/disable a given log, enter within the text channel where you would like it to be logged: !managelogs "
											"= <enable/disable>, <logname>\nFor example, '!managelogs = "
											"enable, guildbanadd'.**'";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					*msgEmbed = EmbedData()
									.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar))
									.setColor(discordGuild.data.borderColor)
									.setDescription(msgString)
									.setTimeStamp(getTimeAndDate())
									.setTitle("__**Manage Logs:**__");
					for (auto& value: fields) {
						msgEmbed->addField(value.name, value.value, value.Inline);
					}
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				else if (setToWhat != "enable" && setToWhat != "disable") {
					jsonifier::string msgString = "------\n**Please, enter enable or disable for the first argument of this command! (!managelogs = "
											"<enable/disable>, <logname>)**\n------";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else if (argsNew.getCommandArguments().values["group1option"].value.operator jsonifier::string() == "") {
					jsonifier::string msgString = "------\n**Please, enter a log name to disable or enable as the second argument of this command!";
					UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed->setColor(discordGuild.data.borderColor);
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					if (setToWhat == "enable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(argsNew.getCommandArguments().values["group1option"].value) == discordGuild.data.logs[x].nameSmall) {
								isItFound									 = true;
								ChannelData channelNew						 = Channels::getCachedChannel({ .channelId = channel.id });
								discordGuild.data.logs[x].loggingChannelId	 = channelNew.id;
								discordGuild.data.logs[x].loggingChannelName = channelNew.name;
								discordGuild.data.logs[x].enabled			 = true;
								discordGuild.writeDataToDB(managerAgent);
								UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
								jsonifier::string msgString = "------\n**Nicely done! You've enabled logging for " + discordGuild.data.logs[x].name + ".\nIn channel <#" +
									discordGuild.data.logs[x].loggingChannelId + ">.** \n------";
								msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
								msgEmbed->setColor(discordGuild.data.borderColor);
								msgEmbed->setDescription(msgString);
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**Manage Logs Enabled:**__");
								RespondToInputEventData dataPackage(argsNew.getInputEventData());
								dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								InputEvents::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							jsonifier::string msgString = "Please enter a proper log name!";
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Manage Logs Issue:**__");
							RespondToInputEventData dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							InputEvents::respondToInputEventAsync(dataPackage).get();
						}
					} else if (setToWhat == "disable") {
						bool isItFound{ false };
						for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
							if (convertToLowerCase(argsNew.getCommandArguments().values["group1option"].value) == discordGuild.data.logs[x].nameSmall) {
								isItFound									 = true;
								ChannelData channelNew						 = Channels::getCachedChannel({ .channelId = argsNew.getChannelData().id });
								discordGuild.data.logs[x].loggingChannelId	 = 0;
								discordGuild.data.logs[x].loggingChannelName = "";
								discordGuild.data.logs[x].enabled			 = false;
								discordGuild.writeDataToDB(managerAgent);
								UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
								jsonifier::string msgString = "------\n**Nicely done! You've disabled logging for " + discordGuild.data.logs[x].name + ".**\n------";
								msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
								msgEmbed->setColor(discordGuild.data.borderColor);
								msgEmbed->setDescription(msgString);
								msgEmbed->setTimeStamp(getTimeAndDate());
								msgEmbed->setTitle("__**Manage Logs Disabled:**__");
								RespondToInputEventData dataPackage(argsNew.getInputEventData());
								dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
								dataPackage.addMessageEmbed(*msgEmbed);
								InputEvents::respondToInputEventAsync(dataPackage).get();
								break;
							}
						}
						if (!isItFound) {
							jsonifier::string msgString = "Please enter a proper log name!";
							UniquePtr<EmbedData> msgEmbed{ makeUnique<EmbedData>() };
							msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							msgEmbed->setColor(discordGuild.data.borderColor);
							msgEmbed->setDescription(msgString);
							msgEmbed->setTimeStamp(getTimeAndDate());
							msgEmbed->setTitle("__**Manage Logs Issue:**__");
							RespondToInputEventData dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(*msgEmbed);
							InputEvents::respondToInputEventAsync(dataPackage).get();
						}
					}
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "ManageLogs::execute()" << error.what() << std::endl;
			}
		}
		~ManageLogs(){};
	};

}// namespace DiscordCoreAPI
