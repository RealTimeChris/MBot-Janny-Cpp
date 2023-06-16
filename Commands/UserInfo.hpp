// UserInfo.hpp - Header for the "user info" command.
// Aug 13, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class UserInfo : public BaseFunction {
	  public:
		UserInfo() {
			this->commandName	  = "userinfo";
			this->helpDescription = "Displays some info about a chosen user.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /userinfo, or /userinfo @USERMENTION, to display the info of another user.\n------");
			msgEmbed.setTitle("__**UserData Info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<UserInfo>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };
				Snowflake userId{ std::stoull(argsNew.getCommandArguments().values["user"].value.operator jsonifier::string().data()) };
				GuildMemberData guildMember = GuildMembers::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
				UserData theUser			= Users::getUserAsync({ .userId = argsNew.getUserData().id }).get();
				jsonifier::vector<EmbedFieldData> fields;
				EmbedFieldData field{};
				field.Inline = true;
				field.value	 = jsonifier::string{ guildMember.getUserData().userName } + "#" + jsonifier::string{ theUser.discriminator };
				field.name	 = "__User Tag: __";
				fields.emplace_back(field);
				EmbedFieldData field1{};
				field1.Inline = true;
				field1.value  = guildMember.getUserData().userName;
				field1.name	  = "__User Name:__";
				fields.emplace_back(field1);
				if (guildMember.nick == jsonifier::string{ "" }) {
					EmbedFieldData field2{};
					field2.Inline = true;
					field2.value  = guildMember.getUserData().userName;
					field2.name	  = "__Display Name:__";
					fields.emplace_back(field2);
				} else {
					EmbedFieldData field2{};
					field2.Inline = true;
					field2.value  = guildMember.nick;
					field2.name	  = "__Display Name:__";
					fields.emplace_back(field2);
				}

				EmbedFieldData field3{};
				field3.Inline = true;
				field3.value  = guildMember.user.id.operator jsonifier::string();
				field3.name	  = "__User ID:__";
				fields.emplace_back(field3);
				EmbedFieldData field4{};
				field4.Inline = true;
				field4.value  = static_cast<jsonifier::string>(guildMember.joinedAt);
				field4.name	  = "__Joined:__";
				fields.emplace_back(field4);
				EmbedFieldData field5{};
				field5.Inline = true;
				field5.value  = guildMember.user.id.getCreatedAtTimeStamp();
				field5.name	  = "__Created At:__";
				fields.emplace_back(field5);
				Permissions permsString							= Permissions{ Permissions::getCurrentChannelPermissions(guildMember, channel) };
				jsonifier::vector<jsonifier::string> permissionsArray = permsString.displayPermissions();
				jsonifier::string msgString;
				for (int32_t x = 0; x < permissionsArray.size(); x += 1) {
					msgString += permissionsArray[x];
					if (x < permissionsArray.size() - 1) {
						msgString += ", ";
					}
				}
				EmbedFieldData field6{};
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
				EmbedFieldData field7{};
				field7.Inline = false;
				field7.value  = msgString;
				field7.name	  = "__Permissions:__";
				fields.emplace_back(field7);

				EmbedData msgEmbed;
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**UserData Info:**__");
				msgEmbed.setImage(guildMember.getUserData().getUserImageUrl(UserImageTypes::Avatar) + "?size=4096");
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.fields = fields;
				RespondToInputEventData dataPackage02(argsNew.getInputEventData());
				dataPackage02.addMessageEmbed(msgEmbed);
				dataPackage02.setResponseType(InputEventResponseType::Interaction_Response);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get();

				return;
			} catch (const std::exception& error) {
				std::cout << "UserInfo::execute()" << error.what() << std::endl;
			}
		}
		~UserInfo(){};
	};

}// namespace DiscordCoreAPI
