#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <iostream>
#include <limits>
#include <thread>
#include <chrono>
#include <random>
#include <set>

const int TILE_SIZE = 30;
const int WIDTH = 20;
const int HEIGHT = 20;

enum class Cell { WALL, PATH, START, END, VISITED, SHORTEST };
enum class Algorithm { NONE, DFS, DIJKSTRA, INSERTION_SORT };

std::vector<std::vector<Cell>> maze(HEIGHT, std::vector<Cell>(WIDTH, Cell::WALL));

sf::RenderWindow window(sf::VideoMode(TILE_SIZE* WIDTH, TILE_SIZE* HEIGHT), "Maze Visualization");

Algorithm currentAlgo = Algorithm::NONE;

void generateMaze() {
    // Спочатку заповнюємо лабіринт стінами
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            maze[i][j] = Cell::WALL;
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3); // для вибору напрямку випадково

    std::vector<std::pair<int, int>> cellsStack; // Стек для зберігання шляху
    int startX = 1, startY = 1;
    maze[startX][startY] = Cell::START;
    cellsStack.push_back({ startX, startY });

    // Напрямки: вверх, вниз, вліво, вправо (з парним кроком)
    std::vector<std::pair<int, int>> directions = { {0, -2}, {0, 2}, {-2, 0}, {2, 0} };

    while (!cellsStack.empty()) {
        auto [x, y] = cellsStack.back();
        std::vector<std::pair<int, int>> neighbors;

        // Знаходимо всі доступні сусідні клітини
        for (auto [dx, dy] : directions) {
            int nx = x + dx, ny = y + dy;
            if (nx > 0 && ny > 0 && nx < HEIGHT - 1 && ny < WIDTH - 1 && maze[nx][ny] == Cell::WALL) {
                neighbors.push_back({ nx, ny });
            }
        }

        if (!neighbors.empty()) {
            // Вибираємо випадкового сусіда
            auto [nx, ny] = neighbors[dis(gen)];

            // Робимо шлях між поточною клітиною та вибраним сусідом
            maze[nx][ny] = Cell::PATH;
            maze[(x + nx) / 2][(y + ny) / 2] = Cell::PATH;

            // Додаємо сусіда до стеку
            cellsStack.push_back({ nx, ny });
        }
        else {
            // Якщо немає доступних сусідів, повертаємося назад
            cellsStack.pop_back();
        }
    }

    // Встановлюємо кінцеву точку на іншому кінці лабіринту
    maze[HEIGHT - 2][WIDTH - 2] = Cell::END;
}


void drawMaze(sf::RenderWindow& window, sf::Font& font, Algorithm algorithm)
{
    sf::Text title;
    title.setString("PathFinding");
    title.setFont(font);
    title.setCharacterSize(35);
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
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);

    text.setString("DFS Algorithm");
    text.setPosition(630, 60);
    window.draw(text);

    text.setString("Num1");
    text.setPosition(660, 90);
    window.draw(text);

    text.setString("Dijkstra Algorithm");
    text.setPosition(630, 160);
    window.draw(text);

    text.setString("Num2");
    text.setPosition(660, 190);
    window.draw(text);

    text.setString("Insertion Sort");
    text.setPosition(630, 260);
    window.draw(text);

    text.setString("Num3");
    text.setPosition(660, 290);
    window.draw(text);
}

void insertionSort(sf::RenderWindow& window, sf::Font& font)
{
    std::vector<int> arr = { 5, 2, 9, 1, 5, 6 };
    for (int i = 1; i < arr.size(); ++i)
    {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;

        // Візуалізація
        window.clear();
        drawMaze(window, font, Algorithm::INSERTION_SORT);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void dfs(int x, int y, sf::RenderWindow& window, sf::Font& font)
{
    if (x < 0 || y < 0 || x >= HEIGHT || y >= WIDTH || maze[x][y] == Cell::WALL || maze[x][y] == Cell::VISITED)
        return;
    

    if (maze[x][y] == Cell::END)
        return; 

    maze[x][y] = Cell::VISITED;

    // Візуалізація
    window.clear();
    drawMaze(window, font, Algorithm::DFS);
    window.display();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    dfs(x + 1, y, window, font); // вниз
    dfs(x - 1, y, window, font); // вгору
    dfs(x, y + 1, window, font); // вправо
    dfs(x, y - 1, window, font); // вліво
}

void dijkstra(sf::RenderWindow& window, sf::Font& font)
{
    std::vector<std::vector<int>> dist(HEIGHT, std::vector<int>(WIDTH, std::numeric_limits<int>::max()));
    std::priority_queue<std::pair<int, std::pair<int, int>>, std::vector<std::pair<int, std::pair<int, int>>>, std::greater<>> pq;

    // Знаходимо стартову позицію
    std::pair<int, int> start;
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

    std::vector<std::pair<int, int>> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    while (!pq.empty())
    {
        auto [currentDist, pos] = pq.top();
        pq.pop();
        int x = pos.first, y = pos.second;

        if (maze[x][y] == Cell::END)
            break; // Дійшли до кінцевої точки

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

        // Візуалізація
        window.clear();
        drawMaze(window, font, Algorithm::DIJKSTRA);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Algorithms Visualization");

    sf::Font font;
    font.loadFromFile("arial.ttf");

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
                    // Виклик DFS
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
                    // Виклик Dijkstra
                    dijkstra(window, font);
                }
                else if (event.key.code == sf::Keyboard::Num3)
                {
                    insertionSort(window, font);
                }
            }
        }

        window.clear();
        drawMaze(window, font, currentAlgo);
        window.display();
    }
    return 0;
}