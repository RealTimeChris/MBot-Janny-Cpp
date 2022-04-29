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
				for (uint32_t x = 0; x < 150; x += 1) {
					CreateMessageData dataPackage{};
					dataPackage.addContent("TEST MESSAGE: " + std::to_string(x));
					dataPackage.channelId = newArgs.eventData.getChannelId();
					Messages::createMessageAsync(dataPackage);
				}


				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		virtual ~Test(){};
	};
}
