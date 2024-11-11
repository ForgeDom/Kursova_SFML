#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <iostream>
#include <limits>
#include <thread>
#include <chrono>
#include <random>
#include <set>
#include <filesystem>
using namespace std;

const int TILE_SIZE = 30;
const int WIDTH = 40;
const int HEIGHT = 40;
const int INFO_HEIGHT = 100;

enum class Cell { WALL, PATH, START, END, VISITED, SHORTEST };
enum class Algorithm { NONE, DFS, DIJKSTRA, ASTAR };

Cell maze[HEIGHT][WIDTH];

Algorithm currentAlgo = Algorithm::NONE;

void generateMaze() {
    cout << "maze dimensions: HEIGHT = " << HEIGHT << ", WIDTH = " << WIDTH << endl;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            maze[i][j] = Cell::WALL;
            cout << "Setting maze[" << i << "][" << j << "] to WALL" << endl;
        }
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 3);

    pair<int, int> cellsStack[HEIGHT * WIDTH];
    int stackSize = 0;

    int startX = 1, startY = 1;
    maze[startX][startY] = Cell::START;
    cellsStack[stackSize++] = { startX, startY };

    pair<int, int> directions[] = { {0, -2}, {0, 2}, {-2, 0}, {2, 0} };

    while (stackSize > 0) {
        auto [x, y] = cellsStack[--stackSize];
        pair<int, int> neighbors[4];
        int neighborCount = 0;

        // Пошук сусідів
        for (auto [dx, dy] : directions) {
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < HEIGHT && ny < WIDTH && maze[nx][ny] == Cell::WALL) {
                neighbors[neighborCount++] = { nx, ny };
            }
        }

        if (neighborCount > 0) {
            auto [nx, ny] = neighbors[dis(gen) % neighborCount];

            maze[nx][ny] = Cell::PATH;

            int midX = (x + nx) / 2;
            int midY = (y + ny) / 2;
            maze[midX][midY] = Cell::PATH;

            cellsStack[stackSize++] = { x, y }; 
            cellsStack[stackSize++] = { nx, ny }; 
        }
    }

    maze[HEIGHT - 2][WIDTH - 2] = Cell::END;

    if (maze[HEIGHT - 3][WIDTH - 2] == Cell::WALL && maze[HEIGHT - 2][WIDTH - 3] == Cell::WALL) {
        maze[HEIGHT - 3][WIDTH - 2] = Cell::PATH;  // Гарантований шлях
    }
}

void drawMaze(sf::RenderWindow& window, sf::Font& font, Algorithm algorithm)
{
    sf::Text title;
    title.setString("PathFinding");
    title.setFont(font);
    title.setCharacterSize(20);
    title.setFillColor(sf::Color::Black);
    window.draw(title);

    sf::RectangleShape rectangle(sf::Vector2f(TILE_SIZE - 2, TILE_SIZE - 2));

    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            switch (maze[i][j])
            {
            case Cell::WALL:
                rectangle.setFillColor(sf::Color::Black);
                break;
            case Cell::PATH:
                rectangle.setFillColor(sf::Color::White);
                break;
            case Cell::START:
                rectangle.setFillColor(sf::Color::Green);
                break;
            case Cell::END:
                rectangle.setFillColor(sf::Color::Red);
                break;
            case Cell::VISITED:
                rectangle.setFillColor(sf::Color::Yellow);
                break;
            case Cell::SHORTEST:
                rectangle.setFillColor(sf::Color::Blue);
                break;
            default:
                break;
            }
            rectangle.setPosition(j * TILE_SIZE, i * TILE_SIZE);
            window.draw(rectangle);
        }
    }
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(17);
    infoText.setFillColor(sf::Color::White);

    infoText.setString("Algorithms:\n"
        "1: Depth First Search (DFS)\n"
        "2: Dijkstra\n"
        "3: A* Search\n"
        "Click 'Generate' to create a new maze");
    infoText.setPosition(10, HEIGHT * TILE_SIZE + 10);
    window.draw(infoText);

    sf::RectangleShape button(sf::Vector2f(120, 30));
    button.setFillColor(sf::Color::Green);
    button.setPosition(WIDTH * TILE_SIZE - 150, HEIGHT * TILE_SIZE + 30);

    sf::Text buttonText;
    buttonText.setFont(font);
    buttonText.setString("Generate");
    buttonText.setCharacterSize(14);
    buttonText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    buttonText.setPosition(button.getPosition().x + button.getSize().x / 2.0f, button.getPosition().y + button.getSize().y / 2.0f);

    window.draw(button);
    window.draw(buttonText);
}

bool isEndReached = false;
void dfs(int x, int y, sf::RenderWindow& window, sf::Font& font)
{
    if (x < 0 || y < 0 || x >= HEIGHT || y >= WIDTH || maze[x][y] == Cell::WALL || maze[x][y] == Cell::VISITED)
        return;

    if (maze[x][y] == Cell::END) {
        maze[x][y] = Cell::VISITED; 
        isEndReached = true;
        window.clear();
        drawMaze(window, font, Algorithm::DFS); 
        window.display();
        return;
    }

    maze[x][y] = Cell::VISITED; 

    window.clear();
    drawMaze(window, font, Algorithm::DFS); 
    window.display();
    this_thread::sleep_for(chrono::milliseconds(75)); 

    if (!isEndReached) dfs(x + 1, y, window, font);
    if (!isEndReached) dfs(x - 1, y, window, font);
    if (!isEndReached) dfs(x, y + 1, window, font);
    if (!isEndReached) dfs(x, y - 1, window, font);
}


void dijkstra(sf::RenderWindow& window, sf::Font& font)
{
    vector<vector<int>> dist(HEIGHT, vector<int>(WIDTH, numeric_limits<int>::max()));
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<>> pq;

    pair<int, int> start;
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            if (maze[i][j] == Cell::START)
            {
                start = { i, j };
                dist[i][j] = 0;
                pq.push({ 0, {i, j} });
                break;
            }
        }
    }

    vector<pair<int, int>> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    while (!pq.empty() && !isEndReached) 
    {
        auto [currentDist, pos] = pq.top();
        pq.pop();
        int x = pos.first, y = pos.second;

        if (maze[x][y] == Cell::END) {
            isEndReached = true;
            maze[x][y] = Cell::VISITED;  
            break;  
        }
        maze[x][y] = Cell::VISITED;

        for (auto [dx, dy] : directions)
        {
            int nx = x + dx, ny = y + dy;

            if (nx >= 0 && ny >= 0 && nx < HEIGHT && ny < WIDTH && maze[nx][ny] != Cell::WALL)
            {
                int newDist = currentDist + 1;
                if (newDist < dist[nx][ny])
                {
                    dist[nx][ny] = newDist;
                    pq.push({ newDist, {nx, ny} });
                }
            }
        }

        window.clear();
        drawMaze(window, font, Algorithm::DIJKSTRA);
        window.display();
        this_thread::sleep_for(chrono::milliseconds(75));  
    }
}

void aStar(sf::RenderWindow& window, sf::Font& font)
{
    vector<vector<int>> dist(HEIGHT, vector<int>(WIDTH, numeric_limits<int>::max()));
    vector<vector<int>> heuristic(HEIGHT, vector<int>(WIDTH, 0));
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<>> pq;

    pair<int, int> start, end;
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            if (maze[i][j] == Cell::START)
                start = { i, j };
            if (maze[i][j] == Cell::END)
                end = { i, j };
        }
    }

    auto manhattanDistance = [](pair<int, int> a, pair<int, int> b) {
        return abs(a.first - b.first) + abs(a.second - b.second);
        };

    dist[start.first][start.second] = 0;
    heuristic[start.first][start.second] = manhattanDistance(start, end);
    pq.push({ heuristic[start.first][start.second], start });

    vector<pair<int, int>> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    while (!pq.empty())
    {
        auto [currentPriority, pos] = pq.top();
        pq.pop();
        int x = pos.first, y = pos.second;

        if (maze[x][y] == Cell::END)
        {
            isEndReached = true;
            break;
        }

        maze[x][y] = Cell::VISITED;

        for (auto [dx, dy] : directions)
        {
            int nx = x + dx, ny = y + dy;

            if (nx >= 0 && ny >= 0 && nx < HEIGHT && ny < WIDTH && maze[nx][ny] != Cell::WALL)
            {
                int newDist = dist[x][y] + 1;
                if (newDist < dist[nx][ny])
                {
                    dist[nx][ny] = newDist;
                    int priority = newDist + manhattanDistance({ nx, ny }, end);
                    pq.push({ priority, {nx, ny} });
                }
            }
        }

        window.clear();
        drawMaze(window, font, Algorithm::ASTAR);
        window.display();
        this_thread::sleep_for(chrono::milliseconds(75));
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(TILE_SIZE * WIDTH, TILE_SIZE * HEIGHT + INFO_HEIGHT), "Algorithms Visualization");

    sf::Font font;
    font.loadFromFile("font.ttf");
    cout << "Executable Path: " << filesystem::current_path() << endl;

    generateMaze();

    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Num1)
                {
                    currentAlgo = Algorithm::DFS;
                    isEndReached = false;
                    for (int i = 0; i < HEIGHT; ++i)
                    {
                        for (int j = 0; j < WIDTH; ++j)
                        {
                            if (maze[i][j] == Cell::START)
                            {
                                dfs(i, j, window, font);
                                break;
                            }
                        }
                    }
                }
                else if (event.key.code == sf::Keyboard::Num2)
                {
                    currentAlgo = Algorithm::DIJKSTRA;
                    isEndReached = false;
                    dijkstra(window, font);
                }
                else if (event.key.code == sf::Keyboard::Num3)
                {
                    currentAlgo = Algorithm::ASTAR;
                    isEndReached = false;
                    aStar(window, font);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (mousePos.x >= WIDTH * TILE_SIZE - 150 && mousePos.x <= WIDTH * TILE_SIZE - 30 &&
                        mousePos.y >= HEIGHT * TILE_SIZE + 30 && mousePos.y <= HEIGHT * TILE_SIZE + 60)
                    {
                        generateMaze();
                    }
                }
            }
        }
        window.clear();
        drawMaze(window, font, currentAlgo);
        window.display();
    }
    return 0;
}