// StreamAudio.hpp - Header for the "disconnect" std::function.
// Aug 25, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class StreamAudio : public BaseFunction {
	  public:
		StreamAudio() {
			this->commandName	  = "streamaudio";
			this->helpDescription = "Stream Audio from one bot to another.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /streamaudio!\n------");
			msgEmbed.setTitle("__**StreamAudio Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		DiscordCoreAPI::UniquePtr<BaseFunction> create() {
			return DiscordCoreAPI::makeUnique<StreamAudio>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };


				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberData guildMember{ argsNew.getGuildMemberData() };

				Snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					DiscordCoreAPI::UniquePtr<EmbedData> newEmbed{ DiscordCoreAPI::makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				StreamInfo theInfo{};
				theInfo.address = argsNew.getCommandArguments().values["connectionip"].value;
				theInfo.port	= 51072;
				theInfo.type	= StreamType::Client;
				if (argsNew.getCommandArguments().values["botaudio"].value == "1") {
					theInfo.streamBotAudio = true;
				} else {
					theInfo.streamBotAudio = false;
				}
				VoiceConnection& voiceConnection = guild.connectToVoice(guildMember.user.id, 0, false, false, theInfo);
				if (!voiceConnection.areWeConnected()) {
					DiscordCoreAPI::UniquePtr<EmbedData> newEmbed{ DiscordCoreAPI::makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Connection Issue:**__");
					newEmbed->setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					DiscordCoreAPI::UniquePtr<EmbedData> newEmbed{ DiscordCoreAPI::makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				std::this_thread::sleep_for(1000ms);
				DiscordCoreClient::getSongAPI(guild.id).play();

				DiscordCoreAPI::UniquePtr<EmbedData> newEmbed{ DiscordCoreAPI::makeUnique<EmbedData>() };
				newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				newEmbed->setDescription("------\n__**Congratulations - you've been connected to an audio channel to stream audio!**__\n------");
				newEmbed->setTimeStamp(getTimeAndDate());
				newEmbed->setTitle("__**Streaming Audio:**__");
				newEmbed->setColor(discordGuild.data.borderColor);
				RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
				dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage02.addMessageEmbed(*newEmbed);
				InputEvents::respondToInputEventAsync(dataPackage02).get();
				return;
			} catch (const std::exception& error) {
				std::cout << "StreamAudio::execute()" << error.what() << std::endl;
			}
		}
		~StreamAudio(){};
	};

}// namespace DiscordCoreAPI
