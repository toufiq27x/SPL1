#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <string>
#include <algorithm>
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
const int SCREEN_HEIGHT = 620;

vector<vector<int>> maze;
int ROWS;
int COLS;

int playerX = 1;
int playerY = 1;
char playerDirection = 'd';

int ghostCount = 2;
int ghostsX[10];
int ghostsY[10];
int ghostMoveCounter = 0;
int ghostMoveFrequency = 5;

int DX[] = {-1, 1, 0, 0};
int DY[] = {0, 0, -1, 1};

int score = 0;
bool gameOver = false;
bool gameWon = false;
bool invincible = false;

int remainingDots = 0;
const int MAX_RED_ZONES = 10;
int redZoneCount = 1;
int redZoneX[MAX_RED_ZONES];
int redZoneY[MAX_RED_ZONES];

int startTime;
int elapsedTime;
int ghostCooldown = 0;

sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "Pac-Man Game");
sf::Font font;

float animationTime = 0.0f;
float mouthAnimation = 0.0f;

// Function declarations
void loadMazeFromFile(const string& filename);
void initializeGhostPositions();
bool isGhostAtPosition(int x, int y);
void drawScoreAndTime(int elapsedTime);
void startGame(const string& levelFile);

struct Node {
    int x, y;
    int g, h;
    Node* parent;

    Node(int x, int y, int g, int h, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), parent(parent) {}

    int f() const {
        return g + h;
    }
};

void drawText(const string& str, float x, float y, int size = 20, sf::Color color = sf::Color::White, bool bold = false) {
    sf::Text text(font, str, size);
    text.setFillColor(color);
    if (bold) text.setStyle(sf::Text::Bold);
    text.setPosition({x, y});
    window.draw(text);
}

void drawCircle(float x, float y, float radius, sf::Color color) {
    sf::CircleShape circle(radius);
    circle.setFillColor(color);
    circle.setPosition({x - radius, y - radius});
    window.draw(circle);
}

void drawPacMan(float x, float y, float radius, sf::Color color, char direction, float mouthOpen) {
    sf::CircleShape pacman(radius, 30);
    pacman.setFillColor(color);
    
    float rotationAngle = 0;
    switch(direction) {
        case 'd': rotationAngle = 0; break;
        case 'a': rotationAngle = 180; break;
        case 'w': rotationAngle = -90; break;
        case 's': rotationAngle = 90; break;
    }
    
    pacman.setOrigin({radius, radius});
    pacman.setPosition({x, y});
    pacman.setRotation(sf::degrees(rotationAngle));
    
    window.draw(pacman);
    
    float mouthAngle = 30.0f * mouthOpen;
    sf::ConvexShape mouth;
    mouth.setPointCount(3);
    mouth.setPoint(0, sf::Vector2f(0, 0));
    
    float rad1 = (mouthAngle) * 3.14159f / 180.0f;
    float rad2 = (-mouthAngle) * 3.14159f / 180.0f;
    
    mouth.setPoint(1, sf::Vector2f(radius * 1.2f * cos(rad1), radius * 1.2f * sin(rad1)));
    mouth.setPoint(2, sf::Vector2f(radius * 1.2f * cos(rad2), radius * 1.2f * sin(rad2)));
    
    mouth.setFillColor(sf::Color(10, 10, 20));
    mouth.setOrigin({0, 0});
    mouth.setPosition({x, y});
    mouth.setRotation(sf::degrees(rotationAngle));
    
    window.draw(mouth);
}

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

    if (maze[playerX][playerY] == DOT) {
        remainingDots--;
        maze[playerX][playerY] = EMPTY;
    }
    
    for (int i = 0; i < ghostCount; ++i) {
        if (maze[ghostsX[i]][ghostsY[i]] == DOT) {
            remainingDots--;
            maze[ghostsX[i]][ghostsY[i]] = EMPTY;
        }
    }

    vector<pair<int, int>> explicitEmptyPositions;
    for (int i = 0; i < ROWS && i < tempMaze.size(); ++i) {
        for (int j = 0; j < COLS && j < tempMaze[i].size(); ++j) {
            if (tempMaze[i][j] == 'E') {
                explicitEmptyPositions.push_back({i, j});
            }
        }
    }

    srand(time(0));
    int count = 0;
    
    while (count < redZoneCount && !explicitEmptyPositions.empty()) {
        int randomIndex = rand() % explicitEmptyPositions.size();
        int x = explicitEmptyPositions[randomIndex].first;
        int y = explicitEmptyPositions[randomIndex].second;
        
        bool isOccupied = (x == playerX && y == playerY);
        for (int g = 0; g < ghostCount && !isOccupied; ++g) {
            if (x == ghostsX[g] && y == ghostsY[g]) isOccupied = true;
        }
        
        if (!isOccupied && maze[x][y] == EMPTY) {
            redZoneX[count] = x;
            redZoneY[count] = y;
            maze[x][y] = RED_ZONE;
            count++;
        }
        
        explicitEmptyPositions.erase(explicitEmptyPositions.begin() + randomIndex);
    }
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
                    sf::Color powerColor(255, 215, 0);
                    powerColor.a = 150 + 105 * sin(animationTime * 3.0f);
                    drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 8 * powerPulse, powerColor);
                    drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 10 * powerPulse, 
                              sf::Color(255, 215, 0, 50));
                    break;
                }
                case RED_ZONE: {
                    float redPulse = 0.8f + 0.2f * sin(animationTime * 4.0f);
                    cell.setFillColor(sf::Color(200 * redPulse, 0, 0));
                    cell.setOutlineColor(sf::Color(255, 50, 50));
                    cell.setOutlineThickness(2);
                    window.draw(cell);
                    break;
                }
                case EMPTY:
                    cell.setFillColor(sf::Color(10, 10, 20));
                    window.draw(cell);
                    break;
            }
        }
    }

    float playerGlow = invincible ? (1.5f + 0.5f * sin(animationTime * 10.0f)) : 1.0f;
    sf::Color playerColor = invincible ? sf::Color(100, 255, 100) : sf::Color(255, 255, 0);
    
    if (invincible) {
        drawCircle(playerY * CELL_SIZE + CELL_SIZE / 2, 
                   playerX * CELL_SIZE + CELL_SIZE / 2, 15 * playerGlow, 
                   sf::Color(100, 255, 100, 100));
    }
    
    float mouthOpen = 0.5f + 0.5f * sin(mouthAnimation * 8.0f);
    drawPacMan(playerY * CELL_SIZE + CELL_SIZE / 2, 
               playerX * CELL_SIZE + CELL_SIZE / 2, 
               10, playerColor, playerDirection, mouthOpen);

    for (int i = 0; i < ghostCount; ++i) {
        float ghostWave = 2.0f * sin(animationTime * 5.0f + i);
        
        drawCircle(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2,
                   ghostsX[i] * CELL_SIZE + CELL_SIZE / 2 + ghostWave, 
                   14, sf::Color(255, 0, 0, 100));
        
        sf::Color ghostColor = invincible ? sf::Color(100, 100, 255) : sf::Color(255, 50, 50);
        drawCircle(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2,
                   ghostsX[i] * CELL_SIZE + CELL_SIZE / 2 + ghostWave, 
                   10, ghostColor);
        
        drawCircle(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2 - 3,
                   ghostsX[i] * CELL_SIZE + CELL_SIZE / 2 - 2 + ghostWave, 
                   2, sf::Color::White);
        drawCircle(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2 + 3,
                   ghostsX[i] * CELL_SIZE + CELL_SIZE / 2 - 2 + ghostWave, 
                   2, sf::Color::White);
    }
}

void drawScoreAndTime(int elapsedTime) {
    sf::RectangleShape hudPanel(sf::Vector2f(SCREEN_WIDTH, 80));
    hudPanel.setPosition({0, SCREEN_HEIGHT - 80});
    hudPanel.setFillColor(sf::Color(20, 20, 40, 230));
    hudPanel.setOutlineColor(sf::Color(70, 130, 255));
    hudPanel.setOutlineThickness(2);
    window.draw(hudPanel);

    drawText("SCORE", 40, SCREEN_HEIGHT - 70, 16, sf::Color(150, 150, 200));
    drawText(to_string(score), 40, SCREEN_HEIGHT - 45, 28, sf::Color::White, true);
    
    drawText("TIME", 250, SCREEN_HEIGHT - 70, 16, sf::Color(150, 150, 200));
    drawText(to_string(elapsedTime) + "s", 250, SCREEN_HEIGHT - 45, 28, sf::Color::White, true);
    
    drawText("DOTS", 650, SCREEN_HEIGHT - 70, 16, sf::Color(150, 150, 200));
drawText(to_string(remainingDots - 1), 650, SCREEN_HEIGHT - 45, 28, sf::Color(255, 200, 100), true);

    if (ghostCooldown > 0) {
        sf::RectangleShape powerBar(sf::Vector2f(150, 30));
        powerBar.setPosition({820, SCREEN_HEIGHT - 55});
        powerBar.setFillColor(sf::Color(255, 215, 0, 200));
        powerBar.setOutlineColor(sf::Color::White);
        powerBar.setOutlineThickness(2);
        window.draw(powerBar);
        
        drawText("POWER!", 845, SCREEN_HEIGHT - 50, 20, sf::Color::Black, true);
    }
}

void heapifyUp(vector<Node*>& heap, int index) {
    while (index > 0) {
        int parentIdx = (index - 1) / 2;
        if (heap[index]->f() >= heap[parentIdx]->f()) break;
        swap(heap[index], heap[parentIdx]);
        index = parentIdx;
    }
}

void heapifyDown(vector<Node*>& heap, int index) {
    int leftChildIdx = 2 * index + 1;
    int rightChildIdx = 2 * index + 2;
    int smallestIdx = index;

    if (leftChildIdx < heap.size() && heap[leftChildIdx]->f() < heap[smallestIdx]->f()) {
        smallestIdx = leftChildIdx;
    }
    if (rightChildIdx < heap.size() && heap[rightChildIdx]->f() < heap[smallestIdx]->f()) {
        smallestIdx = rightChildIdx;
    }
    if (smallestIdx != index) {
        swap(heap[index], heap[smallestIdx]);
        heapifyDown(heap, smallestIdx);
    }
}

void pushToMinHeap(vector<Node*>& heap, Node* node) {
    heap.push_back(node);
    heapifyUp(heap, heap.size() - 1);
}

Node* popFromMinHeap(vector<Node*>& heap) {
    if (heap.empty()) return nullptr;
    Node* top = heap[0];
    heap[0] = heap.back();
    heap.pop_back();
    heapifyDown(heap, 0);
    return top;
}

bool isValidMove(int x, int y) {
    return (x >= 0 && x < ROWS && y >= 0 && y < COLS && 
            maze[x][y] != WALL && maze[x][y] != OBSTACLE);
}

int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

vector<Node> aStar(int startX, int startY, int goalX, int goalY) {
    vector<Node*> openSet;
    unordered_map<int, unordered_map<int, bool>> closedSet;
    unordered_map<int, unordered_map<int, Node*>> allNodes;

    Node* startNode = new Node{startX, startY, 0, manhattanDistance(startX, startY, goalX, goalY), nullptr};
    pushToMinHeap(openSet, startNode);
    allNodes[startX][startY] = startNode;

    vector<Node> path;

    while (!openSet.empty()) {
        Node* currentNode = popFromMinHeap(openSet);

        if (currentNode->x == goalX && currentNode->y == goalY) {
            Node* node = currentNode;
            while (node != nullptr) {
                path.push_back(*node);
                node = node->parent;
            }
            reverse(path.begin(), path.end());
            break;
        }

        closedSet[currentNode->x][currentNode->y] = true;

        for (int i = 0; i < 4; ++i) {
            int newX = currentNode->x + DX[i];
            int newY = currentNode->y + DY[i];

            if (isValidMove(newX, newY) && !closedSet[newX][newY]) {
                int newG = currentNode->g + 1;
                int newH = manhattanDistance(newX, newY, goalX, goalY);

                if (allNodes[newX][newY]) {
                    Node* neighbor = allNodes[newX][newY];
                    if (newG < neighbor->g) {
                        neighbor->g = newG;
                        neighbor->parent = currentNode;
                        pushToMinHeap(openSet, neighbor);
                    }
                } else {
                    Node* newNode = new Node{newX, newY, newG, newH, currentNode};
                    pushToMinHeap(openSet, newNode);
                    allNodes[newX][newY] = newNode;
                }
            }
        }
    }

    for (auto& row : allNodes) {
        for (auto& pair : row.second) {
            delete pair.second;
        }
    }

    return path;
}

void initializeGhostPositions() {
    for (int i = 0; i < ghostCount; ++i) {
        ghostsX[i] = ROWS / 2 + i % 2;
        ghostsY[i] = COLS / 2 + (i / 2);
        maze[ghostsX[i]][ghostsY[i]] = EMPTY;
    }
}

bool isGhostAtPosition(int x, int y) {
    for (int i = 0; i < ghostCount; ++i) {
        if (ghostsX[i] == x && ghostsY[i] == y) return true;
    }
    return false;
}

void moveGhosts() {
    for (int i = 0; i < ghostCount; ++i) {
        int targetX = playerX;
        int targetY = playerY;

        if (i == 0) { targetX = playerX - 1; }
        else if (i == 1) { targetX = playerX + 1; }

        if (rand() % 2 == 0) targetX += (rand() % 2) * 2 - 1;
        if (rand() % 2 == 0) targetY += (rand() % 2) * 2 - 1;
        
        targetX = max(0, min(ROWS - 1, targetX));
        targetY = max(0, min(COLS - 1, targetY));

        vector<Node> path = aStar(ghostsX[i], ghostsY[i], targetX, targetY);

        if (!path.empty() && path.size() > 1) {
            Node nextNode = path[1];
            ghostsX[i] = nextNode.x;
            ghostsY[i] = nextNode.y;
        }
    }
}

void movePlayer(char direction) {
    int newX = playerX, newY = playerY;
    
    switch (direction) {
        case 'w': newX--; break;
        case 's': newX++; break;
        case 'a': newY--; break;
        case 'd': newY++; break;
        default: return;
    }

    if (isValidMove(newX, newY)) {
        playerX = newX;
        playerY = newY;
        playerDirection = direction;
        
        if (maze[newX][newY] == DOT) {
            remainingDots--;
            maze[newX][newY] = EMPTY;
            score += 5;
        } else if (maze[newX][newY] == POWER_UP) {
            score += 10;
            maze[newX][newY] = EMPTY;
            ghostCooldown = 5;
            invincible = true;
        }
        
        if (ghostCooldown > 0) {
            ghostCooldown--;
            if (ghostCooldown == 0) {
                invincible = false;
            }
        }
    }
}

void checkCollision() {
    for (int i = 0; i < redZoneCount; ++i) {
        if (playerX == redZoneX[i] && playerY == redZoneY[i]) {
            gameOver = true;
            gameWon = false;
            return;
        }
    }
    
    if (!invincible) {
        for (int i = 0; i < ghostCount; ++i) {
            if (playerX == ghostsX[i] && playerY == ghostsY[i]) {
                gameOver = true;
                gameWon = false;
                return;
            }
        }
    }
    
    if (remainingDots <= 1) {
        gameOver = true;
        gameWon = true;
        return;
    }
}

void gameOverScreen(int finalScore, bool won) {
    bool running = true;
    float glowTime = 0;
    
    while (running && window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPress->code == sf::Keyboard::Key::Escape || 
                    keyPress->code == sf::Keyboard::Key::Space ||
                    keyPress->code == sf::Keyboard::Key::Enter) {
                    return;
                }
            }
        }

        glowTime += 0.05f;

        window.clear(sf::Color(15, 15, 30));

        for (int i = 0; i < 50; i++) {
            float x = 100 + (i * 25) % 900;
            float y = 50 + ((i * 25) / 900) * 600;
            float pulse = 1.0f + 0.5f * sin(glowTime * 3 + i);
            sf::Color particleColor = won ? sf::Color(100, 255, 100, 50) : sf::Color(255, 100, 100, 50);
            drawCircle(x, y, 5 * pulse, particleColor);
        }

        if (won) {
            for (int i = 0; i < 5; i++) {
                float offset = i * 2;
                drawText("YOU WIN!", 320 + offset, 150 + offset, 64, sf::Color(255, 255, 100, 50), true);
            }
            drawText("YOU WIN!", 320, 150, 64, sf::Color(255, 255, 0), true);
            drawText("ALL DOTS COLLECTED!", 280, 230, 28, sf::Color(150, 255, 150), true);
        } else {
            drawText("GAME OVER!", 305, 153, 64, sf::Color(0, 0, 0, 100), true);
            drawText("GAME OVER!", 300, 150, 64, sf::Color(255, 50, 50), true);
        }

        sf::RectangleShape scorePanel(sf::Vector2f(500, 120));
        scorePanel.setPosition({250, 280});
        scorePanel.setFillColor(sf::Color(30, 40, 80, 200));
        scorePanel.setOutlineColor(sf::Color(70, 130, 255));
        scorePanel.setOutlineThickness(3);
        window.draw(scorePanel);

        drawText("FINAL SCORE", 380, 300, 24, sf::Color(150, 150, 200));
        drawText(to_string(finalScore), 430, 340, 48, sf::Color(255, 215, 0), true);

        drawText("Press SPACE or ESC to exit", 320, 480, 22, sf::Color(150, 200, 255));

        window.display();
    }
}

void startGame(const string& levelFile) {
    playerX = 1;
    playerY = 1;
    playerDirection = 'd';
    gameOver = false;
    gameWon = false;
    score = 0;
    ghostMoveCounter = 0;
    ghostCooldown = 0;
    animationTime = 0;
    mouthAnimation = 0;
    invincible = false;

    loadMazeFromFile(levelFile);
    initializeGhostPositions();
    
    startTime = time(0);

    sf::Clock clock;
    
    while (window.isOpen() && !gameOver) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPress->code) {
                    case sf::Keyboard::Key::W: movePlayer('w'); break;
                    case sf::Keyboard::Key::S: movePlayer('s'); break;
                    case sf::Keyboard::Key::A: movePlayer('a'); break;
                    case sf::Keyboard::Key::D: movePlayer('d'); break;
                    case sf::Keyboard::Key::Escape: 
                        window.close();
                        return;
                    default: break;
                }
                checkCollision();
            }
        }

        if (clock.getElapsedTime().asMilliseconds() >= 200) {
            elapsedTime = time(0) - startTime;

            if (ghostMoveCounter % ghostMoveFrequency == 0) {
                moveGhosts();
            }
            ghostMoveCounter++;

            checkCollision();
            clock.restart();
        }

        animationTime += 0.05f;
        mouthAnimation += 0.1f;

        window.clear(sf::Color(10, 10, 20));
        drawMaze();
        drawScoreAndTime(elapsedTime);
        window.display();
    }

    if (window.isOpen()) {
        int finalScore = (elapsedTime > 0) ? (score * 100 / elapsedTime) : score * 100;
        gameOverScreen(finalScore, gameWon);
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

    startGame("easy_maze.txt");

    return 0;
}
