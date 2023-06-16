// Test.hpp - Header for the "test" command.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"
#include "RegisterApplicationCommands.hpp"

namespace DiscordCoreAPI {

	class Test : public BaseFunction {
	  public:
		Test() {
			this->commandName	  = "test";
			this->helpDescription = "Testing purposes!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /test!\n------");
			msgEmbed.setTitle("__**Test Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Test>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				jsonifier::vector<Song> searchResults{};
				GuildData guild{ argsNew.getInteractionData().guildId };
				GuildMemberData guildMember{ argsNew.getGuildMemberData() };
				RespondToInputEventData dataPackage0(argsNew.getInputEventData());
				dataPackage0.setResponseType(InputEventResponseType::Deferred_Response);
				jsonifier::vector<EmbedData> embedsFromSearch;
				CreateMessageData createMessageData{};
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage0).get();

				createMessageData.channelId = argsNew.getChannelData().id;
				for (size_t x = 0; x < 40; ++x) {
					EmbedData newData{};
					createMessageData.addContent("Testing: " + jsonifier::toString(x) + "\r\n");
					Messages::createMessageAsync(createMessageData);
					newData.description = "Testing: " + jsonifier::toString(x) + "\r\n";
					newData.setColor("fefefe");
					newData.timeStamp = getTimeAndDate();
					embedsFromSearch.emplace_back(newData);
				}
				jsonifier::vector<CoRoutine<InputEventData>> theVector{};
				for (size_t x = 0; x < 40; ++x) {
					RespondToInputEventData responseData{ newEvent };
					responseData.setResponseType(InputEventResponseType::Follow_Up_Message);
					responseData.addMessageEmbed(embedsFromSearch[x]);
					InputEvents::respondToInputEventAsync(responseData);
				}

			} catch (const std::exception& error) {
				std::cout << "Test::execute()" << error.what() << std::endl;
			}
		}
		~Test(){};
	};
}// namespace DiscordCoreAPI
