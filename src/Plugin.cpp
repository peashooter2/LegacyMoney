#include "Plugin.h"
#include "DefaultLangData.h"
#include "LLMoney.h"
#include "Plugin.h"
#include "Settings.h"
#include "ll/api/Logger.h"
#include "ll/api/command/Command.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandOutputMessageType.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandSelector.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "sqlitecpp/SQLiteCpp.h"


ll::Logger* logger;

#define JSON1(key, val)                                                                                                \
    if (json.find(key) != json.end()) {                                                                                \
        const nlohmann::json& out = json.at(key);                                                                      \
        out.get_to(val);                                                                                               \
    }

namespace Settings {
std::string language        = "en";
int         def_money       = 0;
float       pay_tax         = 0.0;
bool        enable_ranking  = true;
bool        enable_commands = true;

nlohmann::json globaljson() {
    nlohmann::json json;
    json["language"]       = language;
    json["def_money"]      = def_money;
    json["pay_tax"]        = pay_tax;
    json["enable_ranking"] = enable_ranking;
    return json;
}

void initjson(nlohmann::json json) {
    JSON1("language", language);
    JSON1("def_money", def_money);
    JSON1("pay_tax", pay_tax);
    JSON1("enable_ranking", enable_ranking);
}
void WriteDefaultConfig(const std::string& fileName) {

    std::ofstream file(fileName);
    if (!file.is_open()) {
        logger->error("Can't open file {}", fileName);
        return;
    }
    auto json = globaljson();
    file << json.dump(4);
    file.close();
}

void LoadConfigFromJson(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        logger->error("Can't open file {}", fileName);
        return;
    }
    nlohmann::json json;
    file >> json;
    file.close();
    initjson(json);
    WriteDefaultConfig(fileName);
}
void reloadJson(const std::string& fileName) {
    std::ofstream file(fileName);
    if (file) {
        file << globaljson().dump(4);
    } else {
        logger->error("Configuration File Creation failed!");
    }
    file.close();
}
} // namespace Settings

bool initDB();

bool cmp(std::pair<std::string, long long> a, std::pair<std::string, long long> b) { return a.second > b.second; }


using ll::i18n_literals::operator""_tr;

// class MoneyCommand : public Command {
//     enum MoneyOP : int { query = 1, hist = 2, pay = 3, set = 4, add = 5, reduce = 6, purge = 7, top = 8 } op;
//     std::string dst;
//     bool        dst_isSet;
//     bool        difftime_isSet;
//     int         moneynum;
//     int         difftime;

// public:
//     void execute(CommandOrigin const& ori, CommandOutput& outp) const {
//         std::string              dstxuid, myuid;
//         ll::service::PlayerInfo& info = ll::service::PlayerInfo::getInstance();
//         switch (op) {
//         case query:
//         case hist:
//             if (dst_isSet && ori.getPermissionsLevel() > CommandPermissionLevel::Any) {
//                 auto playerInfo = info.fromName(dst);
//                 if (playerInfo) {
//                     dstxuid = playerInfo->xuid;
//                 }
//             } else {
//                 if (ori.getOriginType() != CommandOriginType::Player) {
//                     outp.error("money.dontuseinconsole"_tr());
//                     return;
//                 }
//                 dstxuid = ((Player*)ori.getEntity())->getXuid();
//             }
//             if (dstxuid.empty()) {
//                 outp.error("money.no.target"_tr());
//                 return;
//             }
//             break;
//         case pay:
//             if (ori.getOriginType() != CommandOriginType::Player) {
//                 outp.error("money.dontuseinconsole"_tr());
//                 return;
//             }
//         case set:
//         case add:
//         case reduce: {
//             auto playerInfo = info.fromName(dst);
//             if (playerInfo) {
//                 dstxuid = playerInfo->xuid;
//             }
//             if (dstxuid.empty()) {
//                 outp.error("money.no.target"_tr());
//                 return;
//             }
//             break;
//         }
//         case purge:
//             if (ori.getOriginType() != CommandOriginType::Player) {
//                 outp.error("money.dontuseinconsole"_tr());
//                 return;
//             }
//             if (ori.getPermissionsLevel() < CommandPermissionLevel::GameDirectors) {
//                 outp.error("money.no.perm"_tr());
//                 return;
//             }
//             break;
//         case top:
//             if (!Settings::enable_ranking) {
//                 outp.error("Balance ranking not enabled");
//                 return;
//             }
//         }
//         switch (op) {
//         case query:
//             outp.addMessage("Balance: " + std::to_string(LLMoney_Get(dstxuid)), {},
//             CommandOutputMessageType::Success); break;
//         case hist:
//             outp.addMessage(LLMoney_GetHist(dstxuid), {}, CommandOutputMessageType::Success);
//             break;
//         case pay: {
//             if (moneynum <= 0) {
//                 outp.error("money.invalid.arg"_tr());
//                 return;
//             }
//             myuid = ((Player*)ori.getEntity())->getXuid();
//             if (myuid == dstxuid) {
//                 outp.error("money.payyourself"_tr());
//                 return;
//             }
//             if (LLMoney_Trans(myuid, dstxuid, moneynum, "money pay")) {
//                 long long fee = (long long)(moneynum * Settings::pay_tax);
//                 if (fee) LLMoney_Trans(dstxuid, "", fee, "money pay fee");
//                 outp.success("money.pay.succ"_tr());
//             } else {
//                 outp.error("money.not.enough"_tr());
//             }
//             return;
//         } break;
//         case set:
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error("money.no.perm"_tr());
//                 return;
//             }
//             if (LLMoney_Set(dstxuid, moneynum)) {
//                 outp.success("money.set.succ"_tr());
//             } else {
//                 outp.error("money.invalid.arg"_tr());
//             }
//             break;
//         case add:
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error("money.no.perm"_tr());
//                 return;
//             }
//             if (LLMoney_Add(dstxuid, moneynum)) {
//                 outp.success("money.add.succ"_tr());
//             } else {
//                 outp.error("money.invalid.arg"_tr());
//             }
//             break;
//         case reduce:
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error("money.no.perm"_tr());
//                 return;
//             }
//             if (LLMoney_Reduce(dstxuid, moneynum)) {
//                 outp.success("money.reduce.succ"_tr());
//             } else {
//                 outp.error("money.invalid.arg"_tr());
//             }
//             break;
//         case purge:
//             if (difftime_isSet) LLMoney_ClearHist(difftime);
//             else LLMoney_ClearHist(0);
//             break;
//         case top:
//             std::vector<std::pair<std::string, long long>> mapTemp = LLMoney_Ranking();
//             sort(mapTemp.begin(), mapTemp.end(), cmp);
//             outp.success("===== Ranking =====");
//             for (auto it = mapTemp.begin(); it != mapTemp.end(); it++) {
//                 auto playerInfo = info.fromXuid(it->first);
//                 outp.addMessage(
//                     (playerInfo ? playerInfo->name : "NULL") + "  " + std::to_string(it->second),
//                     {},
//                     CommandOutputMessageType::Success
//                 );
//             }
//             outp.addMessage("===================", {}, CommandOutputMessageType::Success);
//         }
//         return;
//     }

//     static void setup(CommandRegistry* registry) {

//         // registerCommand
//         registry->registerCommand(
//             "money",
//             "Economy system",
//             CommandPermissionLevel::Any,
//             {(CommandFlagValue)0},
//             {(CommandFlagValue)0x80}
//         );
//         registry->enum
//         // addEnum
//         registry->addEnum<MoneyOP>(
//             "MoneyOP1",
//             {
//                 {"query", MoneyOP::query},
//                 {"hist",  MoneyOP::hist },
//                 {"top",   MoneyOP::top  }
//         }
//         );
//         registry->addEnum<MoneyOP>(
//             "MoneyOP2",
//             {
//                 {"add",    MoneyOP::add   },
//                 {"pay",    MoneyOP::pay   },
//                 {"reduce", MoneyOP::reduce},
//                 {"set",    MoneyOP::set   }
//         }
//         );
//         registry->addEnum<MoneyOP>(
//             "MoneyOP3",
//             {
//                 {"purge", MoneyOP::purge}
//         }
//         );

//         // registerOverload
//         registry->registerOverload<MoneyCommand>(
//             "money",
//             CommandParameterData::makeMandatory<CommandParameterDataType::Enum>(
//                 &MoneyCommand::op,
//                 "optional",
//                 "MoneyOP1"
//             ),
//             CommandParameterData::makeOptional(&MoneyCommand::dst, "PlayerName", &MoneyCommand::dst_isSet)
//         );
//         registry->registerOverload<MoneyCommand>(
//             "money",
//             CommandParameterData::makeMandatory<CommandParameterDataType::Enum>(
//                 &MoneyCommand::op,
//                 "optional",
//                 "MoneyOP2"
//             ),
//             CommandParameterData::makeMandatory(&MoneyCommand::dst, "PlayerName"),
//             CommandParameterData::makeMandatory(&MoneyCommand::moneynum, "num")
//         );
//         registry->registerOverload<MoneyCommand>(
//             "money",
//             CommandParameterData::makeMandatory<CommandParameterDataType::Enum>(
//                 &MoneyCommand::op,
//                 "optional",
//                 "MoneyOP3"
//             ),
//             CommandParameterData::makeOptional(&MoneyCommand::difftime, "time", &MoneyCommand::difftime_isSet)
//         );
//     }
// };

// class MoneySCommand : public Command {
//     enum MoneyOP : int {
//         query  = 1,
//         hist   = 2,
//         pay    = 3,
//         set    = 4,
//         add    = 5,
//         reduce = 6,
//     } op;
//     CommandSelector<Player> player;
//     bool                    dst_isSet;
//     bool                    difftime_isSet;
//     int                     moneynum;
//     int                     difftime;

// public:
//     void execute(CommandOrigin const& ori, CommandOutput& outp) const {
//         std::vector<std::string>   dstxuidlist;
//         std::optional<std::string> dstxuid;
//         std::string                myuid;
//         switch (op) {
//         case query:
//         case hist:
//             if (dst_isSet) {
//                 if (ori.getPermissionsLevel() > CommandPermissionLevel::Any) {
//                     if (!player.results(ori).empty()) {
//                         dstxuid = player.results(ori).begin().operator*()->getXuid();
//                     }
//                 } else {
//                     outp.error("You don't have permission to do this");
//                     return;
//                 }
//             } else {
//                 if (ori.getOriginType() != CommandOriginType::Player) {
//                     outp.error(tr("money.dontuseinconsole"));
//                     return;
//                 }
//                 dstxuid = ((Player*)ori.getEntity())->getXuid();
//             }
//             if (dstxuid->empty()) {
//                 outp.error(tr("money.no.target"));
//                 return;
//             }
//             break;
//         case pay:
//             if (ori.getOriginType() != CommandOriginType::Player) {
//                 outp.error(tr("money.dontuseinconsole"));
//                 return;
//             }
//             if (player.results(ori).empty()) {
//                 outp.error(tr("money.no.target"));
//                 return;
//             }
//             for (auto resu : player.results(ori)) {
//                 dstxuid = resu->getXuid();
//                 if (dstxuid->empty()) {
//                     outp.error(tr("money.no.target"));
//                     return;
//                 }
//             }
//             break;
//         case set:
//         case add:
//         case reduce:
//             if (player.results(ori).empty()) {
//                 outp.error(tr("money.no.target"));
//                 return;
//             }
//             for (auto resu : player.results(ori)) {
//                 dstxuidlist.push_back(resu->getXuid());
//             }
//             if (dstxuidlist.empty()) {
//                 outp.error(tr("money.no.target"));
//                 return;
//             }
//             break;
//         }
//         switch (op) {
//         case query:
//             outp.addMessage(
//                 "Balance: " + std::to_string(LLMoney_Get(dstxuid.value())),
//                 {},
//                 CommandOutputMessageType::Success
//             );
//             break;
//         case hist:
//             outp.addMessage(LLMoney_GetHist(dstxuid.value()), {}, CommandOutputMessageType::Success);
//             break;
//         case pay:
//             if (moneynum <= 0) {
//                 outp.error(tr("money.invalid.arg"));
//                 return;
//             }
//             myuid = ((Player*)ori.getEntity())->getXuid();
//             if (myuid.empty()) {
//                 outp.error(tr("money.no.target"));
//                 return;
//             }
//             if (myuid == dstxuid.value()) {
//                 outp.error(tr("money.payyourself"));
//                 return;
//             }
//             if (LLMoney_Trans(myuid, dstxuid.value(), moneynum, "money pay")) {
//                 long long fee = (long long)(moneynum * Settings::pay_tax);
//                 if (fee) LLMoney_Trans(dstxuid.value(), "", fee, "money pay fee");
//                 outp.success(tr("money.pay.succ"));
//             } else {
//                 outp.error(tr("money.not.enough"));
//             }
//             break;
//         case set: {
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error(tr("money.no.perm"));
//                 return;
//             }
//             bool su = 0;
//             for (auto i : dstxuidlist) {
//                 if (LLMoney_Set(i, moneynum)) {
//                     su = 1;
//                 }
//             }
//             if (su) {
//                 outp.success(tr("money.set.succ"));
//             } else {
//                 outp.error(tr("money.invalid.arg"));
//             }
//             break;
//         }
//         case add: {
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error(tr("money.no.perm"));
//                 return;
//             }
//             bool su = 0;
//             for (auto i : dstxuidlist) {
//                 if (LLMoney_Add(i, moneynum)) {
//                     su = 1;
//                 }
//             }
//             if (su) {
//                 outp.success(tr("money.add.succ"));
//             } else {
//                 outp.error(tr("money.invalid.arg"));
//             }
//             break;
//         }
//         case reduce: {
//             if ((int)ori.getPermissionsLevel() < 1) {
//                 outp.error(tr("money.no.perm"));
//                 return;
//             }
//             bool su = 0;
//             for (auto i : dstxuidlist) {
//                 if (LLMoney_Reduce(i, moneynum)) {
//                     su = 1;
//                 }
//             }
//             if (su) {
//                 outp.success(tr("money.reduce.succ"));
//             } else {
//                 outp.error(tr("money.invalid.arg"));
//             }
//             break;
//         }
//         }
//         return;
//     }

//     static void setup(CommandRegistry* registry) {
//         // registerCommand
//         registry->registerCommand(
//             "money_s",
//             "Economy system(Selector)",
//             CommandPermissionLevel::Any,
//             {(CommandFlagValue)0},
//             {(CommandFlagValue)0x80}
//         );

//         // addEnum
//         registry->addEnum<MoneyOP>(
//             "MoneyOP1",
//             {
//                 {"query", MoneyOP::query},
//                 {"hist",  MoneyOP::hist }
//         }
//         );
//         registry->addEnum<MoneyOP>(
//             "MoneyOP2",
//             {
//                 {"add",    MoneyOP::add   },
//                 {"pay",    MoneyOP::pay   },
//                 {"reduce", MoneyOP::reduce},
//                 {"set",    MoneyOP::set   }
//         }
//         );

//         // registerOverload
//         registry->registerOverload<MoneySCommand>(
//             "money_s",
//             CommandParameterData::makeMandatory<CommandParameterDataType::Enum>(
//                 &MoneySCommand::op,
//                 "optional",
//                 "MoneyOP1"
//             ),
//             CommandParameterData::makeOptional(&MoneySCommand::player, "PlayerName", &MoneySCommand::dst_isSet)
//         );

//         registry->registerOverload<MoneySCommand>(
//             "money_s",
//             CommandParameterData::makeMandatory<CommandParameterDataType::Enum>(
//                 &MoneySCommand::op,
//                 "optional",
//                 "MoneyOP2"
//             ),
//             CommandParameterData::makeMandatory(&MoneySCommand::player, "PlayerName"),
//             CommandParameterData::makeMandatory(&MoneySCommand::moneynum, "num")
//         );
//     }
// };

void loadCfg() {
    // config
    if (std::filesystem::exists("plugins/LegacyMoney/money.json")) {
        try {
            Settings::LoadConfigFromJson("plugins/LegacyMoney/money.json");
        } catch (std::exception& e) {
            logger->error("Configuration file is Invalid, Error: {}", e.what());
        } catch (...) {
            logger->error("Configuration file is Invalid");
        }
    } else {
        Settings::WriteDefaultConfig("plugins/LegacyMoney/money.json");
    }
}

struct QueryMoney {
    std::string playerName;
};

struct QueryMoneySelector {
    CommandSelector<Player> player;
};

enum MoneyOperation : int { add = 0, reduce = 1, set = 2, pay = 3 };

struct OperateMoney {
    MoneyOperation operation;
    std::string    playerName;
    long long      amount;
};

struct OperateMoneySelector {
    MoneyOperation          operation;
    CommandSelector<Player> player;
    long long               amount;
};

void RegisterMoneyCommands() {
    using ll::command::CommandRegistrar;
    auto& command =
        ll::command::CommandRegistrar::getInstance().getOrCreateCommand("money", "LegacyMoney's main command"_tr());
    command.overload<QueryMoney>()
        .text("query")
        .optional("playerName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, QueryMoney const& param, Command const& cmd) {
            if (param.playerName.empty()) {
                if (origin.getOriginType() != CommandOriginType::Player) {
                    output.error("Please type the name of player you want to query"_tr());
                } else {
                    Actor* actor = origin.getEntity();
                    if (actor) {
                        output.success("Your balance: "_tr().append(
                            std::to_string(LLMoney_Get(static_cast<Player*>(actor)->getXuid()))
                        ));
                    } else {
                        output.error("No player found"_tr());
                    }
                }
            } else {
                if (origin.getPermissionsLevel() >= CommandPermissionLevel::GameDirectors) {
                    auto info = ll::service::PlayerInfo::getInstance().fromName(param.playerName);
                    if (info.has_value()) {
                        output.success(
                            param.playerName + "'s balance: "_tr().append(std::to_string(LLMoney_Get(info->xuid)))
                        );

                    } else {
                        output.error("No player found"_tr());
                    }
                } else {
                    output.error("You don't have permission to do this"_tr());
                }
            }
        }>();
    command.overload<QueryMoneySelector>()
        .text("querys")
        .required("player")
        .execute<
            [&](CommandOrigin const& origin, CommandOutput& output, QueryMoneySelector const& param, Command const& cmd
            ) {
                if (origin.getPermissionsLevel() >= CommandPermissionLevel::GameDirectors) {
                    if (param.player.results(origin).size()) {
                        auto it = param.player.results(origin).data;
                        for (Player* player : *it) {
                            if (player) {
                                output.success(
                                    player->getRealName()
                                    + "'s balance: "_tr().append(std::to_string(LLMoney_Get(player->getXuid())))
                                );
                            }
                        }
                    } else {
                        output.error("No player found"_tr());
                    }
                } else {
                    output.error("You don't have permission to do this"_tr());
                }
            }>();
    command.overload<OperateMoney>()
        .required("operation")
        .required("playerName")
        .required("amount")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, OperateMoney const& param, Command const& cmd
                 ) {
            switch (param.operation) {
            case MoneyOperation::add:
                break;
            case MoneyOperation::reduce:
                break;
            case MoneyOperation::set:
                break;
            case MoneyOperation::pay:
                break;
            default:
                break;
            }
        }>();
}

namespace legacymoney {

Plugin::Plugin(ll::plugin::NativePlugin& self) : mSelf(self) {
    logger = &mSelf.getLogger();
    mSelf.getLogger().info("Loaded!");
    loadCfg();
    if (!initDB()) {
        return;
    }
    ll::i18n::getInstance() = std::make_unique<ll::i18n::SingleFileI18N>(
        ll::i18n::SingleFileI18N("plugins/LegacyMoney/language.json", Settings::language, defaultLangData)
    );
}

bool Plugin::enable() {
    if (Settings::enable_commands) {
        // CommandRegistry& reg = ll::service::getCommandRegistry().get();
        // MoneyCommand::setup(&reg);
        // MoneySCommand::setup(&reg);
        RegisterMoneyCommands();
    }
    return true;
}

bool Plugin::disable() { return true; }

} // namespace legacymoney
