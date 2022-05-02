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
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				auto newEvent = InputEvents::respondToEventAsync(dataPackage).get();
				RespondToInputEventData dataPackage02{ newEvent };
				dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
				dataPackage02.addContent("TEST MESSAGE: TEST");
				InputEvents::respondToEventAsync(dataPackage02);
				for (uint32_t x = 0; x < 50; x += 1) {
					RespondToInputEventData dataPackage02{ newEvent };
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					dataPackage02.addContent("TEST MESSAGE: " + std::to_string(x));
					InputEvents::respondToEventAsync(dataPackage02);
				}
				auto guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				std ::vector<CoRoutine<GuildMember>> theMembers{};
				auto x = 0;
				//guild.members.size();

				for (auto& [key, value]: guild.members) {
					x += 1;
					auto newGuildMember = GuildMembers::getGuildMemberAsync({ .guildMemberId = value.user.id, .guildId = newArgs.eventData.getGuildId() });
					theMembers.push_back(std::move(newGuildMember));
					std::cout << "WERE HERE THIS IS IT" << std::endl;
					if (x >= guild.members.size() / 10) {
						break;
					}
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