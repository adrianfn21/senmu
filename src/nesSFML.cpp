#include <SFML/Graphics.hpp>
#include <argparse/argparse.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

namespace {

constexpr std::size_t SCALE_FACTOR = 2;

/**
 * @brief Reusable canvas for displaying a NES image in an SFML window.
 *
 * The texture and sprite are created once and updated in place every frame.
 * Allocating a new sf::Texture per frame is what tanked the frame rate.
 */
template <std::size_t Width, std::size_t Height>
class WindowCanvas {
  public:
    WindowCanvas() {
        texture.create(Width, Height);
        sprite.setTexture(texture);
        sprite.setScale(static_cast<float>(SCALE_FACTOR), static_cast<float>(SCALE_FACTOR));
    }

    void draw(sf::RenderWindow& window, const NES::Image<NES::Color, Width, Height>& img) {
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
        window.draw(sprite);
    }

  private:
    sf::Texture texture;
    sf::Sprite sprite;
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

    constexpr size_t PATTERNS_SIZE = 128;

    sf::RenderWindow mainWindow(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "NES Emulator");
    mainWindow.setFramerateLimit(60);
    sf::RenderWindow nametableWindow1(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "Nametable 1");
    sf::RenderWindow nametableWindow2(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "Nametable 2");
    sf::RenderWindow patternWindow1(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 1");
    sf::RenderWindow patternWindow2(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 2");
    mainWindow.setPosition(sf::Vector2i(0, 0));
    nametableWindow1.setPosition(sf::Vector2i(256 * SCALE_FACTOR, 0));
    nametableWindow2.setPosition(sf::Vector2i(256 * SCALE_FACTOR * 2, 0));
    patternWindow1.setPosition(sf::Vector2i(256 * SCALE_FACTOR, 240 * SCALE_FACTOR));
    patternWindow2.setPosition(sf::Vector2i(256 * SCALE_FACTOR + PATTERNS_SIZE * SCALE_FACTOR, 240 * SCALE_FACTOR));

    std::uint8_t selectedPalette = 0x00;

    WindowCanvas<256, 240> mainCanvas;
    WindowCanvas<256, 240> nametableCanvas1;
    WindowCanvas<256, 240> nametableCanvas2;
    WindowCanvas<128, 128> patternCanvas1;
    WindowCanvas<128, 128> patternCanvas2;

    // Skip the first frames if specified in the arguments (debug mode)
    for (int i = 0; i < parser.get<int>("skip"); i++) {
        nes.runUntilFrame();
    }

    std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();
    while (patternWindow1.isOpen() && patternWindow2.isOpen() && nes.isRunning()) {
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

        while (patternWindow1.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                patternWindow1.close();
            }

            // space bar pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                selectedPalette = static_cast<std::uint8_t>((selectedPalette + 1) % 0x08);
                patternWindow1.setTitle("Sprites 1 - Palette: " + std::to_string(selectedPalette));
            }
        }
        while (patternWindow2.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                patternWindow2.close();
            }

            // space bar pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                selectedPalette = static_cast<std::uint8_t>((selectedPalette + 1) % 0x08);
                patternWindow2.setTitle("Sprites 2 - Palette: " + std::to_string(selectedPalette));
            }
        }

        nes.runUntilFrame();

        auto present = [](sf::RenderWindow& window, auto& canvas, const auto& image) {
            window.clear();
            canvas.draw(window, image);
            window.display();
        };

        present(patternWindow1, patternCanvas1, nes.ppu.renderPatternTable(0, selectedPalette));
        present(patternWindow2, patternCanvas2, nes.ppu.renderPatternTable(1, selectedPalette));
        present(mainWindow, mainCanvas, nes.ppu.renderFrame());
        present(nametableWindow1, nametableCanvas1, nes.ppu.renderNametable1());
        present(nametableWindow2, nametableCanvas2, nes.ppu.renderNametable2());
    }

    return 0;
}