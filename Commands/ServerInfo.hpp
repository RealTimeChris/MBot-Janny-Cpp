// ServerInfo.hpp - Header for the "server info" command.
// Aug 3, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class ServerInfo : public BaseFunction {
	  public:
		ServerInfo() {
			this->commandName = "serverinfo";
			this->helpDescription = "Displays some info about the current server.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /serverinfo.\n------");
			msgEmbed.setTitle("__**Server Info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<ServerInfo>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				uint32_t categoryCount = 0;
				uint32_t voiceChannelCount = 0;
				uint32_t textChannelCount = 0;
				for (auto value: guild.channels) {
					Channel newChannel = Channels::getCachedChannelAsync({ .channelId = value }).get();
					if (newChannel.type == ChannelType::Guild_Voice) {
						voiceChannelCount += 1;
					}
					if (newChannel.type == ChannelType::Guild_Text) {
						textChannelCount += 1;
					}
					if (newChannel.type == ChannelType::Guild_Category) {
						categoryCount += 1;
					}
				}

				std::vector<EmbedFieldData> fields;
				EmbedFieldData field1 = { .Inline = true, .value = guild.name, .name = "__Server Name:__ " };
				fields.push_back(field1);
				EmbedFieldData field2 = { .Inline = true, .value = std::to_string(guild.id), .name = "__Server ID:__ " };
				fields.push_back(field2);
				EmbedFieldData field3 = { .Inline = true, .value = std::to_string(guild.memberCount), .name = "__Server Member Count:__ " };
				fields.push_back(field3);
				EmbedFieldData field4 = { .Inline = true, .value = "<@" + guild.ownerId + ">", .name = "__Server Owner:__ " };
				fields.push_back(field4);
				EmbedFieldData field5 = { .Inline = true, .value = guild.ownerId, .name = "__Server Owner ID:__ " };
				fields.push_back(field5);
				EmbedFieldData field6 = { .Inline = true, .value = std::to_string(guild.roles.size()), .name = "__Role Count:__ " };
				fields.push_back(field6);
				EmbedFieldData field7 = { .Inline = true, .value = std::to_string(categoryCount), .name = "__Channel Category Count:__ " };
				fields.push_back(field7);
				EmbedFieldData field8 = { .Inline = true, .value = std::to_string(textChannelCount), .name = "__Text Channel Count:__ " };
				fields.push_back(field8);
				EmbedFieldData field9 = { .Inline = true, .value = std::to_string(voiceChannelCount), .name = "__Voice Channel Count:__ " };
				fields.push_back(field9);
				EmbedFieldData field10 = { .Inline = true, .value = guild.getCreatedAtTimestamp(TimeFormat::LongDateTime), .name = "__Created At:__ " };
				fields.push_back(field10);

				EmbedData msgEmbed{};
				msgEmbed.fields = fields;
				msgEmbed.setImage(guild.icon);
				msgEmbed.setTitle("__**Server Info:**__");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor(discordGuild.data.borderColor);
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				InputEvents::respondToInputEventAsync(dataPackage).get();
				return;
			} catch (...) {
				reportException("ServerInfo::execute()");
			}
		}
		~ServerInfo(){};
	};

}
