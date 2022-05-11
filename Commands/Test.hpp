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
				RespondToInputEventData dataPackage02{ newArgs.eventData };
				dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage02.addContent(
					"<t:" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + ":F>");
				File theFile{};
				dataPackage02.addButton(false, "test_button", "TEST BUTTON", ButtonStyle::Danger, "👨");
				theFile.data = loadFileContents("C:/users/chris/downloads/Cran04.jpeg");
				theFile.fileName = "Cran02.jpeg";
				dataPackage02.addFile(theFile);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();
				std::vector<CoRoutine<InputEventData>> theResponses{};
				RespondToInputEventData dataPackage01{ newEvent };
				dataPackage01.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage01.addContent("TEST MESSAGE: " + std::to_string(1));
				newEvent = InputEvents::respondToInputEventAsync(dataPackage01).get();
				std::cout << "WERE HERE THIS IS REALLY IT 01010101" + std::to_string(2) << std::endl;
				for (uint32_t x = 0; x < 50; x += 1) {
					RespondToInputEventData dataPackage02{ newEvent };
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					dataPackage02.addContent("TEST MESSAGE: " + std::to_string(x));
					theResponses.push_back(InputEvents::respondToInputEventAsync(dataPackage02));
					std::cout << "WERE HERE THIS IS REALLY IT 01010101" + std::to_string(x) << std::endl;
				}
				auto guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				std ::vector<CoRoutine<GuildMember>> theMembers{};
				auto x = 0;

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
					std::cout << "WERE HERE THIS IS IT: " << value.get().user.userName << std::endl;
				}

				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}