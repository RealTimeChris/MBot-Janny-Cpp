// OnRoleDeletion.hpp - Header for the "on role deletion" command.
// Sep 11, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	void onRoleDeletionToBeWrapped(OnRoleDeletionData dataPackageNew) {
		try {
			if (dataPackageNew.roleOld.id == "") {
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
					Guilds::getGuildAuditLogsAsync({ .actionType = AuditLogEvent::Role_Delete, .guildId = dataPackageNew.guildId, .userId = "", .limit = 1 })
						.get();
				int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				AuditLogEntryData auditLogEntry{};
				for (auto value: auditLogs.auditLogEntries) {
					if (currentTime - convertTimestampToMsInteger(value.createdTimeStamp) < 10000) {
						auditLogEntry = value;
					}
				}

				EmbedData msgEmbed{};
				std::string msgString{};
				msgString = "__**Deleted Role:**__ " + dataPackageNew.roleOld.name + "\n";
				User user = Users::getUserAsync({ .userId = auditLogEntry.userId }).get();
				msgString += "__**Deleted By:**__ <@!" + user.id + ">(" + user.userName + "#" + user.discriminator + ")\n";
				msgString += "__**Role Count:**__ " + std::to_string(guild.roles.size());

				msgEmbed.setTitle("__**Role Deleted:**__");
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
			reportException("onRoleDeletionToBeWrapped Error: ");
		}
	}

	void onRoleDeletionEvent(OnRoleDeletionData dataPackageNew) {
		ThreadPool::executeFunctionAfterTimePeriod<OnRoleDeletionData>(onRoleDeletionToBeWrapped, 5000, false, dataPackageNew);
	}
}