// main.cpp - Main entry point.
// Jun 17, 2021
// Chris M.
// https://github.com/RealTimeChris
#ifndef AVX_TYPE
	#define AVX_TYPE 126
#endif
#include "Commands/CommandsList.hpp"

DiscordCoreAPI::CoRoutine<void> onGuildCreation(DiscordCoreAPI::OnGuildCreationData dataPackage) {
	co_await DiscordCoreAPI::NewThreadAwaitable<void>();
	DiscordCoreAPI::DiscordGuild discordGuild{ DiscordCoreAPI::managerAgent, dataPackage.value };
	discordGuild.getDataFromDB(DiscordCoreAPI::managerAgent);
	for (auto& value : discordGuild.data.deletionChannels) {
		value.currentlyBeingDeleted = false;
	}
	discordGuild.writeDataToDB(DiscordCoreAPI::managerAgent);

	for (auto& value : dataPackage.value.members) {
		auto guildMemberNew = DiscordCoreAPI::GuildMembers::getCachedGuildMember({ .guildMemberId = value.user.id, .guildId = dataPackage.value.id });
		for (auto& value02 : discordGuild.data.defaultRoleIds) {
			bool isItFound{ false };
			for (auto& value03 : guildMemberNew.roles) {
				if (value03 == value02) {
					isItFound = true;
				}
			}
			if (!isItFound) {
				DiscordCoreAPI::AddGuildMemberRoleData dataPackage02{};
				dataPackage02.guildId = discordGuild.data.guildId;
				dataPackage02.reason  = "Default Role!";
				dataPackage02.roleId  = value02;
				dataPackage02.userId  = value.user.id;
				DiscordCoreAPI::Roles::addGuildMemberRoleAsync(dataPackage02).get();
			}
		}

		DiscordCoreAPI::DiscordGuildMember guildMember{ DiscordCoreAPI::managerAgent, guildMemberNew };
		guildMember.writeDataToDB(DiscordCoreAPI::managerAgent);
	}
	co_return;
}

DiscordCoreAPI::CoRoutine<void> onGuildMemberAdd(DiscordCoreAPI::OnGuildMemberAddData dataPackage) {
	co_await DiscordCoreAPI::NewThreadAwaitable<void>();
	DiscordCoreAPI::addNewUserRoleAsync(dataPackage.value).get();
	co_return;
}

DiscordCoreAPI::CoRoutine<void> onInviteCreation(DiscordCoreAPI::OnInviteCreationData dataPackage) {
	co_await DiscordCoreAPI::NewThreadAwaitable<void>();
	DiscordCoreAPI::MonitorInvites::execute(dataPackage);
	co_return;
}

DiscordCoreAPI::CoRoutine<void> onInviteDeletion(DiscordCoreAPI::OnInviteDeletionData dataPackage) {
	co_await DiscordCoreAPI::NewThreadAwaitable<void>();
	DiscordCoreAPI::MonitorInvites::execute(dataPackage);
	co_return;
}

void onBoot02(DiscordCoreAPI::DiscordCoreClient*) {
	jsonifier::vector<DiscordCoreAPI::GuildData> guilds = DiscordCoreAPI::Guilds::getAllGuildsAsync();
	for (auto& value : guilds) {
		DiscordCoreAPI::MonitorInvites::updateInvitesDataBaseToWrap(value.id);
	}
}

void onBoot00(DiscordCoreAPI::DiscordCoreClient* args) {
	auto botUser = args->getBotUser();
	DiscordCoreAPI::managerAgent.initialize(botUser.id);
	DiscordCoreAPI::DiscordUser theUser{ DiscordCoreAPI::managerAgent, botUser.userName, botUser.id };
}

int32_t main() {
	DiscordCoreAPI::UniquePtr<jsonifier::vector<int32_t>> uniquePtr{ std::make_unique<jsonifier::vector<int32_t>>() };
	jsonifier::vector<DiscordCoreAPI::RepeatedFunctionData> functionVector{};
	DiscordCoreAPI::RepeatedFunctionData function01{};
	function01.function		= &onBoot00;
	function01.intervalInMs = 2500;
	function01.repeated		= false;
	functionVector.emplace_back(function01);
	DiscordCoreAPI::RepeatedFunctionData function03{};
	function03.function		= &onBoot02;
	function03.intervalInMs = 15000;
	function03.repeated		= false;
	functionVector.emplace_back(function03);
	DiscordCoreAPI::RepeatedFunctionData function04{};
	function04.function		= &DiscordCoreAPI::deleteMessages;
	function04.intervalInMs = 60000;
	function04.repeated		= true;
	functionVector.emplace_back(function04);
	DiscordCoreAPI::RegisterApplicationCommands theData{};
	DiscordCoreAPI::ShardingOptions shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 1;
	shardOptions.startingShard				  = 0;
	shardOptions.totalNumberOfShards		  = 1;
	DiscordCoreAPI::LoggingOptions logOptions{};
	logOptions.logWebSocketSuccessMessages = true;
	logOptions.logWebSocketErrorMessages   = true;
	logOptions.logGeneralErrorMessages	   = true;
	logOptions.logHttpsErrorMessages	   = true;
	logOptions.logHttpsSuccessMessages	   = true;
	DiscordCoreAPI::DiscordCoreClientConfig clientConfig{};
	// clientConfig.connectionAddress = "192.168.0.10";
	clientConfig.botToken						= "";
	clientConfig.logOptions						= logOptions;
	clientConfig.shardOptions					= shardOptions;
	clientConfig.cacheOptions.cacheChannels		= true;
	clientConfig.cacheOptions.cacheGuilds		= true;
	clientConfig.cacheOptions.cacheUsers		= true;
	clientConfig.cacheOptions.cacheRoles		= true;
	clientConfig.cacheOptions.cacheGuildMembers = true;
	clientConfig.functionsToExecute				= functionVector;
	jsonifier::vector<DiscordCoreAPI::ActivityData> activities{};
	DiscordCoreAPI::ActivityData activity{};
	activity.name = "/help for my commands!";
	activity.type = DiscordCoreAPI::ActivityType::Game;
	activities.emplace_back(activity);
	clientConfig.presenceData.activities = activities;
	clientConfig.presenceData.afk		 = false;
	clientConfig.textFormat				 = DiscordCoreAPI::TextFormat::Etf;
	clientConfig.presenceData.since		 = 0;
	clientConfig.presenceData.status	 = DiscordCoreAPI::PresenceUpdateState::Online;
	auto thePtr							 = DiscordCoreAPI::makeUnique<DiscordCoreAPI::DiscordCoreClient>(clientConfig);
	thePtr->getEventManager().onInviteCreation(onInviteCreation);
	thePtr->getEventManager().onInviteDeletion(onInviteDeletion);
	thePtr->getEventManager().onGuildCreation(onGuildCreation);
	thePtr->getEventManager().onGuildMemberAdd(onGuildMemberAdd);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "avatar" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Avatar>(), theData.createAvatarData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "ban" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Ban>(), theData.createBanCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "botinfo" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::BotInfo>(), theData.createBotInfoCommandData);
	thePtr->registerFunction(
		jsonifier::vector<jsonifier::string>{ "displayguildsdata" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::DisplayGuildsData>(), theData.createDisplayGuildsDataCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "disconnect" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Disconnect>(), theData.createDisconnectData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "ghost" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Ghost>(), theData.createGhostCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "help" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Help>(), theData.createHelpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "managelogs" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::ManageLogs>(), theData.createManageLogsData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "purge" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Purge>(), theData.createPurgeCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "serverinfo" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::ServerInfo>(), theData.createServerInfoCommandData);
	thePtr->registerFunction(
		jsonifier::vector<jsonifier::string>{ "setbordercolor" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetBorderColor>(), theData.createSetBorderColorCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setdefaultrole" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetDefaultRole>(), theData.createSetDefaultRoleIdsData);
	thePtr->registerFunction(
		jsonifier::vector<jsonifier::string>{ "setdeletionstatus" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetDeletionStatus>(), theData.createSetDeletionStatusCommandData);
	thePtr->registerFunction(
		jsonifier::vector<jsonifier::string>{ "setinviteschannel" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetInvitesChannel>(), theData.createSetInvitesChannelCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "streamaudio" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::StreamAudio>(), theData.createStreamAudioData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "test" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Test>(), theData.createTestData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "pushme" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::PushMe>(), theData.createPushmeCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "userinfo", "user info" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::UserInfo>(), theData.createUserInfoData);
	thePtr->runBot();
	return 0;
}
