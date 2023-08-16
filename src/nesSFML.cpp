#include <SFML/Graphics.hpp>
#include <iostream>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

sf::Image getPatternTable(const NES::NesSystem& nes, uint8_t patternTable, uint8_t palette, unsigned int width, unsigned int height) {
    sf::Image pattern;
    pattern.create(width, height, sf::Color::Black);

    constexpr size_t TILE_SIZE = 8;
    constexpr size_t GRID_TILE_SIZE = 16;

    std::array<sf::Color, 4> paletteColors;
    for (uint8_t c = 0x00; c < 0x04; c++) {
        NES::Color color = nes.getColor(palette, c);
        paletteColors[c] = sf::Color(color.r, color.g, color.b);
    }

    for (size_t tile_i = 0; tile_i < GRID_TILE_SIZE; tile_i++) {
        for (size_t tile_j = 0; tile_j < GRID_TILE_SIZE; tile_j++) {

            auto data = nes.getSprite(static_cast<uint8_t>(tile_i), static_cast<uint8_t>(tile_j), patternTable);

            for (size_t pixel_i = 0; pixel_i < 8; pixel_i++) {
                for (size_t pixel_j = 0; pixel_j < 8; pixel_j++) {
                    sf::Color c = paletteColors[data[pixel_i * TILE_SIZE + pixel_j]];
                    pattern.setPixel(static_cast<unsigned int>(tile_j * TILE_SIZE + pixel_j), static_cast<unsigned int>(tile_i * TILE_SIZE + pixel_i), c);
                }
            }
        }
    }

    return pattern;
}

const std::string GAME_PATH = "nestest.nes";

int main() {

    constexpr size_t PATTERNS_SIZE = 128;
    constexpr size_t SCALE_FACTOR = 2;

    sf::RenderWindow patternWindow1(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 1");
    sf::RenderWindow patternWindow2(sf::VideoMode(PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR), "Sprites 2");
    patternWindow1.setPosition(sf::Vector2i(0, 0));
    patternWindow2.setPosition(sf::Vector2i(PATTERNS_SIZE * SCALE_FACTOR, 0));

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

        patternWindow1.clear();
        sf::Image img = getPatternTable(nes, 0, selectedPalette, PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR);
        sf::Texture patternTexture;
        patternTexture.loadFromImage(img);
        sf::Sprite patternSprite;
        patternSprite.setTexture(patternTexture);
        patternSprite.setScale(SCALE_FACTOR, SCALE_FACTOR);
        patternWindow1.draw(patternSprite);
        patternWindow1.display();

        patternWindow2.clear();
        img = getPatternTable(nes, 1, selectedPalette, PATTERNS_SIZE * SCALE_FACTOR, PATTERNS_SIZE * SCALE_FACTOR);
        patternTexture.loadFromImage(img);
        patternSprite.setTexture(patternTexture);
        patternSprite.setScale(SCALE_FACTOR, SCALE_FACTOR);
        patternWindow2.draw(patternSprite);
        patternWindow2.display();
    }

    return 0;
}