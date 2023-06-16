// Disconnect.hpp - Header for the "disconnect" std::function.
// Aug 25, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Disconnect : public BaseFunction {
	  public:
		Disconnect() {
			this->commandName	  = "disconnect";
			this->helpDescription = "Disconnect the bot from voice chat.";
			EmbedData msgEmbed;
			msgEmbed.setDescription("------\nSimply enter /disconnect!\n------");
			msgEmbed.setTitle("__**Disconnect Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Disconnect>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };


				if (guild.areWeConnected()) {
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setDescription("------\n__**I'm disconnecting from the voice channel!**__\n------");
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Disconnected:**__");
					msgEmbed.setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(argsNewer, 20000);
					guild.disconnect();
				} else {
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setDescription("------\n__**I was already disconnected!**__\n------");
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Disconnected:**__");
					msgEmbed.setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
				}

				return;
			} catch (const std::exception& error) {
				std::cout << "Disconnect::execute()" << error.what() << std::endl;
			}
		}
		~Disconnect(){};
	};

}// namespace DiscordCoreAPI
