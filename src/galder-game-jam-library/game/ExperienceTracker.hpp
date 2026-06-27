//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_EXPERIENCETRACKER_HPP
#define GALDER_GAME_JAM_2026_EXPERIENCETRACKER_HPP

#include "../data/ExperienceConfig.hpp"

namespace ggj
{
    /*!
     * Tracks a player's experience points and current level.
     *
     * XP required to advance from level N to N+1:
     *   threshold(N) = baseXpPerLevel * 1.5 * N
     *
     * On level-up the threshold is subtracted from current XP so the
     * residual carries forward to the next level.
     */
    class ExperienceTracker
    {
        public:
            ExperienceTracker() = default;

            /*!
             * Award XP and trigger any resulting level-ups.
             *
             * @param amount Non-negative amount of XP to add.
             * @param cfg    The experience configuration.
             */
            void addXp(int amount, const ExperienceConfig& cfg)
            {
                m_totalXp   += amount;
                m_currentXp += amount;
                checkLevelUp(cfg);
            }

            void restore(int level, int currentXp, int totalXp)
            {
                m_level     = level;
                m_currentXp = currentXp;
                m_totalXp   = totalXp;
            }

            [[nodiscard]] int getLevel()    const { return m_level;    }
            [[nodiscard]] int getCurrentXp() const { return m_currentXp; }
            [[nodiscard]] int getTotalXp()  const { return m_totalXp;  }

            /*!
             * @return XP needed to advance from the current level to the next.
             */
            [[nodiscard]] int getXpToNextLevel(const ExperienceConfig& cfg) const
            {
                return xpThreshold(m_level, cfg);
            }

        private:
            void checkLevelUp(const ExperienceConfig& cfg)
            {
                while (m_level < cfg.maxLevel)
                {
                    int needed = xpThreshold(m_level, cfg);
                    if (m_currentXp >= needed)
                    {
                        m_currentXp -= needed;
                        ++m_level;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            static int xpThreshold(int level, const ExperienceConfig& cfg)
            {
                return static_cast<int>(cfg.baseXpPerLevel * 1.5f * level);
            }

            int m_level     { 1 };
            int m_currentXp { 0 };
            int m_totalXp   { 0 };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_EXPERIENCETRACKER_HPP
