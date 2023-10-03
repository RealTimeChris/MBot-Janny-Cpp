// main.cpp - Main entry point.
// jun 17, 2021
// chris m.
// https://github.com/real_time_chris

#include "commands/CommandsList.hpp"

discord_core_api::co_routine<void> onGuildCreation(discord_core_api::on_guild_creation_data dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::discord_guild discordGuild{discord_core_api::managerAgent, dataPackage.value };
	discordGuild.getDataFromDB(discord_core_api::managerAgent);
	for (auto& value: discordGuild.data.deletionChannels) {
		value.currentlyBeingDeleted = false;
	}
	discordGuild.writeDataToDB(discord_core_api::managerAgent);

	for (auto& value: dataPackage.value.members) {
		auto guildMemberNew = discord_core_api::guild_members::getCachedGuildMember({ .guildMemberId = value.user.id, .guildId = dataPackage.value.id });
		for (auto& value02: discordGuild.data.defaultRoleIds) {
			bool isItFound{ false };
			for (auto& value03: guildMemberNew.roles) {
				if (value03 == value02) {
					isItFound = true;
				}
			}
			if (!isItFound) {
				discord_core_api::add_guild_member_role_data dataPackage02{};
				dataPackage02.guildId = discordGuild.data.guildId;
				dataPackage02.reason = "default role!";
				dataPackage02.roleId = value02;
				dataPackage02.userId = value.user.id;
				discord_core_api::roles::addGuildMemberRoleAsync(dataPackage02).get();
			}
		}

		discord_core_api::discord_guild_member guildMember{ discord_core_api::managerAgent, guildMemberNew };
		guildMember.writeDataToDB(discord_core_api::managerAgent);
	}
	co_return;
}

discord_core_api::co_routine<void> onGuildMemberAdd(discord_core_api::on_guild_member_add_data dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::addNewUserRoleAsync(dataPackage.value).get();
	co_return;
}

discord_core_api::co_routine<void> onInviteCreation(discord_core_api::on_invite_creation_data dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::monitor_invites::execute(dataPackage);
	co_return;
}

discord_core_api::co_routine<void> onInviteDeletion(discord_core_api::on_invite_deletion_data dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::monitor_invites::execute(dataPackage);
	co_return;
}

void onBoot02(discord_core_api::discord_core_client*) {
	jsonifier::vector<discord_core_api::guild_data> guilds = discord_core_api::guilds::getAllGuildsAsync();
	for (auto& value: guilds) {
		discord_core_api::monitor_invites::updateInvitesDataBaseToWrap(value.id);
	}
}

void onBoot00(discord_core_api::discord_core_client* args) {
	auto botUser = args->getBotUser();
	discord_core_api::managerAgent.initialize(botUser.id);
	discord_core_api::discord_user theUser{ discord_core_api::managerAgent, botUser.userName, botUser.id };
}

int32_t main() {
	jsonifier::vector<int32_t> uniquePtr{};
	jsonifier::vector<discord_core_api::repeated_function_data> functionVector{};
	discord_core_api::repeated_function_data function01{};
	function01.function = &onBoot00;
	function01.intervalInMs = 2500;
	function01.repeated = false;
	functionVector.emplace_back(function01);
	discord_core_api::repeated_function_data function03{};
	function03.function = &onBoot02;
	function03.intervalInMs = 15000;
	function03.repeated = false;
	functionVector.emplace_back(function03);
	discord_core_api::repeated_function_data function04{};
	function04.function = &discord_core_api::deleteMessages;
	function04.intervalInMs = 60000;
	function04.repeated = true;
	functionVector.emplace_back(function04);
	discord_core_api::register_application_commands theData{};
	discord_core_api::sharding_options shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 1;
	shardOptions.startingShard = 0;
	shardOptions.totalNumberOfShards = 1;
	discord_core_api::logging_options logOptions{};
	logOptions.logWebSocketErrorMessages = true;
	logOptions.logGeneralErrorMessages	 = true;
	logOptions.logHttpsErrorMessages	 = true;
	discord_core_api::discord_core_client_config clientConfig{};
	//clientConfig.connectionAddress = "192.168.0.10";
	clientConfig.botToken = "";
	clientConfig.logOptions = logOptions;
	clientConfig.shardOptions = shardOptions;
	clientConfig.cacheOptions.cacheChannels = true;
	clientConfig.cacheOptions.cacheGuilds = true;
	clientConfig.cacheOptions.cacheUsers = true;
	clientConfig.cacheOptions.cacheRoles = true;
	clientConfig.cacheOptions.cacheGuildMembers = true;
	clientConfig.functionsToExecute = functionVector;
	jsonifier::vector<discord_core_api::activity_data> activities{};
	discord_core_api::activity_data activity{};
	activity.type  = discord_core_api::activity_type::custom;
	activity.state = "enter /help for a list of my commands!";
	activity.name  = "enter /help for a list of my commands!";
	activities.emplace_back(activity);
	clientConfig.presenceData.activities = activities;
	clientConfig.presenceData.afk		 = false;
	clientConfig.textFormat				 = discord_core_api::text_format::etf;
	auto thePtr = discord_core_api::makeUnique<discord_core_api::discord_core_client>(clientConfig);
	thePtr->getEventManager().onInviteCreation(onInviteCreation);
	thePtr->getEventManager().onInviteDeletion(onInviteDeletion);
	thePtr->getEventManager().onGuildCreation(onGuildCreation);
	thePtr->getEventManager().onGuildMemberAdd(onGuildMemberAdd);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "avatar" }, discord_core_api::makeUnique<discord_core_api::Avatar>(), theData.createAvatarData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "ban" }, discord_core_api::makeUnique<discord_core_api::ban>(), theData.createBanCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "botinfo" }, discord_core_api::makeUnique<discord_core_api::bot_info>(),
		theData.createBotInfoCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "displayguildsdata" }, discord_core_api::makeUnique<discord_core_api::display_guilds_data>(),
		theData.createDisplayGuildsDataCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "disconnect" }, discord_core_api::makeUnique<discord_core_api::disconnect>(),
		theData.createDisconnectData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "ghost" }, discord_core_api::makeUnique<discord_core_api::ghost>(),
		theData.createGhostCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "help" }, discord_core_api::makeUnique<discord_core_api::help>(), theData.createHelpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "managelogs" }, discord_core_api::makeUnique<discord_core_api::manage_logs>(),
		theData.createManageLogsData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "purge" }, discord_core_api::makeUnique<discord_core_api::purge>(),
		theData.createPurgeCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "serverinfo" }, discord_core_api::makeUnique<discord_core_api::server_info>(),
		theData.createServerInfoCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setbordercolor" }, discord_core_api::makeUnique<discord_core_api::set_border_color>(),
		theData.createSetBorderColorCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setdefaultrole" }, discord_core_api::makeUnique<discord_core_api::set_default_role>(),
		theData.createSetDefaultRoleIdsData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setdeletionstatus" }, discord_core_api::makeUnique<discord_core_api::set_deletion_status>(),
		theData.createSetDeletionStatusCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setinviteschannel" }, discord_core_api::makeUnique<discord_core_api::set_invites_channel>(),
		theData.createSetInvitesChannelCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "streamaudio" }, discord_core_api::makeUnique<discord_core_api::stream_audio>(),
		theData.createStreamAudioData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "test" }, discord_core_api::makeUnique<discord_core_api::test>(), theData.createTestData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "pushme" }, discord_core_api::makeUnique<discord_core_api::push_me>(),
		theData.createPushmeCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "userinfo", "user info" }, discord_core_api::makeUnique<discord_core_api::user_info>(),
		theData.createUserInfoData);
	thePtr->runBot();
	return 0;
}
