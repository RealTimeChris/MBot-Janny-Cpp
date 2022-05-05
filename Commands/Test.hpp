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
				RespondToInputEventData dataPackage00{ newArgs.eventData };
				dataPackage00.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				for (auto& [key, value]: newArgs.eventData.getInteractionData().data.applicationCommanddata.resolved.attachments) {
					std::cout << "THE KEY: " << key << "\nTHE VALUES: " << value.contentType << "\n" << value.filename << std::endl;
					std::cout << value.url << std::endl;
				}
				std::cout << "THE SIZE: " << newArgs.eventData.getInteractionData().data.applicationCommanddata.resolved.attachments.size() << std::endl;
				auto newEvent = InputEvents::respondToEventAsync(dataPackage00).get();
				StartThreadInForumChannelData dataPackage{};
				dataPackage.channelId = newArgs.eventData.getChannelId();
				dataPackage.message.content = "TESTING";
				dataPackage.name = "THE TEST THREAD";
				Threads::startThreadInForumChannelAsync(dataPackage).get();
				RespondToInputEventData dataPackage02{ newEvent };
				dataPackage02.setResponseType(InputEventResponseType::Edit_Ephemeral_Interaction_Response);
				EmbedData package02{};
				package02.setDescription("TESTING DESCRIPTION");
				dataPackage02.addMessageEmbed(package02);
				dataPackage02.addContent("TEESTING");
				InputEvents::respondToEventAsync(dataPackage02).get();
				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}