//
// Created by robin on 26.10.22.
//

#ifndef GAME_DEV_TEMPLATE_GALDERGAMEJAMLIBRARY_H
#define GAME_DEV_TEMPLATE_GALDERGAMEJAMLIBRARY_H

#include "../gamedev-base/GameDevBase.h"

//Interfaces
#include "interfaces/system/IMapper.h"
#include "interfaces/graphics/IPaletteSwapShader.h"
#include "interfaces/graphics/ITintShader.h"
#include "interfaces/graphics/IMaskRecolorShader.h"

//Data
#include "data/AudioOptions.hpp"
#include "data/PlayerNetworkData.hpp"
#include "data/ServerNetworkData.hpp"

//Enums
#include "enums/AnimationName.h"
#include "enums/KeyboardKey.h"
#include "enums/MusicName.h"
#include "enums/PlayerState.h"
#include "enums/SoundName.h"

//Graphics
#include "graphics/CharacterPalette.h"
#include "graphics/PaletteSwapShader.h"
#include "graphics/TintShader.h"
#include "graphics/MaskRecolorShader.h"

//Game
#include "game/Player.h"
#include "game/Player2.h"

//Enemies
#include "game/enemies/Bat.h"
#include "game/enemies/Snake.h"
#include "game/enemies/Spider.h"
#include "game/enemies/Thing.h"
#include "game/enemies/Ghost.h"

// Collectibles
#include "game/collectibles/Coin.h"

//interactables
#include "game/interactables/Portal.h"
#include "game/interactables/PowerUp.h"

//Hud
#include "game/Hud.h"

//Managers
#include "managers/AnimationManager.h"
#include "managers/CollisionManager.h"
#include "managers/DummyDebugManager.hpp"
#include "managers/DebugManager.h"
#include "managers/GameManager.h"
#include "managers/OptionsManager.h"
#include "managers/ResourceManager.hpp"
#include "managers/InputManager.h"
#include "managers/SoundManager.h"
#include "managers/TextureManager.h"
#include "managers/UserDataManager.hpp"
#include "managers/ManagerDiCollection.hpp"

//Dialog
#include "game/Dialog.h"
#include "system/CharacterSelectScreen.h"
#include "system/MainMenuScreen.h"
#include "system/NetplayScreen.h"
#include "system/OptionsScreen.h"
#include "system/InGameMenuScreen.h"
#include "system/TalentScreen.h"

//Character select

#include "managers/WorldManager.h"

//Network
#include "network/GalderClient.hpp"
#include "network/GalderServer.hpp"
#include "interfaces/manager/INetworkManager.h"
#include "managers/NetworkManager.h"

//System
#include "system/World.h"
#include "system/Mapper.hpp"


#endif //GAME_DEV_TEMPLATE_GALDERGAMEJAMLIBRARY_H
