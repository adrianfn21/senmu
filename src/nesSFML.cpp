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

    constexpr size_t PATTERNS_SIZE = 128;
    constexpr size_t SCALE_FACTOR = 2;

    sf::RenderWindow bgWindow(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "Background");
    sf::RenderWindow nametableWindow1(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "Nametable 1");
    sf::RenderWindow nametableWindow2(sf::VideoMode(256 * SCALE_FACTOR, 240 * SCALE_FACTOR), "Nametable 2");
    sf::RenderWindow patternWindow1(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 1");
    sf::RenderWindow patternWindow2(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 2");
    bgWindow.setPosition(sf::Vector2i(0, 0));
    nametableWindow1.setPosition(sf::Vector2i(256 * SCALE_FACTOR, 0));
    nametableWindow2.setPosition(sf::Vector2i(256 * SCALE_FACTOR * 2, 0));
    patternWindow1.setPosition(sf::Vector2i(256 * SCALE_FACTOR, 240 * SCALE_FACTOR));
    patternWindow2.setPosition(sf::Vector2i(256 * SCALE_FACTOR + PATTERNS_SIZE * SCALE_FACTOR, 240 * SCALE_FACTOR));

    NES::NesSystem nes((iNES::iNES(GAME_PATH)));

    uint8_t selectedPalette = 0x00;

    while (patternWindow1.isOpen() && patternWindow2.isOpen() && nes.isRunning()) {
        sf::Event event{};
        while (patternWindow1.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                patternWindow1.close();
            }

            // space bar pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                selectedPalette = static_cast<uint8_t>((selectedPalette + 1) % 0x08);
                patternWindow1.setTitle("Sprites 1 - Palette: " + std::to_string(selectedPalette));
            }
        }
        while (patternWindow2.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                patternWindow2.close();
            }

            // space bar pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                selectedPalette = static_cast<uint8_t>((selectedPalette + 1) % 0x08);
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

        bgWindow.clear();
        display(bgWindow, nes.ppu.renderBackground());

        nametableWindow1.clear();
        display(nametableWindow1, nes.ppu.renderNametable1());

        nametableWindow2.clear();
        display(nametableWindow2, nes.ppu.renderNametable2());
    }

    return 0;
}