// SetBorderColor.hpp - Header for the "set border color" command.
// Jun 26, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "./../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class SetBorderColor : public BaseFunction {
	  public:
		SetBorderColor() {
			this->commandName	  = "setbordercolor";
			this->helpDescription = "Set the bot's default border color for message embeds.";
			EmbedData msgEmbed;
			msgEmbed.setDescription("------\nSimply enter /setbordercolor <HEXCOLORVALUE>!\n------");
			msgEmbed.setTitle("__**Set Border Color Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetBorderColor>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };
				GuildMemberData guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember, true);
				if (!doWeHaveAdminPermission) {
					return;
				}
				jsonifier::string borderColor;

				if (std::stoull(argsNew.getCommandArguments().values["hexcolorvalue"].value, 0, 16) < 0 ||
					std::stoull(argsNew.getCommandArguments().values["hexcolorvalue"].value, 0, 16) > std::stoull("fefefe", 0, 16)) {
					jsonifier::string msgString = "------\n**Please, enter a hex-color value between 0 and FeFeFe! (!setbordercolor = BOTNAME, HEXCOLORVALUE)**\n------";
					EmbedData msgEmbed;
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					borderColor = argsNew.getCommandArguments().values["hexcolorvalue"].value;

					discordGuild.data.borderColor = borderColor;
					discordGuild.writeDataToDB(managerAgent);

					EmbedData msgEmbed;
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(discordGuild.data.borderColor);
					msgEmbed.setDescription(
						"Nicely done, you've updated the default border color for this bot!\n------\n__**Border Color Values:**__ " + discordGuild.data.borderColor + "\n------");
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Updated Border Color:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto argsNewer = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
			} catch (const std::exception& error) {
				std::cout << "SetBorderColor::execute()" << error.what() << std::endl;
			}
		};
		~SetBorderColor(){};
	};
};// namespace DiscordCoreAPI
