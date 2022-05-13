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
				if (newArgs.commandData.optionsArgs.size() > 0) {
					std::cout << "THE RESULT: " << newArgs.commandData.optionsArgs[0] << std::endl;
				} else if (newArgs.commandData.optionsArgs.size() > 1) {
					std::cout << "THE RESULT 01: " << newArgs.commandData.optionsArgs[0] << std::endl;
					std::cout << "THE RESULT 02: " << newArgs.commandData.optionsArgs[1] << std::endl;
				}
				
				CreateGlobalApplicationCommandData createTestData{};
				createTestData.dmPermission = true;
				createTestData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createTestData.type = ApplicationCommandType::Chat_Input;
				createTestData.name = "test";
				createTestData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createTestData.description = "Test command.";
				ApplicationCommandOptionData testOptionOne{};
				testOptionOne.type = ApplicationCommandOptionType::Attachment;
				testOptionOne.name = "attachment";
				testOptionOne.required = false;
				testOptionOne.description = "Test attachment!";
				createTestData.options.push_back(testOptionOne);
				ApplicationCommandOptionData testOptionTwo{};
				testOptionTwo.type = ApplicationCommandOptionType::String;
				testOptionTwo.name = "test_string";
				testOptionTwo.required = false;
				testOptionTwo.autocomplete = true;
				testOptionTwo.description = "Test string!";
				createTestData.options.push_back(testOptionTwo);
				ApplicationCommands::createGlobalApplicationCommandAsync(createTestData).get();

			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}