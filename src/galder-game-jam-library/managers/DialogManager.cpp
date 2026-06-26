//
// Created by soberga on 23.11.2025.
//

#include "DialogManager.h"

void DialogManager::addDialog(const DialogItem& dialog)
{
    dialogs[dialog.dialogId] = dialog;
}

DialogItem* DialogManager::getDialog(const std::string& id)
{
    auto it = dialogs.find(id);
    if (it != dialogs.end())
        return &it->second;
    return nullptr;
}

const DialogItem* DialogManager::getDialog(const std::string& id) const
{
    auto it = dialogs.find(id);
    if (it != dialogs.end())
        return &it->second;
    return nullptr;
}

DialogItem* DialogManager::choose(const DialogItem& dialog, size_t choiceIndex)
{
    if (choiceIndex >= dialog.choices.size())
        return nullptr;

    const std::string& nextId = dialog.choices[choiceIndex].nextDialogId;
    return getDialog(nextId);
}

bool DialogManager::hasDialog(const std::string& id) const
{
    return dialogs.find(id) != dialogs.end();
}

void DialogManager::setCondition(const std::string& condition)
{
    m_conditions.insert(condition);
}

bool DialogManager::hasCondition(const std::string& condition) const
{
    return m_conditions.count(condition) > 0;
}

std::vector<DialogChoice> DialogManager::getVisibleChoices(const DialogItem& item) const
{
    std::vector<DialogChoice> visible;
    for (const DialogChoice& choice : item.choices)
    {
        if (choice.condition.empty() || hasCondition(choice.condition))
            visible.push_back(choice);
    }
    return visible;
}