// Test.hpp - Header for the "test" command.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Test : public BaseFunction {
	  public:
		Test() {
			this->commandName = "test";
			this->helpDescription = "Testing purposes!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /test!\n------");
			msgEmbed.setTitle("__**Test Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Test>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				
				RespondToInputEventData dataPackage{ newArgs.eventData };
				dataPackage.setResponseType(InputEventResponseType::Deferred_Response);
				auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
				InputEvents::deleteInputEventResponseAsync(newEvent);
				for (uint32_t x = 0; x < 50; x += 1) {
					CreateMessageData dataPackage03{};
					dataPackage03.channelId = newArgs.eventData.getChannelId();
					dataPackage03.addContent("MESSAGE #: " + std::to_string(x) + "\n<t:" +
						std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + ":F>");
					Messages::createMessageAsync(dataPackage03);
				}
				auto guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				std ::vector<CoRoutine<GuildMember>> theMembers{};
				auto x = 0;
				//guild.members.size();

				for (auto& [key, value]: guild.members) {
					x += 1;
					auto newGuildMember = GuildMembers::getGuildMemberAsync({ .guildMemberId = value.user.id, .guildId = newArgs.eventData.getGuildId() });
					theMembers.push_back(std::move(newGuildMember));
					if (x >= guild.members.size() / 10) {
						break;

					}
					std::cout << "WERE HERE THIS IS IT" << std::endl;
				}

				for (auto& value: theMembers) {
					std::cout << "WERE HERE THIS IS IT" << value.get().user.userName << std::endl;
				}

				
				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}