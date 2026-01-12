#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>

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

float animationTime = 0.0f;

void loadMazeFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    vector<string> tempMaze;
    string line;
    
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        tempMaze.push_back(line);
    }
    file.close();

    if (tempMaze.empty()) {
        cout << "Error: Maze file is empty!" << endl;
        exit(1);
    }

    ROWS = tempMaze.size();
    COLS = 0;
    for (const auto& row : tempMaze) {
        if (row.size() > COLS) COLS = row.size();
    }
    
    maze.resize(ROWS, vector<int>(COLS, EMPTY));
    remainingDots = 0;
    
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            char cell = (j < tempMaze[i].size()) ? tempMaze[i][j] : ' ';
            
            switch (cell) {
                case '#': maze[i][j] = WALL; break;
                case 'O': maze[i][j] = OBSTACLE; break;
                case 'P': maze[i][j] = POWER_UP; break;
                case '.': maze[i][j] = DOT; remainingDots++; break;
                case 'E': maze[i][j] = EMPTY; break;
                case ' ': maze[i][j] = EMPTY; break;
                default: maze[i][j] = EMPTY; break;
            }
        }
    }
}

void drawCircle(float x, float y, float radius, sf::Color color) {
    sf::CircleShape circle(radius);
    circle.setFillColor(color);
    circle.setPosition({x - radius, y - radius});
    window.draw(circle);
}

void drawMaze() {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            float x = j * CELL_SIZE;
            float y = i * CELL_SIZE;

            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition({x, y});

            switch (maze[i][j]) {
                case WALL:
                    cell.setFillColor(sf::Color(30, 60, 150));
                    cell.setOutlineColor(sf::Color(50, 100, 200));
                    cell.setOutlineThickness(1);
                    window.draw(cell);
                    break;
                case OBSTACLE:
                    cell.setFillColor(sf::Color(50, 50, 80));
                    cell.setOutlineColor(sf::Color(80, 80, 120));
                    cell.setOutlineThickness(1);
                    window.draw(cell);
                    break;
                case DOT: {
                    cell.setFillColor(sf::Color(10, 10, 20));
                    window.draw(cell);
                    float pulse = 1.0f + 0.3f * sin(animationTime * 5.0f);
                    drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 3 * pulse, sf::Color(255, 200, 100));
                    break;
                }
                case POWER_UP: {
                    cell.setFillColor(sf::Color(10, 10, 20));
                    window.draw(cell);
                    float powerPulse = 1.0f + 0.5f * sin(animationTime * 3.0f);
                    drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 8 * powerPulse, sf::Color(255, 215, 0));
                    break;
                }
                case EMPTY:
                    cell.setFillColor(sf::Color(10, 10, 20));
                    window.draw(cell);
                    break;
            }
        }
    }

    // Draw player
    drawCircle(playerY * CELL_SIZE + CELL_SIZE / 2, 
               playerX * CELL_SIZE + CELL_SIZE / 2, 
               10, sf::Color(255, 255, 0));
}

int main() {
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
                cout << "Error: Could not load font!" << endl;
                return 1;
            }
        }
    }
    
    loadMazeFromFile("easy_maze.txt");
    
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        
        animationTime += 0.05f;
        
        window.clear(sf::Color(10, 10, 20));
        drawMaze();
        window.display();
    }
    
    return 0;
}
