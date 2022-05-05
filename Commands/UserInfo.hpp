// UserInfo.hpp - Header for the "user info" command.
// Aug 13, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class UserInfo : public BaseFunction {
	  public:
		UserInfo() {
			this->commandName = "userinfo";
			this->helpDescription = "Displays some info about a chosen user.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription(
				"------\nEnter /userinfo to display your own info!\nOr /userinfo @USERMENTION, to display the info of another user.\n------");
			msgEmbed.setTitle("__**User Info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<UserInfo>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ .channelId = newArgs.eventData.getChannelId() }).get();

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				Guild guild = Guilds::getCachedGuildAsync({ newArgs.eventData.getGuildId() }).get();

				DiscordGuild discordGuild{ guild };
				std::string userID;
				std::regex userIdRegexp("\\d{18}");
				if (newArgs.commandData.optionsArgs.size() == 0) {
					userID = newArgs.eventData.getAuthorId();
				} else if (std::regex_search(newArgs.commandData.optionsArgs[0], userIdRegexp)) {
					std::cmatch userIdMatch{};
					std::regex_search(newArgs.commandData.optionsArgs[0].c_str(), userIdMatch, userIdRegexp);
					userID = userIdMatch.str();
				}

				User user = Users::getUserAsync({ .userId = userID }).get();
				if (user.id == "") {
					std::string msgString = "------\n**Please enter a valid user ID or user mention! (!userinfo = @USERMENTION)**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(newArgs.eventData);
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto eventNew = InputEvents::respondToEventAsync(dataPackage).get();
					return;
				}
				GuildMember guildMember = GuildMembers::getGuildMemberAsync({ .guildMemberId = userID, .guildId = newArgs.eventData.getGuildId() }).get();
				std::vector<EmbedFieldData> fields;
				EmbedData msgEmbed{};
				if (guildMember.joinedAt.getOriginalTimeStamp() != "") {
					EmbedFieldData field{ .value = guildMember.user.userName + "#" + guildMember.user.discriminator, .name = "__User Tag: __", .Inline = true };
					fields.push_back(field);
					EmbedFieldData field1{ .value = guildMember.user.userName, .name = "__User Name:__", .Inline = true };
					fields.push_back(field1);
					if (guildMember.nick == "") {
						EmbedFieldData field2{ .value = guildMember.user.userName, .name = "__Display Name:__", .Inline = true };
						fields.push_back(field2);
					} else {
						EmbedFieldData field2 = { .value = guildMember.nick, .name = "__Display Name:__", .Inline = true };
						fields.push_back(field2);
					}
					EmbedFieldData field3{
						.value = guildMember.user.id,
						.name = "__User ID:__",
						.Inline = true,
					};
					fields.push_back(field3);
					EmbedFieldData field4{ .value = guildMember.joinedAt.getDateTimeStamp(TimeFormat::LongDateTime), .name = "__Joined:__", .Inline = true };
					fields.push_back(field4);
					EmbedFieldData field5{ .value = guildMember.user.createdAt, .name = "__Created At:__", .Inline = true };
					fields.push_back(field5);
					Permissions permsString{ Permissions::getCurrentGuildPermissions(guildMember) };
					std::vector<std::string> permissionsArray{ permsString.displayPermissions() };
					std::string msgString{};
					for (int32_t x = 0; x < permissionsArray.size(); x += 1) {
						msgString += permissionsArray[x];
						if (x < permissionsArray.size() - 1) {
							msgString += ", ";
						}
					}

					EmbedFieldData field6{ .value = "", .name = "__Roles:__", .Inline = false };

					for (uint32_t x = 0; x < guildMember.roles.size(); x += 1) {
						field6.value += "<@&" + guildMember.roles[x] + ">";
						if (x < guildMember.roles.size() - 1) {
							field6.value += ", ";
						}
					}
					if (field6.value != "") {
						fields.push_back(field6);
					}
					if (msgString != "") {
						EmbedFieldData field7 = { .value = msgString, .name = "__Permissions:__", .Inline = false };
						fields.push_back(field7);
					}

					msgEmbed.setImage(guildMember.user.avatar);
				} else if (user.userName != "") {
					EmbedFieldData field{ .value = user.userName + "#" + user.discriminator, .name = "__User Tag: __", .Inline = true };
					fields.push_back(field);
					EmbedFieldData field1{ .value = user.userName, .name = "__User Name:__", .Inline = true };
					fields.push_back(field1);
					EmbedFieldData field3{ .value = user.id, .name = "__User ID:__", .Inline = true };
					fields.push_back(field3);
					EmbedFieldData field5{ .value = user.createdAt, .name = "__Created At:__", .Inline = true };
					fields.push_back(field5);
					EmbedFieldData field6{ .value = "", .name = "__Roles:__", .Inline = false };
					std::string msgString{ "__**User Info**__" };
					msgEmbed.setDescription(msgString);
					msgEmbed.setImage(user.avatar);
				} else {
					std::string msgString = "------\n**Please enter a valid user ID or user mention! (!userinfo = @USERMENTION)**\n------";
					msgEmbed.setDescription(msgString);
				}


				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**User Info:**__");

				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.fields = fields;
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.addMessageEmbed(msgEmbed);
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				auto eventNew = InputEvents::respondToEventAsync(dataPackage).get();

				return;
			} catch (...) {
				reportException("UserInfo::execute()");
			}
		}
		virtual ~UserInfo(){};
	};

}
