//
// Created by soberga on 23.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H
#define GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct DialogChoice
{
    std::string text;
    std::string nextDialogId;
    std::string condition; ///< Empty means always visible; non-empty requires the condition to be set.
};

struct DialogItem
{
    std::string dialogId;
    std::string text;
    std::vector<DialogChoice> choices;
    std::string affirmCondition; ///< Condition recorded as true when this dialog is displayed.
};

class DialogManager
{
    public:
    void addDialog(const DialogItem& dialog);
    DialogItem* getDialog(const std::string& id);
    const DialogItem* getDialog(const std::string& id) const;
    DialogItem* choose(const DialogItem& dialog, size_t choiceIndex);
    bool hasDialog(const std::string& id) const;
    void setCondition(const std::string& condition);
    [[nodiscard]] bool hasCondition(const std::string& condition) const;
    [[nodiscard]] std::vector<DialogChoice> getVisibleChoices(const DialogItem& item) const;

    private:
    std::unordered_map<std::string, DialogItem> dialogs;
    std::unordered_set<std::string> m_conditions;
};

#endif //GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H