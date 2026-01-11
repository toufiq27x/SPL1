#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

int main() {
    sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "Pac-Man Game");
    sf::Font font;
    
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
                cout << "Error: Could not load font!" << endl;
                return 1;
            }
        }
    }
    
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);
        window.display();
    }
    
    return 0;
}
