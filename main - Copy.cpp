// main.cpp - Main entry point.
// Jun 17, 2021
// Chris M.
// https://github.com/RealTimeChris

#include "Commands/CommandsList.hpp"

void onInviteCreation(DiscordCoreAPI::OnInviteCreationData dataPackage) {
	DiscordCoreAPI::MonitorInvites::execute(dataPackage);
}

void onInviteDeletion(DiscordCoreAPI::OnInviteDeletionData dataPackage) {
	DiscordCoreAPI::MonitorInvites::execute(dataPackage);
}

void onGuildCreation(DiscordCoreAPI::OnGuildCreationData dataPackage) {
	DiscordCoreAPI::DiscordGuild discordGuild{ dataPackage.guild };
	discordGuild.getDataFromDB();
	
	for (auto& value: dataPackage.guild.members) {
		auto guildMemberNew = DiscordCoreAPI::GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = value, .guildId = dataPackage.guild.id }).get();
		for (auto& value02: discordGuild.data.defaultRoleIds) {
			bool isItFound{ false };
			for (auto& value03: guildMemberNew.roles) {
				if (value03 == value02) {
					isItFound = true;
				}
			}
			if (!isItFound) {
				DiscordCoreAPI::AddGuildMemberRoleData dataPackage02{};
				dataPackage02.guildId = discordGuild.data.guildId;
				dataPackage02.reason = "Default Role!";
				dataPackage02.roleId = value02;
				dataPackage02.userId = value;
				DiscordCoreAPI::Roles::addGuildMemberRoleAsync(dataPackage02).get();
			}
		}

		DiscordCoreAPI::DiscordGuildMember guildMember{ guildMemberNew };
		guildMember.writeDataToDB();
	}
	for (auto& value: discordGuild.data.deletionChannels) {
		value.currentlyBeingDeleted = false;
	}
	discordGuild.writeDataToDB();
}

void onGuildMemberAdd(DiscordCoreAPI::OnGuildMemberAddData dataPackage) {
	DiscordCoreAPI::DiscordGuildMember guildMember(dataPackage.guildMember);
	DiscordCoreAPI::addNewUserRoleAsync(dataPackage.guildMember).get();

	DiscordCoreAPI::MonitorInvitesArgs inviteArgs(dataPackage.guildMember, dataPackage.discordCoreClient);
	DiscordCoreAPI::MonitorInvites::execute(inviteArgs);
}

void onBoot02(DiscordCoreAPI::DiscordCoreClient*) {
	std::vector<DiscordCoreAPI::GuildData> guilds = DiscordCoreAPI::Guilds::getAllGuildsAsync().get();
	for (auto& value: guilds) {
		std::jthread theThread = std::jthread{ [=]() {
			DiscordCoreAPI::MonitorInvites::updateInvitesDataBaseToWrap(value.id);
		} };
		theThread.detach();
	}
}

void onBoot00(DiscordCoreAPI::DiscordCoreClient* args) {
	auto botUser = args->getBotUser();
	DiscordCoreAPI::DatabaseManagerAgent::initialize(botUser.id);
	DiscordCoreAPI::DiscordUser theUser{ botUser.userName, botUser.id };
}

void onBoot01(DiscordCoreAPI::DiscordCoreClient* args) {
	std::vector<DiscordCoreAPI::ActivityData> activities{};
	DiscordCoreAPI::ActivityData activity{};
	activity.name = "/help for my commands!";
	activity.type = DiscordCoreAPI::ActivityType::Game;
	activities.push_back(activity);
	DiscordCoreAPI::UpdatePresenceData dataPackage{ .activities = activities, .status = "online", .afk = false };
	args->getBotUser().updatePresence(dataPackage);
}

int32_t main() {
	std::string botToken = "YOUR_BOT_TOKEN_HERE";
	std::vector<DiscordCoreAPI::RepeatedFunctionData> functionVector{};
	functionVector.reserve(5);
	DiscordCoreAPI::RepeatedFunctionData function00{};
	function00.function = DiscordCoreAPI::startupToWrapTwo;
	function00.intervalInMs = 10000;
	function00.repeated = false;
	functionVector.push_back(function00);
	DiscordCoreAPI::RepeatedFunctionData function01{};
	function01.function = onBoot00;
	function01.intervalInMs = 0;
	function01.repeated = false;
	functionVector.push_back(function01);
	DiscordCoreAPI::RepeatedFunctionData function02{};
	function02.function = onBoot01;
	function02.intervalInMs = 500;
	function02.repeated = false;
	functionVector.push_back(function02);
	DiscordCoreAPI::RepeatedFunctionData function03{};
	function03.function = onBoot02;
	function03.intervalInMs = 15000;
	function03.repeated = false;
	//functionVector.push_back(function03);
	DiscordCoreAPI::RepeatedFunctionData function04{};
	function04.function = std::ref(DiscordCoreAPI::deleteMessages);
	function04.intervalInMs = 60000;
	function04.repeated = true;
	functionVector.push_back(function04);
	DiscordCoreAPI::ShardingOptions shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 1;
	shardOptions.startingShard = 0;
	shardOptions.totalNumberOfShards = 1;
	DiscordCoreAPI::LoggingOptions logOptions{};
	logOptions.logFFMPEGErrorMessages = true;
	logOptions.logGeneralErrorMessages = true;
	logOptions.logHttpErrorMessages = true;
	logOptions.logWebSocketErrorMessages = true;
	DiscordCoreAPI::DiscordCoreClientConfig clientConfig{};
	//clientConfig.alternateConnectionAddress = "127.0.0.1";
	clientConfig.botToken = botToken;
	clientConfig.logOptions = logOptions;
	clientConfig.shardOptions = shardOptions;
	clientConfig.functionsToExecute = functionVector;
	auto thePtr = std::make_unique<DiscordCoreAPI::DiscordCoreClient>(clientConfig);
	thePtr->eventManager.onGuildCreation(&onGuildCreation);
	thePtr->eventManager.onGuildMemberAdd(&onGuildMemberAdd);
	thePtr->eventManager.onInviteCreation(&onInviteCreation);
	thePtr->eventManager.onInviteDeletion(&onInviteDeletion);
	thePtr->eventManager.onMessageCreation(&DiscordCoreAPI::onMessageReceived);
	thePtr->registerFunction(std::vector<std::string>{ "ban" }, std::make_unique<DiscordCoreAPI::Ban>());
	thePtr->registerFunction(std::vector<std::string>{ "botinfo" }, std::make_unique<DiscordCoreAPI::BotInfo>());
	thePtr->registerFunction(std::vector<std::string>{ "displayguildsdata" }, std::make_unique<DiscordCoreAPI::DisplayGuildsData>());
	thePtr->registerFunction(std::vector<std::string>{ "ghost" }, std::make_unique<DiscordCoreAPI::Ghost>());
	thePtr->registerFunction(std::vector<std::string>{ "help" }, std::make_unique<DiscordCoreAPI::Help>());
	thePtr->registerFunction(std::vector<std::string>{ "managelogs" }, std::make_unique<DiscordCoreAPI::ManageLogs>());
	thePtr->registerFunction(std::vector<std::string>{ "purge" }, std::make_unique<DiscordCoreAPI::Purge>());
	thePtr->registerFunction(std::vector<std::string>{ "registerapplicationcommands" }, std::make_unique<DiscordCoreAPI::RegisterApplicationCommands>());
	thePtr->registerFunction(std::vector<std::string>{ "serverinfo" }, std::make_unique<DiscordCoreAPI::ServerInfo>());
	thePtr->registerFunction(std::vector<std::string>{ "setbordercolor" }, std::make_unique<DiscordCoreAPI::SetBorderColor>());
	thePtr->registerFunction(std::vector<std::string>{ "setdefaultrole" }, std::make_unique<DiscordCoreAPI::SetDefaultRole>());
	thePtr->registerFunction(std::vector<std::string>{ "setdeletionstatus" }, std::make_unique<DiscordCoreAPI::SetDeletionStatus>());
	thePtr->registerFunction(std::vector<std::string>{ "setinviteschannel" }, std::make_unique<DiscordCoreAPI::SetInvitesChannel>());
	thePtr->registerFunction(std::vector<std::string>{ "reactionrole" }, std::make_unique<DiscordCoreAPI::ReactionRole>());
	thePtr->registerFunction(std::vector<std::string>{ "test" }, std::make_unique<DiscordCoreAPI::Test>());
	thePtr->registerFunction(std::vector<std::string>{ "pushme" }, std::make_unique<DiscordCoreAPI::PushMe>());
	thePtr->registerFunction(std::vector<std::string>{ "userinfo", "user info" }, std::make_unique<DiscordCoreAPI::UserInfo>());
	thePtr->runBot();
	return 0;
}