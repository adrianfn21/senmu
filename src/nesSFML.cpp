#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

int main(int argc, char** argv) {
    // args: [1] path to rom

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path to rom>" << std::endl;
        return 1;
    }

    const std::string GAME_PATH = argv[1];

    iNES::iNES rom(GAME_PATH);
    NES::NesSystem nes(rom);

    constexpr size_t PATTERNS_SIZE = 128;
    constexpr size_t SCALE_FACTOR = 2;

    sf::RenderWindow mainWindow(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "NES Emulator");
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

    while (patternWindow1.isOpen() && patternWindow2.isOpen() && nes.isRunning()) {
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

        auto display = []<size_t width, size_t height>(sf::RenderWindow& rw, const NES::Image<NES::Color, width, height>& img) {
            sf::Image sfmlImage;
            sfmlImage.create(width, height);

            for (unsigned int y = 0; y < height; y++) {
                for (unsigned int x = 0; x < width; x++) {
                    sfmlImage.setPixel(x, y, sf::Color(img[y][x].r, img[y][x].g, img[y][x].b));
                }
            }

            sf::Texture texture;
            texture.loadFromImage(sfmlImage);
            sf::Sprite sprite;
            sprite.setTexture(texture);
            sprite.setScale(SCALE_FACTOR, SCALE_FACTOR);

            rw.draw(sprite);
            rw.display();
        };

        patternWindow1.clear();
        display(patternWindow1, nes.ppu.renderPatternTable(0, selectedPalette));

        patternWindow2.clear();
        display(patternWindow2, nes.ppu.renderPatternTable(1, selectedPalette));

        mainWindow.clear();
        display(mainWindow, nes.ppu.renderFrame());

        nametableWindow1.clear();
        display(nametableWindow1, nes.ppu.renderNametable1());

        nametableWindow2.clear();
        display(nametableWindow2, nes.ppu.renderNametable2());
    }

    return 0;
}