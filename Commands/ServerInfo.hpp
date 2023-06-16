// ServerInfo.hpp - Header for the "server info" command.
// Aug 3, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class ServerInfo : public BaseFunction {
	  public:
		ServerInfo() {
			this->commandName	  = "serverinfo";
			this->helpDescription = "Displays some info about the current server.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /serverinfo.\n------");
			msgEmbed.setTitle("__**Server Info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<ServerInfo>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				uint32_t categoryCount	   = 0;
				uint32_t voiceChannelCount = 0;
				uint32_t textChannelCount  = 0;
				for (auto& value: guild.channels) {
					ChannelData newChannel = Channels::getCachedChannel({ .channelId = value.id });
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

				jsonifier::vector<EmbedFieldData> fields;
				EmbedFieldData field1{};
				field1.Inline = true;
				field1.value  = guild.name;
				field1.name	  = "__Server Name:__ ";
				fields.emplace_back(field1);
				EmbedFieldData field2{};
				field2.Inline = true;
				field2.value  = guild.id.operator jsonifier::string();
				field2.name	  = "__Server ID:__ ";
				fields.emplace_back(field2);
				EmbedFieldData field3{};
				field3.Inline = true;
				field3.value  = jsonifier::toString(guild.memberCount);
				field3.name	  = "__Server Member Count:__ ";
				fields.emplace_back(field3);
				EmbedFieldData field4{};
				field4.Inline = true;
				field4.value  = "<@" + guild.ownerId + ">";
				field4.name	  = "__Server Owner:__ ";
				fields.emplace_back(field4);
				EmbedFieldData field5{};
				field5.Inline = true;
				field5.value  = guild.ownerId.operator jsonifier::string();
				field5.name	  = "__Server Owner ID:__ ";
				fields.emplace_back(field5);
				EmbedFieldData field6{};
				field6.Inline = true;
				field6.value  = jsonifier::toString(guild.roles.size());
				field6.name	  = "__Role Count:__ ";
				fields.emplace_back(field6);
				EmbedFieldData field7{};
				field7.Inline = true;
				field7.value  = jsonifier::toString(categoryCount);
				field7.name	  = "__Channel Category Count:__ ";
				fields.emplace_back(field7);
				EmbedFieldData field8{};
				field8.Inline = true;
				field8.value  = jsonifier::toString(textChannelCount);
				field8.name	  = "__Text ChannelData Count:__ ";
				fields.emplace_back(field8);
				EmbedFieldData field9{};
				field9.Inline = true;
				field9.value  = jsonifier::toString(voiceChannelCount);
				field9.name	  = "__Voice ChannelData Count:__ ";
				fields.emplace_back(field9);
				EmbedFieldData field10{};
				field10.Inline = true;
				field10.value  = guild.id.getCreatedAtTimeStamp();
				field10.name   = "__Created At:__ ";
				fields.emplace_back(field10);

				EmbedData msgEmbed{};
				msgEmbed.fields = fields;
				msgEmbed.setImage(guild.getGuildImageUrl(GuildImageTypes::Splash));
				msgEmbed.setTitle("__**Server Info:**__");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(discordGuild.data.borderColor);
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				InputEvents::respondToInputEventAsync(dataPackage).get();
				return;
			} catch (const std::exception& error) {
				std::cout << "ServerInfo::execute()" << error.what() << std::endl;
			}
		}
		~ServerInfo(){};
	};

}// namespace DiscordCoreAPI
