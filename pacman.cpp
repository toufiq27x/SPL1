#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

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
        
        window.clear(sf::Color::Black);
        window.display();
    }
    
    return 0;
}
