// UserInfo.hpp - Header for the "user info" command.
// aug 13, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class user_info : public base_function {
	  public:
		user_info() {
			this->commandName	  = "userinfo";
			this->helpDescription = "displays some info about a chosen user.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /userinfo, or /userinfo @usermention, to display the info of another user.\n------");
			msgEmbed.setTitle("__**user_data info usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<user_info>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ managerAgent, guild };
				snowflake userId{ jsonifier::strToUint64(argsNew.getCommandArguments().values["user"].operator jsonifier::string()) };
				guild_member_data guildMember = guild_members::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
				user_data theUser			= users::getUserAsync({ .userId = argsNew.getUserData().id }).get();
				jsonifier::vector<embed_field_data> fields;
				embed_field_data field{};
				field.Inline = true;
				field.value	 = jsonifier::string{ guildMember.getUserData().userName } + "#" + jsonifier::string{ theUser.discriminator };
				field.name	 = "__User tag: __";
				fields.emplace_back(field);
				embed_field_data field1{};
				field1.Inline = true;
				field1.value  = guildMember.getUserData().userName;
				field1.name	  = "__User name:__";
				fields.emplace_back(field1);
				if (guildMember.nick == jsonifier::string{ "" }) {
					embed_field_data field2{};
					field2.Inline = true;
					field2.value  = guildMember.getUserData().userName;
					field2.name	  = "__Display name:__";
					fields.emplace_back(field2);
				} else {
					embed_field_data field2{};
					field2.Inline = true;
					field2.value  = guildMember.nick;
					field2.name	  = "__Display name:__";
					fields.emplace_back(field2);
				}

				embed_field_data field3{};
				field3.Inline = true;
				field3.value  = guildMember.user.id.operator jsonifier::string();
				field3.name	  = "__User id:__";
				fields.emplace_back(field3);
				embed_field_data field4{};
				field4.Inline = true;
				field4.value  = static_cast<jsonifier::string>(guildMember.joinedAt);
				field4.name	  = "__Joined:__";
				fields.emplace_back(field4);
				embed_field_data field5{};
				field5.Inline = true;
				field5.value  = guildMember.user.id.getCreatedAtTimeStamp();
				field5.name	  = "__Created at:__";
				fields.emplace_back(field5);
				permissions permsString							= permissions{ permissions::getCurrentChannelPermissions(guildMember, channel) };
				jsonifier::vector<jsonifier::string> permissionsArray = permsString.displayPermissions();
				jsonifier::string msgString;
				for (int32_t x = 0; x < permissionsArray.size(); x += 1) {
					msgString += permissionsArray[x];
					if (x < permissionsArray.size() - 1) {
						msgString += ", ";
					}
				}
				embed_field_data field6{};
				field6.Inline = false;
				field6.value  = "";
				field6.name	  = "__Roles:__";

				for (uint32_t x = 0; x < guildMember.roles.size(); x += 1) {
					field6.value += "<@&" + guildMember.roles[x] + ">";
					if (x < guildMember.roles.size() - 1) {
						field6.value += ", ";
					}
				}
				fields.emplace_back(field6);
				embed_field_data field7{};
				field7.Inline = false;
				field7.value  = msgString;
				field7.name	  = "__Permissions:__";
				fields.emplace_back(field7);

				embed_data msgEmbed;
				msgEmbed.setColor("fefefe");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**user_data info:**__");
				msgEmbed.setImage(guildMember.getUserData().getUserImageUrl<user_image_types::Avatar>() + "?size=4096");
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl<user_image_types::Avatar>());
				msgEmbed.fields = fields;
				respond_to_input_event_data dataPackage02(argsNew.getInputEventData());
				dataPackage02.addMessageEmbed(msgEmbed);
				dataPackage02.setResponseType(input_event_response_type::Interaction_Response);
				auto eventNew = input_events::respondToInputEventAsync(dataPackage02).get();

				return;
			} catch (const std::exception& error) {
				std::cout << "user_info::execute()" << error.what() << std::endl;
			}
		}
		~user_info(){};
	};

}// namespace discord_core_api
