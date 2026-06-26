//
// Created by soberga on 26.06.2026.
//

#include "DialogLoader.h"
#include "tileson.hpp"
#include <fstream>
#include <sstream>

namespace ggj
{
    bool DialogLoader::load(const std::filesystem::path& filePath, DialogManager& manager) const
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return false;

        std::ostringstream ss;
        ss << file.rdbuf();

        std::string err;
        json11::Json root = json11::Json::parse(ss.str(), err);
        if (!err.empty() || !root.is_object())
            return false;

        const json11::Json& dialogs = root["dialogs"];
        if (!dialogs.is_array())
            return false;

        for (const json11::Json& item : dialogs.array_items())
        {
            if (!item.is_object())
                continue;

            DialogItem dialogItem;
            dialogItem.dialogId = item["id"].string_value();
            dialogItem.text = item["text"].string_value();
            dialogItem.affirmCondition = item["affirmCondition"].string_value();

            const json11::Json& choices = item["choices"];
            if (choices.is_array())
            {
                for (const json11::Json& choice : choices.array_items())
                {
                    if (!choice.is_object())
                        continue;

                    DialogChoice dialogChoice;
                    dialogChoice.text = choice["text"].string_value();
                    dialogChoice.nextDialogId = choice["nextDialogId"].string_value();
                    dialogChoice.condition = choice["condition"].string_value();
                    dialogItem.choices.push_back(std::move(dialogChoice));
                }
            }

            manager.addDialog(dialogItem);
        }

        return true;
    }
} // namespace ggj
