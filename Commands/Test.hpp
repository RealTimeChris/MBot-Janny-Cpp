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
				
				File theFile{};
				theFile.fileName = "RTCHRISSANTAHAT.png";
				theFile.data = loadFileContents("C:/Users/Chris/Downloads/RTCHRISSANTAHAT.png");
				RespondToInputEventData dataPackage{ newArgs.eventData };
				dataPackage.addFile(theFile);
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				EmbedData package02{};
				package02.setDescription("TESTING DESCRIPTION");
				package02.setImage("attachment://meth01.jpg");
				dataPackage.addMessageEmbed(package02);
				dataPackage.addContent("TEESTING");
				InputEvents::respondToEventAsync(dataPackage).get();
				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}