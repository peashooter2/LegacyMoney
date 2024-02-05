#pragma once
#include <ll/api/i18n/I18n.h>

static const ll::i18n::SingleFileI18N::LangData defaultLangData = {
    {"en",
     {{"money.no.target", "Can not find target player."},
      {"money.invalid.arg", "Invalid command argument, or unknown error."},
      {"money.succ", "Command successfully executed."},
      {"money.not.enough", "You have insufficient balance."},
      {"money.no.perm", "You do not have the required permissions to execute this command."},
      {"money.pay.succ", "Transfer successful."},
      {"money.add.succ", "Balance successfully increased."},
      {"money.reduce.succ", "Balance successfully reduced."},
      {"money.set.succ", "Balance successfully set."},
      {"money.dontuseinconsole", "Don't use this command in console."},
      {"money.payyourself", "You can't pay to yourself."}}},
    {"ru",
     {{"money.no.target", "Не удаётся найти игрока."},
      {"money.invalid.arg", "Неправильный аргумент команды, или неизвестная ошибка."},
      {"money.succ", "Команда выполнена успешно."},
      {"money.not.enough", "У вас недостаточный баланс."},
      {"money.no.perm", "У вас нет прав для использования этой команды."},
      {"money.pay.succ", "Переведено успешно."},
      {"money.add.succ", "Баланс успешно увеличен."},
      {"money.reduce.succ", "Баланс успешно уменьшен."},
      {"money.set.succ", "Баланс успешно установлен."},
      {"money.dontuseinconsole", "Не используйте эту команду в консоли."},
      {"money.payyourself", "You can't pay to yourself."}}},
    {"zh_CN",
     {{"money.no.target", "找不到目标"},
      {"money.invalid.arg", "参数错误或未知错误"},
      {"money.succ", "执行成功"},
      {"money.not.enough", "金钱不足"},
      {"money.no.perm", "你无权执行"},
      {"money.pay.succ", "支付成功"},
      {"money.add.succ", "添加成功"},
      {"money.reduce.succ", "移除成功"},
      {"money.set.succ", "设置成功"},
      {"money.dontuseinconsole", "禁止在控制台使用此指令"},
      {"money.payyourself", "你不能给自己付钱"}}  },
    {"zh_TW",
     {{"money.no.target", "找不到目標"},
      {"money.invalid.arg", "參數錯誤或未知錯誤"},
      {"money.succ", "執行成功"},
      {"money.not.enough", "金錢不足"},
      {"money.no.perm", "你沒有權限使用此指令!"},
      {"money.pay.succ", "支付成功"},
      {"money.add.succ", "添加成功"},
      {"money.reduce.succ", "移除成功"},
      {"money.set.succ", "設置成功"},
      {"money.dontuseinconsole", "禁止在控制台使用此指令"},
      {"money.payyourself", "你不能給自己付錢"}}  }
};
