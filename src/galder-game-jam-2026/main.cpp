// Boost.DI caps injected constructors at 10 parameters by default. ManagerDiCollection exceeds
// that (more so in debug builds with the extra DebugState), so raise the limit before di.hpp.
#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 16

#include "../galder-game-jam-library/GalderGameJamLibrary.h"
#include "di.hpp"
#include "fmt/color.h"
#include "raylib-cpp.hpp"

#ifdef GAME_DEV_DEBUG
    #include "../galder-game-jam-library/managers/DebugMenu.hpp"
#endif

auto buildInjector(ggj::ILogger& logger, raylib::Window& window)
{
    auto injector = boost::di::make_injector(
        boost::di::bind<ggj::IGameManager>().to<ggj::GameManager>(), boost::di::bind<ggj::IWorldManager>().to<ggj::WorldManager>(),
        boost::di::bind<ggj::ITextureManager<ggj::TextureName, raylib::Texture>>().to<ggj::TextureManager>(), boost::di::bind<ggj::IMapper>().to<ggj::Mapper>(),
        // Singleton: the keyboard capture flag (chat) must be shared by every consumer.
        boost::di::bind<ggj::IInputManager<ggj::KeyboardKey>>().to<ggj::InputManager>().in(boost::di::singleton),
        boost::di::bind<ggj::IResourceManager<ggj::ResourceName>>().to<ggj::ResourceManager<ggj::ResourceName>>(),
        boost::di::bind<ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>>().to<ggj::AnimationManager>(),
        boost::di::bind<ggj::IUserDataManager<ggj::PhysicsObject*>>().to<ggj::UserDataManager>(),
        boost::di::bind<ggj::IOptionsManager>().to<ggj::OptionsManager>().in(boost::di::singleton), boost::di::bind<ggj::ISoundManager>().to<ggj::SoundManager>(),

        boost::di::bind<ggj::IExecutableInfo>().to<ggj::ExecutableInfo>(), boost::di::bind<ggj::IDebugManager>().to<ggj::DebugManager>(), boost::di::bind<ggj::CollisionManager>(),
        boost::di::bind<ggj::IIpAddressResolver>().to<ggj::IpAddressResolver>(), boost::di::bind<ggj::INetworkManager>().to<ggj::NetworkManager>().in(boost::di::singleton),
        boost::di::bind<ggj::PlayerPersistentState>().in(boost::di::singleton), boost::di::bind<ggj::IPaletteSwapShader>().to<ggj::PaletteSwapShader>().in(boost::di::singleton),
        boost::di::bind<ggj::IMaskRecolorShader>().to<ggj::MaskRecolorShader>().in(boost::di::singleton),
#ifdef GAME_DEV_DEBUG
        boost::di::bind<ggj::DebugState>().in(boost::di::singleton),
#endif
        boost::di::bind<raylib::Window>().to(window), boost::di::bind<ggj::ILogger>().to(logger));

    return injector;
}

struct App
{
    ggj::ILogger& logger;
    ggj::IExecutableInfo& executableInfo;
    ggj::IGameManager& game;
    ggj::IResourceManager<ggj::ResourceName>& resources;
    ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animations;
    ggj::IOptionsManager& options;
    ggj::ISoundManager& sounds;
    ggj::IPaletteSwapShader& paletteSwapShader;
    ggj::IMaskRecolorShader& maskRecolorShader;
    raylib::Window& window;

    bool initialize()
    {
        resources.initialize();
        options.initialize();
        animations.initialize();
        sounds.initialize();
        // Not fatal: without the shaders the players and portraits are just drawn in their default colours.
        paletteSwapShader.initialize();
        maskRecolorShader.initialize();

        if (!game.initialize())
            return false;

        return true;
    }
};

int main(int argc, char** argv)
{
    int screenWidth = 800;  // HalfWidth: 400
    int screenHeight = 600; // HalfHeight: 300
    raylib::Window window { screenWidth, screenHeight, "Raylib Template" };
    window.SetState(FLAG_WINDOW_RESIZABLE);

    ggj::ConsoleLogger l = ggj::ConsoleLogger(ggj::LogLevel::Debug);

    //Handle dependency injection
    auto injector = buildInjector(l, window);
    App app = injector.create<App>();
    ggj::ILogger& logger = app.logger;
    app.window.SetTitle("Galder Game Jam 2026");

    logger.information(fmt::format("Content path: {0}", app.executableInfo.getContentRootDirectory().string()));
    //Initialize everything
    if (!app.initialize())
        app.logger.critical("App initialization failed!");

    SetTargetFPS(60);
    ggj::Timer timer;
    while (!window.ShouldClose())
    {
        float timeDelta = timer.secondsElapsed();
        app.game.update(timeDelta);
        timer.restart();
        app.game.draw();
    }

    return 0;
}
