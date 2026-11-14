//
// Created by robin on 27.10.22.
//

#ifndef GAME_DEV_TEMPLATE_RESOURCENAME_H
#define GAME_DEV_TEMPLATE_RESOURCENAME_H

#include <cstdint>

namespace ggj
{
    /*!
     * ResourceName should be composed by something like this:
     * <ResourceType><ResourceId>.
     * Example:
     * GraphicsPlayer - Where <ResourceType>=Graphics and <ResourceId>=Player.
     *
     * ResourceTypes:
     * Graphics
     * Sound
     *
     */
    enum class ResourceName : uint16_t
    {
        None = 0,
        GraphicsPlayer = 1,
        MapProject = 7,
        GraphicsMagicCliffsTileset = 10,
        GraphicsEnemyBat = 11,
        GraphicsCrate = 12,
        GraphicsNinjaSmiley = 13,
        GraphicsJumpolini = 14,
        GraphicsGem1 = 15,
        GraphicsGem2 = 16,
        GraphicsGem3 = 17,
        GraphicsGem4 = 18,

        GraphicsBackground01MountainsGrass = 118,
        GraphicsBackground02MountainsDesert = 119,
        GraphicsBackground03MountainsSnow = 120,
        GraphicsBackground04CityNight = 121,
        GraphicsBackground05Graveyard = 122,
        GraphicsCharacterHudChar01 = 123,
        GraphicsCharacterHudChar02 = 124,
        GraphicsCharacterHudChar03 = 125,
        GraphicsCharacterHudChar04 = 126,
        GraphicsCharacterHudChar05 = 127,
        GraphicsCharacterHudChar06 = 128,
        GraphicsCharacterHudChar07 = 129,
        GraphicsCharacterHudChar08 = 130,
        GraphicsCharacterHudChar09 = 131,
        GraphicsCharacterHudChar10 = 132,
        GraphicsCharacterHudChar11 = 133,
        GraphicsCharacterHudChar12 = 134,
        GraphicsCharacterHudChar13 = 135,
        GraphicsCharacterHudChar14 = 136,
        GraphicsCharacterHudChar15 = 137,
        GraphicsCharacterHudChar16 = 138,
        GraphicsCharacterHudChar17 = 139,
        GraphicsCharacterHudChar18 = 140,
        GraphicsCharacterHudChar19 = 141,
        GraphicsCharacterHudChar20 = 142,
        GraphicsCharacterHudChar21 = 143,
        GraphicsCharacterHudChar22 = 144,
        GraphicsCharacterHudChar23 = 145,
        GraphicsCharacterHudChar24 = 146,
        GraphicsCharacterHudChar25 = 147,
        GraphicsEnemyGhostFlying = 148,
        GraphicsEnemySpiderWalk = 152,
        GraphicsEnemyThingWalk = 156,
        GraphicsExplosion1 = 160,
        GraphicsExplosion2 = 161,
        GraphicsExplosion3 = 162,
        GraphicsExplosion4 = 163,
        GraphicsExplosion5 = 164,
        GraphicsExplosion6 = 165,
        GraphicsHudUiBigPieces = 166,
        GraphicsMeleeSwoosh = 167,
        GraphicsPlayerSpritemapGreenpants = 168,
        GraphicsPlayerSpritemapGreypants = 169,
        GraphicsPlayerSpritemapBluepants = 170,
        GraphicsPlayerSpritemapRedpants = 171,
        GraphicsPowerupsRpgIcons = 172,
        GraphicsShootFire = 173,
        GraphicsTilesJungleTerrain = 175,
        GraphicsTilesObjects = 176,
        GraphicsCoin = 177,
        GraphicsPortal = 178,

        TopDownForest = 200,
        RoguePlayer = 201,
        WizardPlayer = 202,
        MageBall = 203,
        DungeonPrison = 204,
        DungeonPrisonProps = 205,

        GraphicsNpcBlacksmith = 206,
        GraphicsNpcScholar = 207,
        GraphicsNpcTreant = 208,
        GraphicsNpcButcher = 209,
        GraphicsNpcMage = 210,

        HudRpg = 300,
        HudRpgSubset = 301,
        HudRpgRoguePortrait = 302,
        HudRpgWizardPortrait = 303,

        CharacterSelectRoguePortrait = 330,
        CharacterSelectWizardPortrait = 331,

        GraphicsMainMenuBackground = 340,

        GraphicsEnemyClaude    = 350,
        GraphicsEnemyClaudling = 351,
        GraphicsEnemyBomb = 352,
        GraphicsEnemyBombExplosion = 353,

        FontDroidSans = 400,
        FontDroidSansBold = 401,
        FontPixantiqua = 402,

        DialogNpc = 500,

        ConfigOptions = 550,

        ShaderPaletteSwap = 560,
        ShaderTint = 561,

        SoundCoinPickup = 600,
        SoundEnemyDie = 601,
        SoundPortalUse = 602,
        SoundPlayerAttack = 603,
        SoundDialogMumble = 604,
        SoundDialogTalking1 = 605,
        SoundDialogTalking2 = 606,
        SoundDialogTalking3 = 607,
        SoundDialogTalking4 = 608,
        SoundDialogTalking5 = 609,
        SoundDialogTalking6 = 610,
        SoundDialogTalking7 = 611,
        SoundLevelUp = 612,
        SoundPlayerHurt = 613,
        SoundPlayerDeath = 614,
        SoundPlayerRespawn = 615,
        SoundPortalOpen = 616,
        SoundExtraLife = 617,
        SoundWizardShoot = 618,

        MusicForest1 = 700,
        MusicForest2 = 701,
        MusicDungeon1 = 702,
        MusicDungeon2 = 703,
        MusicBossBattle1 = 704,
        MusicBossBattle2 = 705,
        MusicBossBattle3 = 706,
        MusicBossBattle4 = 707,
        MusicMainMenu1 = 708,
        MusicMainMenu2 = 709,

        MusicGlitch1 = 710,
        MusicGlitch2 = 711,

        MusicVictory1 = 712,
        MusicVictory2 = 713
    };
} // namespace ggj

#endif //GAME_DEV_TEMPLATE_RESOURCENAME_H
