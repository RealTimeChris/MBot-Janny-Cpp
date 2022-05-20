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

		void execute(BaseFunctionArguments& newArgs) {
			try {
				RespondToInputEventData dataPackage{ newArgs.eventData };
				File theFile{};
				theFile.fileName = "Cran04.jpeg";
				theFile.data = loadFileContents("C:/Users/Chris/Downloads/Cran04.jpeg");
				dataPackage.addFile(theFile);
				EmbedData theEmbed{};
				theEmbed.description = "TESTING!";
				theEmbed.image.url = "attachment://Cran04.jpeg";
				dataPackage.addMessageEmbed(theEmbed);
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				auto inputsNew = InputEvents::respondToInputEventAsync(dataPackage).get();

				for (uint32_t x = 0; x < 50; x += 1) {
					RespondToInputEventData dataPackageNew{ inputsNew };
					dataPackageNew.addContent("POST NUMBER: " + std::to_string(x) +
						"\n<t:" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + ":F>");
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					InputEvents::respondToInputEventAsync(dataPackageNew);
				}
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		~Test(){};
	};
}