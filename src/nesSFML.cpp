#include <SFML/Graphics.hpp>
#include <argparse/argparse.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

namespace {

constexpr unsigned PLAY_SCALE = 3;
constexpr unsigned DEBUG_SCALE = 2;

// Native sizes of the NES image buffers.
constexpr unsigned SCREEN_WIDTH = 256;
constexpr unsigned SCREEN_HEIGHT = 240;
constexpr unsigned PATTERN_SIZE = 128;

enum class Mode { Play, Debug };

/**
 * @brief Reusable texture for one NES image, updated in place every frame.
 *
 * Allocating a new sf::Texture per frame is what tanked the frame rate.
 */
template <std::size_t Width, std::size_t Height>
class View {
  public:
    View() {
        texture.create(Width, Height);
        sprite_.setTexture(texture);
    }

    void update(const NES::Image<NES::Color, Width, Height>& img) {
        for (std::size_t y = 0; y < Height; ++y) {
            for (std::size_t x = 0; x < Width; ++x) {
                const NES::Color& c = img[y][x];
                sf::Uint8* pixel = &pixels[(y * Width + x) * 4];
                pixel[0] = c.r;
                pixel[1] = c.g;
                pixel[2] = c.b;
                pixel[3] = 0xFF;
            }
        }
        texture.update(pixels.data());
    }

    void place(float x, float y, float scale) {
        sprite_.setPosition(x, y);
        sprite_.setScale(scale, scale);
    }

    [[nodiscard]] const sf::Sprite& sprite() const { return sprite_; }

  private:
    sf::Texture texture;
    sf::Sprite sprite_;
    std::array<sf::Uint8, Width * Height * 4> pixels{};
};

}  // namespace

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("senmu - A NES Emulator");
    parser.add_argument("rom").help("Path to the ROM file to load").required();
    parser.add_argument("--skip").help("Number of frames to run/skip before displaying the window (debug mode)").default_value(0).scan<'i', int>();
    parser.parse_args(argc, argv);

    iNES::iNES rom(parser.get<std::string>("rom"));
    NES::NesSystem nes(rom);

    View<SCREEN_WIDTH, SCREEN_HEIGHT> mainView;
    View<SCREEN_WIDTH, SCREEN_HEIGHT> nametable1View;
    View<SCREEN_WIDTH, SCREEN_HEIGHT> nametable2View;
    View<PATTERN_SIZE, PATTERN_SIZE> pattern1View;
    View<PATTERN_SIZE, PATTERN_SIZE> pattern2View;

    sf::RenderWindow mainWindow(sf::VideoMode(SCREEN_WIDTH * PLAY_SCALE, SCREEN_HEIGHT * PLAY_SCALE), "NES Emulator");
    mainWindow.setFramerateLimit(60);

    std::uint8_t selectedPalette = 0x00;
    Mode mode = Mode::Play;

    // Play mode: just the game, scaled up. Debug mode: the screen at 2x on the
    // left, nametables at 1x stacked on the right, and the sprite tables at 1x
    // stacked to the right of the nametables. A small margin separates sections.
    auto applyLayout = [&] {
        constexpr float MARGIN = 8.f;
        if (mode == Mode::Play) {
            mainWindow.setSize({SCREEN_WIDTH * PLAY_SCALE, SCREEN_HEIGHT * PLAY_SCALE});
            mainView.place(0.f, 0.f, static_cast<float>(PLAY_SCALE));
        } else {
            constexpr float SCREEN_W = SCREEN_WIDTH * DEBUG_SCALE;
            constexpr float SCREEN_H = SCREEN_HEIGHT * DEBUG_SCALE;
            constexpr float NT_W = SCREEN_WIDTH;
            constexpr float NT_H = SCREEN_HEIGHT;
            constexpr float PATTERN = PATTERN_SIZE;

            const float ntX = SCREEN_W + MARGIN;
            const float patternX = ntX + NT_W + MARGIN;

            mainWindow.setSize({static_cast<unsigned>(patternX + PATTERN), static_cast<unsigned>(SCREEN_H)});
            mainView.place(0.f, 0.f, static_cast<float>(DEBUG_SCALE));
            nametable1View.place(ntX, 0.f, 1.f);
            nametable2View.place(ntX, NT_H, 1.f);
            pattern1View.place(patternX, 0.f, 1.f);
            pattern2View.place(patternX, PATTERN, 1.f);
        }
    };

    // Skip the first frames if specified in the arguments (debug mode)
    for (int i = 0; i < parser.get<int>("skip"); i++) {
        nes.runUntilFrame();
    }

    applyLayout();

    bool tabHeld = false;
    std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();
    while (mainWindow.isOpen() && nes.isRunning()) {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        float fps = 1.0f / static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime).count()) * 1000.0f;
        mainWindow.setTitle("NES Emulator - FPS: " + std::to_string(fps));
        lastFrameTime = currentTime;

        sf::Event event{};
        while (mainWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                mainWindow.close();
            }

            // read controls
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Tab:
                        // Ignore auto-repeat, otherwise the mode would flicker.
                        if (!tabHeld) {
                            tabHeld = true;
                            mode = mode == Mode::Play ? Mode::Debug : Mode::Play;
                            applyLayout();
                        }
                        break;
                    case sf::Keyboard::Space:
                        selectedPalette = static_cast<std::uint8_t>((selectedPalette + 1) % 0x08);
                        break;
                    case sf::Keyboard::X:
                        nes.setButton(NES::Controller::P1, NES::Button::A, true);
                        break;
                    case sf::Keyboard::Z:
                        nes.setButton(NES::Controller::P1, NES::Button::B, true);
                        break;
                    case sf::Keyboard::BackSpace:
                        nes.setButton(NES::Controller::P1, NES::Button::SELECT, true);
                        break;
                    case sf::Keyboard::Enter:
                        nes.setButton(NES::Controller::P1, NES::Button::START, true);
                        break;
                    case sf::Keyboard::Up:
                        nes.setButton(NES::Controller::P1, NES::Button::UP, true);
                        break;
                    case sf::Keyboard::Down:
                        nes.setButton(NES::Controller::P1, NES::Button::DOWN, true);
                        break;
                    case sf::Keyboard::Left:
                        nes.setButton(NES::Controller::P1, NES::Button::LEFT, true);
                        break;
                    case sf::Keyboard::Right:
                        nes.setButton(NES::Controller::P1, NES::Button::RIGHT, true);
                        break;
                    default:
                        break;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                switch (event.key.code) {
                    case sf::Keyboard::Tab:
                        tabHeld = false;
                        break;
                    case sf::Keyboard::X:
                        nes.setButton(NES::Controller::P1, NES::Button::A, false);
                        break;
                    case sf::Keyboard::Z:
                        nes.setButton(NES::Controller::P1, NES::Button::B, false);
                        break;
                    case sf::Keyboard::BackSpace:
                        nes.setButton(NES::Controller::P1, NES::Button::SELECT, false);
                        break;
                    case sf::Keyboard::Enter:
                        nes.setButton(NES::Controller::P1, NES::Button::START, false);
                        break;
                    case sf::Keyboard::Up:
                        nes.setButton(NES::Controller::P1, NES::Button::UP, false);
                        break;
                    case sf::Keyboard::Down:
                        nes.setButton(NES::Controller::P1, NES::Button::DOWN, false);
                        break;
                    case sf::Keyboard::Left:
                        nes.setButton(NES::Controller::P1, NES::Button::LEFT, false);
                        break;
                    case sf::Keyboard::Right:
                        nes.setButton(NES::Controller::P1, NES::Button::RIGHT, false);
                        break;
                    default:
                        break;
                }
            }
        }

        nes.runUntilFrame();

        mainView.update(nes.ppu.renderFrame());
        mainWindow.clear();
        mainWindow.draw(mainView.sprite());

        if (mode == Mode::Debug) {
            nametable1View.update(nes.ppu.renderNametable1());
            nametable2View.update(nes.ppu.renderNametable2());
            pattern1View.update(nes.ppu.renderPatternTable(0, selectedPalette));
            pattern2View.update(nes.ppu.renderPatternTable(1, selectedPalette));
            mainWindow.draw(nametable1View.sprite());
            mainWindow.draw(nametable2View.sprite());
            mainWindow.draw(pattern1View.sprite());
            mainWindow.draw(pattern2View.sprite());
        }

        mainWindow.display();
    }

    return 0;
}
