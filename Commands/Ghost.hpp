// Ghost.hpp - Header for the "ghost" command.
// jul 31, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class ghost : public base_function {
	  public:
		ghost() {
			this->commandName	  = "ghost";
			this->helpDescription = "'ghosts' or 'unghosts' a server memeber.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /ghost add, reason, @usermention or /ghost remove, @usermention, /ghost view.\n------");
			msgEmbed.setTitle("__**ghost usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<ghost>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data sendingGuildMember = argsNew.getGuildMemberData();

				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, sendingGuildMember, true);
				auto argsNewer				 = argsNew.getInputEventData();
				if (!doWeHaveAdminPermission) {
					return;
				}
				jsonifier::string whatAreWeDoing{};
				jsonifier::string ghostReason{};
				snowflake userId{};
				if (argsNew.getSubCommandName() == "view") {
					whatAreWeDoing = "viewing";
					userId		   = sendingGuildMember.user.id;
				}
				if (argsNew.getCommandArguments().values.size() > 0 && argsNew.getSubCommandName() == "add") {
					whatAreWeDoing		  = "add";
					size_t argOne				  = argsNew.getCommandArguments().values["user"].operator size_t();
					jsonifier::string argTwo	  = argsNew.getCommandArguments().values["reason"].operator jsonifier::string();
					ghostReason			  = argTwo;
					userId						  = argOne;
				} else if (argsNew.getCommandArguments().values.size() > 0 && argsNew.getSubCommandName() == "remove") {
					whatAreWeDoing		  = "remove";
					jsonifier::string argOne	  = argsNew.getCommandArguments().values["user"].operator jsonifier::string();
					jsonifier::string userIDOne = argOne;
					userId						  = jsonifier::strToUint64(userIDOne);
				}
				input_event_data newEvent01 = argsNewer;

				guild_member_data targetGuildMember = guild_members::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
				discord_guild_member discordGuildMember(managerAgent, targetGuildMember);
				if (whatAreWeDoing == "add") {
					timeout_guild_member_data modifyData{};
					modifyData.numOfMinutesToTimeoutFor = timeout_durations::Week;
					modifyData.guildId					= guild.id;
					modifyData.guildMemberId			= targetGuildMember.user.id;
					modifyData.reason					= ghostReason;
					targetGuildMember					= guild_members::timeoutGuildMemberAsync(modifyData).get();

					jsonifier::string msgString = "------\n**hello! you've been redacted, on the server " + jsonifier::string{ guild.name } + " for the following reason(s): " + ghostReason +
						"\n please, contact a moderator or admin to clear this issue up! thanks!**\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(discord_core_client::getInstance()->getBotUser().userName, discord_core_client::getInstance()->getBotUser().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**you\'ve been ghosted:**__");

					auto dm_channel = channels::createDMChannelAsync({ .userId = userId }).get();
					create_message_data dataPackage{ dm_channel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					messages::createMessageAsync(dataPackage).get();

					if (targetGuildMember.user.id == 0) {
						jsonifier::string msgStringNew = "------\n**hello! there was an error while trying to ghost <@" + userId + ">**\n------\n";
						unique_ptr<embed_data> msgEmbedNew{ makeUnique<embed_data>() };
						msgEmbedNew->setAuthor(discord_core_client::getInstance()->getBotUser().userName, discord_core_client::getInstance()->getBotUser().getUserImageUrl<user_image_types::Avatar>());
						msgEmbedNew->setColor("fefefe");
						msgEmbedNew->setDescription(msgStringNew);
						msgEmbedNew->setTimeStamp(getTimeAndDate());
						msgEmbedNew->setTitle("__**ghosting error:**__");
						respond_to_input_event_data dataPackage01(newEvent01);
						dataPackage01.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage01.addMessageEmbed(*msgEmbedNew);
						auto eventNew = input_events::respondToInputEventAsync(dataPackage01).get();
						return;
					}

					discordGuild.data.ghostedIds.emplace_back(targetGuildMember.user.id);
					discordGuild.writeDataToDB(managerAgent);

					jsonifier::string msgString2 = "------\n**hello! you've ghosted the following member:** <@" + targetGuildMember.user.id + "> (" +
						jsonifier::string{ targetGuildMember.getUserData().userName } + ")\n------";
					embed_data msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.getUserData().userName, sendingGuildMember.getGuildMemberImageUrl<guild_member_image_types::Avatar>());
					msgEmbed2.setColor("fefefe");
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**new server member ghosted:**__");
					respond_to_input_event_data dataPackage02(newEvent01);
					dataPackage02.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = input_events::respondToInputEventAsync(dataPackage02).get();
				} else if (whatAreWeDoing == "viewing") {
					jsonifier::string msgString = "------\n";

					for (auto& value: discordGuild.data.ghostedIds) {
						msgString += "<@" + value + ">\n";
					}

					msgString += "------";

					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**currently ghosted members:**__");
					respond_to_input_event_data dataPackage01(newEvent01);
					dataPackage01.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage01.addMessageEmbed(*msgEmbed);
					auto eventNew = input_events::respondToInputEventAsync(dataPackage01).get();
					return;
				} else if (whatAreWeDoing == "remove") {
					bool isItThere{ false };
					int32_t index{ 0 };
					for (uint32_t x = 0; x < discordGuild.data.ghostedIds.size(); x += 1) {
						if (discordGuild.data.ghostedIds[x] == targetGuildMember.user.id) {
							isItThere = true;
							index	  = x;
							break;
						}
					}
					timeout_guild_member_data modifyData{};
					modifyData.numOfMinutesToTimeoutFor = timeout_durations::None;
					modifyData.guildId					= guild.id;
					modifyData.reason					= ghostReason;
					modifyData.guildMemberId			= targetGuildMember.user.id;
					targetGuildMember					= guild_members::timeoutGuildMemberAsync(modifyData).get();

					if (targetGuildMember.user.id == 0 || !isItThere) {
						jsonifier::string msgString = "------\n**hello! there was an error while trying to un-ghost <@" + userId + ">**\n------\n";
						unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
						msgEmbed->setAuthor(discord_core_client::getInstance()->getBotUser().userName, discord_core_client::getInstance()->getBotUser().getUserImageUrl<user_image_types::Avatar>());
						msgEmbed->setColor("fefefe");
						msgEmbed->setDescription(msgString);
						msgEmbed->setTimeStamp(getTimeAndDate());
						msgEmbed->setTitle("__**un-ghosting error:**__");
						respond_to_input_event_data dataPackage(newEvent01);
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(*msgEmbed);
						auto eventNew = input_events::respondToInputEventAsync(dataPackage).get();
						return;
					}

					discordGuild.data.ghostedIds.erase(discordGuild.data.ghostedIds.begin() + index);
					discordGuild.writeDataToDB(managerAgent);
					jsonifier::string msgString = "------\n**hello! you\'ve had your redacted status removed! have a great day!**\n------";
					unique_ptr<embed_data> msgEmbed{ makeUnique<embed_data>() };
					msgEmbed->setAuthor(discord_core_client::getInstance()->getBotUser().userName, discord_core_client::getInstance()->getBotUser().getUserImageUrl<user_image_types::Avatar>());
					msgEmbed->setColor("fefefe");
					msgEmbed->setDescription(msgString);
					msgEmbed->setTimeStamp(getTimeAndDate());
					msgEmbed->setTitle("__**you\'ve been un-ghosted:**__");

					auto dm_channel = channels::createDMChannelAsync({ .userId = userId }).get();
					create_message_data dataPackage{ dm_channel.id };
					dataPackage.addMessageEmbed(*msgEmbed);
					messages::createMessageAsync(dataPackage).get();

					jsonifier::string msgString2 = "------\n**hello! you've un-ghosted the following member:** <@" + targetGuildMember.user.id + "> (" +
						jsonifier::string{ targetGuildMember.getUserData().userName } + ")\n------";
					embed_data msgEmbed2;
					msgEmbed2.setAuthor(sendingGuildMember.getUserData().userName, sendingGuildMember.getGuildMemberImageUrl<guild_member_image_types::Avatar>());
					msgEmbed2.setColor("fefefe");
					msgEmbed2.setDescription(msgString2);
					msgEmbed2.setTimeStamp(getTimeAndDate());
					msgEmbed2.setTitle("__**new server member un-ghosted:**__");
					respond_to_input_event_data dataPackage02(newEvent01);
					dataPackage02.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed2);
					auto eventNew = input_events::respondToInputEventAsync(dataPackage02).get();
					return;
				}
				return;
			} catch (const std::exception& error) {
				std::cout << "ghost::execute()" << error.what() << std::endl;
			}
		}
		~ghost(){};
	};
}// namespace discord_core_api
