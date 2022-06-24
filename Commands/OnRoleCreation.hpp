// OnRoleCreation.hpp - Header for the "on role creation" command.
// Sep 11, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	void onRoleCreationToBeWrapped(OnRoleCreationData dataPackageNew) {
		try {
			if (dataPackageNew.role.id == 0) {
				return;
			}

			Guild guild = Guilds::getCachedGuildAsync({ .guildId = dataPackageNew.guildId }).get();
			DiscordGuild discordGuild{ guild };

			Log log{};
			for (int32_t x = 0; x < discordGuild.data.logs.size(); x += 1) {
				if (discordGuild.data.logs[x].nameSmall == "rolecreate") {
					log = discordGuild.data.logs[x];
					break;
				}
			}

			if (log.enabled == true) {
				Channel textChannel = Channels::getCachedChannelAsync({ .channelId = log.loggingChannelId }).get();

				AuditLogData auditLogs =
					Guilds::getGuildAuditLogsAsync({ .actionType = AuditLogEvent::Role_Create, .guildId = dataPackageNew.guildId, .userId = 0, .limit = 1 }).get();
				int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				AuditLogEntryData auditLogEntry{};
				for (auto& value: auditLogs.auditLogEntries) {
					if (currentTime - value.createdTimeStamp.convertTimestampToMsInteger() < 15000) {
						auditLogEntry = value;
					}
				}
				Role newRole = Roles::getCachedRoleAsync({ .roleId = dataPackageNew.role.id }).get();
				EmbedData msgEmbed{};
				std::string msgString{};
				msgString = "__**New Role:**__ <@&" + std::to_string(newRole.id) + ">(" + newRole.name + ")\n";
				User user = Users::getUserAsync({ .userId = auditLogEntry.userId }).get();
				msgString += "__**Created By:**__ <@" + std::to_string(user.id) + ">(" + user.userName + "#" + user.discriminator + ")\n";
				msgString += "__**Role Count:**__ " + std::to_string(guild.roles.size());

				msgEmbed.setTitle("__**Role Created:**__");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setDescription(msgString);
				msgEmbed.setColor(discordGuild.data.borderColor);
				CreateMessageData dataPackage01{};
				dataPackage01.addContent(msgString);
				dataPackage01.channelId = textChannel.id;
				dataPackage01.addMessageEmbed(msgEmbed);
				Messages::createMessageAsync(dataPackage01).get();
			}
		} catch (...) {
			reportException("onRoleCreationToBeWrapped Error: ");
		}
	}

	void onRoleCreationEvent(OnRoleCreationData dataPackageNew) {
		ThreadPool::executeFunctionAfterTimePeriod<OnRoleCreationData>(onRoleCreationToBeWrapped, 10000, true, dataPackageNew);
	}
}
