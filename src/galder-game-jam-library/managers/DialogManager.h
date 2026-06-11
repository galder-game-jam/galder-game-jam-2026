//
// Created by soberga on 23.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H
#define GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H


#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct DialogChoice {
    std::string text;
    std::string nextDialogId;
};

struct DialogItem {
    std::string dialogId;
    std::string text;
    std::vector<DialogChoice> choices;
};

class DialogManager {
public:
    void addDialog(const DialogItem& dialog);
    DialogItem* getDialog(const std::string& id);
    const DialogItem* getDialog(const std::string& id) const;
    DialogItem* choose(const DialogItem& dialog, size_t choiceIndex);
    bool hasDialog(const std::string& id) const;
private:
    std::unordered_map<std::string, DialogItem> dialogs;
};

#endif //GALDER_GAME_JAM_2025_PROJECT_DIALOGMANAGER_H