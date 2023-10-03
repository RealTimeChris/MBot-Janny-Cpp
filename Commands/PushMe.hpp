// PushMe.hpp - Header for the "PushMe" command.
// jul 31, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class push_me : public base_function {
	  public:
		push_me() {
			this->commandName	  = "pushme";
			this->helpDescription = "push me to find out!";
			embed_data msgEmbed{};
			msgEmbed.setDescription("enter /pushme");
			msgEmbed.setTitle("__**push_me usage**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<push_me>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };

				guild_member_data guildMember = guild_members::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });

				jsonifier::string msgString = "------\n__**push me!**__\n------";
				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**push_me:**__");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.addButton(false, "push_me", "press-me", button_style::Success, "✅");
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = input_events::respondToInputEventAsync(dataPackage).get();

				button_collector buttonCollector{ eventNew };
				auto createResponseData = makeUnique<create_interaction_response_data>();
				auto embedData			= makeUnique<embed_data>();
				embedData->setColor("fefefe");
				embedData->setTitle("__**permissions issue:**__");
				embedData->setTimeStamp(getTimeAndDate());
				embedData->setDescription("sorry, but that button can only be pressed by <@" + argsNew.getUserData().id + ">!");
				createResponseData->addMessageEmbed(*embedData);
				createResponseData->setResponseType(interaction_callback_type::Channel_Message_With_Source);
				createResponseData->setFlags(64);
				auto resultData = buttonCollector.collectButtonData(false, 120000, 1, *createResponseData, argsNew.getUserData().id).get();
				if (resultData[0].buttonId == "push_me") {
					timeout_guild_member_data dataPackage02{};
					dataPackage02.guildId				   = guild.id;
					dataPackage02.guildMemberId			   = argsNew.getUserData().id;
					dataPackage02.numOfMinutesToTimeoutFor = timeout_durations::Five_Minutes;
					dataPackage02.reason				   = "pressed the buttton!";
					guild_members::timeoutGuildMemberAsync(dataPackage02).get();
				}

				respond_to_input_event_data dataPackage03{ *resultData[0].interactionData };
				dataPackage03.addMessageEmbed(msgEmbed);
				dataPackage03.setResponseType(input_event_response_type::Edit_Interaction_Response);
				input_events::respondToInputEventAsync(dataPackage03).get();

				return;

			} catch (const std::exception& error) {
				std::cout << "push_me::execute()" << error.what() << std::endl;
			}
		}
		~push_me(){};
	};

}// namespace discord_core_api