// ServerInfo.hpp - Header for the "server info" command.
// aug 3, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class server_info : public base_function {
	  public:
		server_info() {
			this->commandName	  = "serverinfo";
			this->helpDescription = "displays some info about the current server.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /serverinfo.\n------");
			msgEmbed.setTitle("__**server info usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<server_info>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ to_entity<guild_data>{}.toEntity(argsNew.getInteractionData().guildId) };
				discord_guild discordGuild{ managerAgent, guild };

				uint32_t categoryCount	   = 0;
				uint32_t voiceChannelCount = 0;
				uint32_t textChannelCount  = 0;
				for (auto& value: guild.channels) {
					channel_data newChannel = channels::getCachedChannel({ .channelId = value.id });
					if (newChannel.type == channel_type::Guild_Voice) {
						voiceChannelCount += 1;
					}
					if (newChannel.type == channel_type::Guild_Text) {
						textChannelCount += 1;
					}
					if (newChannel.type == channel_type::Guild_Category) {
						categoryCount += 1;
					}
				}

				jsonifier::vector<embed_field_data> fields;
				embed_field_data field1{};
				field1.Inline = true;
				field1.value  = guild.name;
				field1.name	  = "__Server name:__ ";
				fields.emplace_back(field1);
				embed_field_data field2{};
				field2.Inline = true;
				field2.value  = guild.id.operator jsonifier::string();
				field2.name	  = "__Server id:__ ";
				fields.emplace_back(field2);
				embed_field_data field3{};
				field3.Inline = true;
				field3.value  = jsonifier::toString(guild.memberCount);
				field3.name	  = "__Server member count:__ ";
				fields.emplace_back(field3);
				embed_field_data field4{};
				field4.Inline = true;
				field4.value  = "<@" + guild.ownerId + ">";
				field4.name	  = "__Server owner:__ ";
				fields.emplace_back(field4);
				embed_field_data field5{};
				field5.Inline = true;
				field5.value  = guild.ownerId.operator jsonifier::string();
				field5.name	  = "__Server owner id:__ ";
				fields.emplace_back(field5);
				embed_field_data field6{};
				field6.Inline = true;
				field6.value  = jsonifier::toString(guild.roles.size());
				field6.name	  = "__Role count:__ ";
				fields.emplace_back(field6);
				embed_field_data field7{};
				field7.Inline = true;
				field7.value  = jsonifier::toString(categoryCount);
				field7.name	  = "__Channel category count:__ ";
				fields.emplace_back(field7);
				embed_field_data field8{};
				field8.Inline = true;
				field8.value  = jsonifier::toString(textChannelCount);
				field8.name	  = "__Text channel_data count:__ ";
				fields.emplace_back(field8);
				embed_field_data field9{};
				field9.Inline = true;
				field9.value  = jsonifier::toString(voiceChannelCount);
				field9.name	  = "__Voice channel_data count:__ ";
				fields.emplace_back(field9);
				embed_field_data field10{};
				field10.Inline = true;
				field10.value  = guild.id.getCreatedAtTimeStamp();
				field10.name   = "__Created at:__ ";
				fields.emplace_back(field10);

				embed_data msgEmbed{};
				msgEmbed.fields = fields;
				std::cout << "CURRENT SPLASH URL: " << guild.getGuildImageUrl(guild_image_types::Splash) << std::endl;
				msgEmbed.setImage(guild.getGuildImageUrl(guild_image_types::Splash));
				msgEmbed.setTitle("__**server info:**__");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				input_events::respondToInputEventAsync(dataPackage).get();
				return;
			} catch (const std::exception& error) {
				std::cout << "server_info::execute()" << error.what() << std::endl;
			}
		}
		~server_info(){};
	};

}// namespace discord_core_api
