project(galder-game-jam-library VERSION 1.0.0)

add_library(galder-game-jam-library GalderGameJamLibrary.h managers/GameManager.cpp managers/GameManager.h
        managers/ResourceManager.hpp managers/InputManager.cpp managers/InputManager.h enums/KeyboardKey.h
        managers/WorldManager.cpp managers/WorldManager.h system/World.cpp system/World.h
        enums/TextureName.h system/Mapper.hpp managers/TextureManager.h managers/TextureManager.cpp interfaces/system/IMapper.h
        system/WorldLayer.cpp system/WorldLayer.h graphics/Sprite.cpp graphics/Sprite.h graphics/PhysicsObject.cpp
        graphics/PhysicsObject.h enums/PhysicsShape.h graphics/PhysicsSprite.cpp graphics/PhysicsSprite.h
        managers/CollisionManager.cpp managers/CollisionManager.h managers/DummyDebugManager.hpp managers/DebugMenu.hpp
        managers/DebugManager.cpp managers/DebugManager.h game/Player.cpp game/Player.h game/Player2.cpp game/Player2.h
        managers/AnimationManager.cpp managers/AnimationManager.h graphics/Animation.cpp
        graphics/Animation.h graphics/Frame.hpp enums/AnimationName.h enums/PlayerState.h
        system/UserData.hpp enums/ObjectType.h interfaces/manager/IUserDataManager.h managers/UserDataManager.hpp
        game/enemies/Bat.cpp game/enemies/Bat.h system/ObjectGeneratorData.hpp
        game/enemies/Snake.cpp game/enemies/Snake.h game/collectibles/Coin.cpp game/collectibles/Coin.h
        game/enemies/Spider.cpp game/enemies/Spider.h game/enemies/Thing.cpp game/enemies/Thing.h
        game/enemies/Ghost.cpp game/enemies/Ghost.h game/interactables/Portal.cpp game/interactables/Portal.h
        game/enemies/ClaudeEnemy.cpp game/enemies/ClaudeEnemy.h
        game/enemies/Claudling.cpp game/enemies/Claudling.h
        game/enemies/Bomb.cpp game/enemies/Bomb.h
        game/enemies/TreantEnemy.cpp game/enemies/TreantEnemy.h
        game/interactables/PowerUp.cpp game/interactables/PowerUp.h game/Hitbox.hpp
        data/PlayerNetworkData.hpp
        data/ServerNetworkData.hpp
        data/ClientMessage.hpp
        data/ServerMessage.hpp
        data/RemotePlayer.hpp
        data/ConnectedPlayer.hpp
        data/DiscoveredServer.hpp
        data/EnemyNetworkState.hpp
        data/ProjectileEvent.hpp
        data/ClaudlingSpawnEvent.hpp
        data/EnemyDamageEvent.hpp
        data/BombSpawnEvent.hpp
        data/RootSpawnEvent.hpp
        data/ClaudeBombConfig.hpp
        interfaces/game/IPlayerLocator.h
        enums/NetMessageType.h
        enums/NetworkRole.h
        interfaces/manager/INetworkManager.h
        network/GalderClient.hpp
        network/GalderServer.hpp
        network/LanBeacon.h
        network/LanBeacon.cpp
        managers/NetworkManager.cpp
        managers/NetworkManager.h
        data/ChatMessage.hpp
        data/ChatEntry.hpp
        system/ChatText.hpp
        system/ChatLog.cpp
        system/ChatLog.h
        game/ChatBox.cpp
        game/ChatBox.h
        managers/ServerInfoMenu.hpp
        managers/MessagePopup.hpp
        game/projectiles/MageBall.hpp
        game/projectiles/Projectile.cpp
        game/projectiles/Projectile.h
        managers/ManagerDiCollection.hpp
        managers/OptionsManager.h
        managers/OptionsManager.cpp
        managers/SoundManager.h
        managers/SoundManager.cpp
        enums/SoundName.h
        enums/MusicName.h
        game/Hud.cpp
        game/Hud.h
        game/Minimap.cpp
        game/Minimap.h
        game/Dialog.cpp
        game/Dialog.h
        managers/DialogManager.cpp
        managers/DialogManager.h
        managers/DialogLoader.cpp
        managers/DialogLoader.h
        system/CharacterSelectScreen.cpp
        system/CharacterSelectScreen.h
        system/MainMenuScreen.cpp
        system/MainMenuScreen.h
        system/NetplayScreen.cpp
        system/NetplayScreen.h
        system/OptionsScreen.cpp
        system/OptionsScreen.h
        system/InGameMenuScreen.cpp
        system/InGameMenuScreen.h
        system/TalentScreen.cpp
        system/TalentScreen.h
        data/TalentData.hpp
        data/TalentState.hpp
        data/CharacterColors.hpp
        graphics/CharacterPalette.cpp
        graphics/CharacterPalette.h
        graphics/PaletteSwapShader.cpp
        graphics/PaletteSwapShader.h
        game/enemies/TreantRoot.cpp
        game/enemies/TreantRoot.h
        graphics/TintShader.cpp
        graphics/TintShader.h
        interfaces/graphics/IPaletteSwapShader.h
        interfaces/graphics/ITintShader.h
        interfaces/graphics/IPaletteSwapShader.h
        graphics/MaskRecolorShader.cpp
        graphics/MaskRecolorShader.h
        interfaces/graphics/IMaskRecolorShader.h)

# Link raylib_imgui so its PUBLIC compile definitions (IMGUI_DISABLE_OBSOLETE_*) reach every file
# that includes imgui.h. Without them, ImGuiIO/ImGuiStyle get a different layout here than in
# imgui.cpp, and writes to them corrupt neighbouring ImGui state.
target_link_libraries(galder-game-jam-library PUBLIC raylib_imgui)
