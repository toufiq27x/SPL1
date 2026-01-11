#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

using namespace std;

#define CELL_SIZE 20

#define WALL 1
#define OBSTACLE 2
#define DOT 3
#define POWER_UP 4
#define RED_ZONE 5
#define EMPTY 0

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

vector<vector<int>> maze;
int ROWS;
int COLS;

int playerX = 1;
int playerY = 1;
char playerDirection = 'd';

int score = 0;
int level = 1;
bool gameOver = false;
bool gameWon = false;

int remainingDots = 0;

sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "Pac-Man Game");
sf::Font font;

int main() {
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
