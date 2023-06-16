// PushMe.hpp - Header for the "PushMe" command.
// Jul 31, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class PushMe : public BaseFunction {
	  public:
		PushMe() {
			this->commandName	  = "pushme";
			this->helpDescription = "Push me to find out!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("Enter /pushme");
			msgEmbed.setTitle("__**PushMe Usage**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<PushMe>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember = GuildMembers::getCachedGuildMember({ .guildMemberId = argsNew.getUserData().id, .guildId = guild.id });

				jsonifier::string msgString = "------\n__**Push Me!**__\n------";
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setDescription(msgString);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**PushMe:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.addButton(false, "push_me", "Press-Me", ButtonStyle::Success, "✅");
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage).get();

				ButtonCollector buttonCollector{ eventNew };
				auto createResponseData = makeUnique<CreateInteractionResponseData>();
				auto embedData			= makeUnique<EmbedData>();
				embedData->setColor("FEFEFE");
				embedData->setTitle("__**Permissions Issue:**__");
				embedData->setTimeStamp(getTimeAndDate());
				embedData->setDescription("Sorry, but that button can only be pressed by <@" + argsNew.getUserData().id + ">!");
				createResponseData->addMessageEmbed(*embedData);
				createResponseData->setResponseType(InteractionCallbackType::Channel_Message_With_Source);
				createResponseData->setFlags(64);
				auto resultData = buttonCollector.collectButtonData(false, 120000, 1, *createResponseData, argsNew.getUserData().id).get();
				if (resultData[0].buttonId == "push_me") {
					TimeoutGuildMemberData dataPackage02{};
					dataPackage02.guildId				   = guild.id;
					dataPackage02.guildMemberId			   = argsNew.getUserData().id;
					dataPackage02.numOfMinutesToTimeoutFor = TimeoutDurations::Five_Minutes;
					dataPackage02.reason				   = "PRESSED THE BUTTTON!";
					GuildMembers::timeoutGuildMemberAsync(dataPackage02).get();
				}

				RespondToInputEventData dataPackage03{ *resultData[0].interactionData };
				dataPackage03.addMessageEmbed(msgEmbed);
				dataPackage03.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				InputEvents::respondToInputEventAsync(dataPackage03).get();

				return;

			} catch (const std::exception& error) {
				std::cout << "PushMe::execute()" << error.what() << std::endl;
			}
		}
		~PushMe(){};
	};

}// namespace DiscordCoreAPI