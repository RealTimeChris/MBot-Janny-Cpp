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
				EmbedFieldData field1{};
				field1.Inline = true;
				field1.value = guild.name;
				field1.name = "__Server Name:__ ";
				fields.push_back(field1);
				EmbedFieldData field2{};
				field2.Inline = true;
				field2.value = std::to_string(guild.id);
				field2.name = "__Server ID:__ ";
				fields.push_back(field2);
				EmbedFieldData field3{};
				field3.Inline = true;
				field3.value = std::to_string(guild.memberCount);
				field3.name = "__Server Member Count:__ ";
				fields.push_back(field3);
				EmbedFieldData field4{};
				field4.Inline = true;
				field4.value = "<@" + std::to_string(guild.ownerId) + ">";
				field4.name = "__Server Owner:__ ";
				fields.push_back(field4);
				EmbedFieldData field5{};
				field5.Inline = true;
				field5.value = std::to_string(guild.ownerId);
				field5.name = "__Server Owner ID:__ ";
				fields.push_back(field5);
				EmbedFieldData field6{};
				field6.Inline = true;
				field6.value = std::to_string(guild.roles.size());
				field6.name = "__Role Count:__ ";
				fields.push_back(field6);
				EmbedFieldData field7{};
				field7.Inline = true;
				field7.value = std::to_string(categoryCount);
				field7.name = "__Channel Category Count:__ ";
				fields.push_back(field7);
				EmbedFieldData field8{};
				field8.Inline = true;
				field8.value = std::to_string(textChannelCount);
				field8.name = "__Text Channel Count:__ ";
				fields.push_back(field8);
				EmbedFieldData field9{};
				field9.Inline = true;
				field9.value = std::to_string(voiceChannelCount);
				field9.name = "__Voice Channel Count:__ ";
				fields.push_back(field9);
				EmbedFieldData field10{};
				field10.Inline = true;
				field10.value = guild.getCreatedAtTimestamp(TimeFormat::LongDateTime);
				field10.name = "__Created At:__ ";
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
