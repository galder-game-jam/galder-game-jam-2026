//
// Created by soberga on 26.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_DIALOGLOADER_H
#define GALDER_GAME_JAM_2026_DIALOGLOADER_H

#pragma once
#include <filesystem>
#include "DialogManager.h"

namespace ggj
{
    /*!
     * Loads dialog entries from a JSON file and populates a DialogManager.
     *
     * Expected JSON schema:
     * @code
     * {
     *   "groups": [
     *     {
     *       "npc": "npcName",
     *       "dialogs": [
     *         {
     *           "id": "someId",
     *           "text": "Dialog text here",
     *           "affirmCondition": "optionalConditionSetWhenDisplayed",
     *           "choices": [
     *             {
     *               "text": "Option A",
     *               "nextDialogId": "someOtherId",
     *               "condition": "optionalRequiredCondition"
     *             }
     *           ]
     *         }
     *       ]
     *     }
     *   ]
     * }
     * @endcode
     *
     * Conditions are set either from dialog (@c affirmCondition) or from game state
     * (e.g. @c levelComplete, @c battleHardened, @c leveledUp).
     * A choice is visible when its @c condition is empty or has been set.
     */
    class DialogLoader
    {
        public:
        /*!
             * Parses the JSON file at filePath and adds all dialogs to manager.
             *
             * @param filePath Path to the JSON dialog file.
             * @param manager  DialogManager to populate.
             * @return True on success, false if the file cannot be opened or parsed.
             */
        [[nodiscard]] bool load(const std::filesystem::path& filePath, DialogManager& manager) const;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_DIALOGLOADER_H
