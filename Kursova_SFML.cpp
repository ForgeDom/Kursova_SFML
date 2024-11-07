#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <stack>
#include <iostream>
#include <limits>
#include <algorithm>

const int TILE_SIZE = 30;
const int WIDTH = 20;
const int HEIGHT = 15;

enum class Cell { WALL, PATH, START, END, VISITED, SHORTEST };

std::vector<std::vector<Cell>> maze = {
    {Cell::START, Cell::PATH, Cell::WALL,  Cell::PATH, Cell::END },
    {Cell::WALL,  Cell::PATH, Cell::WALL,  Cell::PATH, Cell::WALL},
    {Cell::PATH,  Cell::PATH, Cell::PATH,  Cell::PATH, Cell::PATH},
    {Cell::PATH,  Cell::WALL, Cell::WALL,  Cell::PATH, Cell::WALL},
    {Cell::PATH,  Cell::PATH, Cell::PATH,  Cell::PATH, Cell::PATH},
};

struct Node {
    int x, y, distance;

    Node(int _x, int _y, int _dist) : x(_x), y(_y), distance(_dist) {}

    bool operator>(const Node& other) const {
        return distance > other.distance;
    }
};

std::vector<sf::Vector2i> directions = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

bool inBounds(int x, int y) {
    return x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH;
}

void drawMaze(sf::RenderWindow& window) {
    sf::RectangleShape cell(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            switch (maze[i][j]) {
            case Cell::WALL:     cell.setFillColor(sf::Color::Black); break;
            case Cell::PATH:     cell.setFillColor(sf::Color::White); break;
            case Cell::START:    cell.setFillColor(sf::Color::Green); break;
            case Cell::END:      cell.setFillColor(sf::Color::Red); break;
            case Cell::VISITED:  cell.setFillColor(sf::Color::Blue); break;
            case Cell::SHORTEST: cell.setFillColor(sf::Color::Yellow); break;
            }
            cell.setPosition(j * TILE_SIZE, i * TILE_SIZE);
            window.draw(cell);
        }
    }
}

bool dfs(int x, int y) {
    if (!inBounds(x, y) || maze[x][y] == Cell::WALL || maze[x][y] == Cell::VISITED)
        return false;
    if (maze[x][y] == Cell::END) return true;

    maze[x][y] = Cell::VISITED;

    for (auto [dx, dy] : directions) {
        if (dfs(x + dx, y + dy)) return true;
    }

    return false;
}

void dijkstra(sf::Vector2i start, sf::Vector2i end) {
    std::vector<std::vector<int>> dist(HEIGHT, std::vector<int>(WIDTH, std::numeric_limits<int>::max()));
    dist[start.x][start.y] = 0;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    pq.emplace(start.x, start.y, 0);

    while (!pq.empty()) {
        auto [x, y, d] = pq.top();
        pq.pop();

        if (maze[x][y] == Cell::END) break;
        if (maze[x][y] != Cell::START) maze[x][y] = Cell::VISITED;

        for (auto [dx, dy] : directions) {
            int nx = x + dx, ny = y + dy;
            if (inBounds(nx, ny) && maze[nx][ny] != Cell::WALL && dist[nx][ny] > d + 1) {
                dist[nx][ny] = d + 1;
                pq.emplace(nx, ny, dist[nx][ny]);
            }
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE), "Maze Solver");

    sf::Vector2i start(0, 0);
    sf::Vector2i end(0, 4);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear();
        drawMaze(window);
        window.display();

        dfs(start.x, start.y);

        dijkstra(start, end);
    }

    return 0;
}