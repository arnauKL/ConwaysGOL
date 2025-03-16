#include <random>
#include <ctime>
#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>

namespace conf
{
    const int cell_size = 10;
    const int updates_per_second = 10;
}

void initialiseRandomGrid(int** grid, int** nextgrid, const int& rows, const int& cols)
{
    std::default_random_engine randomGenerator(time(nullptr));
    std::uniform_int_distribution<int> diceRoll(0, 1);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            const int value = diceRoll(randomGenerator);
            grid[i][j] = value;
            nextgrid[i][j] = value;
        }
    }
}

int countAliveCells(int** gameGrid, const int& x, const int& y, const int& rows, const int&  cols) {

    int count = 0;
    // Iterate through the 3x3 grid centered at (x, y)
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // Skip the center cell itself
            if (i == 0 && j == 0) continue;

            // Calculate the wrapped neighbor coordinates
            int neighborX = (x + i + rows) % rows;
            int neighborY = (y + j + cols) % cols;

            // Add the value of the neighbor cell (if its alive, it's 1, otherwise, 0)
            count += gameGrid[neighborX][neighborY];
        }
    }
    return count;
}

void updateGrid(int**& cellGrid, int**& nextGrid, bool** dirtyGrid, const int& rows, const int& cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            const int aliveCount = countAliveCells(cellGrid, i, j, rows, cols);

            if(cellGrid[i][j] == 1) // Cell is alive
            {
                if(aliveCount == 2 || aliveCount == 3)
                    nextGrid[i][j] = 1;
                else
                {
                    nextGrid[i][j] = 0;
                    dirtyGrid[i][j] = true;
                }
            }
            else // Cell is dead
            {
                if (aliveCount == 3)
                {
                    nextGrid[i][j] = 1;
                    dirtyGrid[i][j] = true;
                }
                else
                    nextGrid[i][j] = 0;
            }
        }
    }
    int** temp = cellGrid;
    cellGrid = nextGrid;
    nextGrid = temp;
}

void updateGeometry(const int index, const int i, const int j, sf::VertexArray& va, const sf::Color& c)
{
    const int vertex_index = 4 * index; // To find the index of the cell we are updating

    const float xpos = j * conf::cell_size;
    const float ypos = i * conf::cell_size;

    va[vertex_index + 0].position = sf::Vector2f(xpos, ypos);
    va[vertex_index + 1].position = sf::Vector2f(xpos + conf::cell_size, ypos);
    va[vertex_index + 2].position = sf::Vector2f(xpos + conf::cell_size, ypos + conf::cell_size);
    va[vertex_index + 3].position = sf::Vector2f(xpos, ypos + conf::cell_size);

    va[vertex_index + 0].color = c;
    va[vertex_index + 1].color = c;
    va[vertex_index + 2].color = c;
    va[vertex_index + 3].color = c;
}

void renderGrid(sf::RenderWindow& window, sf::VertexArray& va, const int& cols, const int& rows, int** gameGrid, bool** dirtyGrid)
{
    int index = 0;
    // Update geometry
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if(dirtyGrid[i][j])
            {
                if (gameGrid[i][j] == 1)
                    updateGeometry(index, i, j, va, sf::Color::White);
                else
                    updateGeometry(index, i, j, va, sf::Color::Black);
                dirtyGrid[i][j] = false;
            }
            index++;
        }
    }
    window.draw(va);
}

int main()
{
    auto window = sf::RenderWindow{ { 1920u, 1080u }, "Game of Life", sf::Style::Fullscreen };
    window.setFramerateLimit(200);

    const int rows = window.getSize().y / conf::cell_size;
    const int cols = window.getSize().x / conf::cell_size;

    // Primer declaram un punter que apuntarà a una sèrie de punters
    // I reservem la memòria pel nombre de files

    int** cellGrid = new int*[rows];
    int** nextGrid = new int*[rows];
    bool** dirtyGrid = new bool*[rows];

    // Ara, a dins de cadascuna d les files, reservam "cols" ammount d'enters per columnes
    for (int i = 0; i < rows; i++)
    {
        cellGrid[i] = new int[cols];
        nextGrid[i] = new int[cols];
        dirtyGrid[i] = new bool[cols];
    }

    initialiseRandomGrid(cellGrid, nextGrid, rows, cols);

    sf::VertexArray va = sf::VertexArray(sf::Quads, cols*rows*4);

    bool paused = false;
    bool drawing = false;

    // Measuring fps
    sf::Clock fpsClock;        // Clock to measure time per frame
    //float ticks = 0.0f;
    //int a = 0;
    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (event.key.code == sf::Keyboard::Space)
                    paused = !paused;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                drawing = true;
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
                drawing = false;
        }

        const float frameTime = fpsClock.getElapsedTime().asSeconds(); // Time for this frame
        //if (a > 100) { a = 0; std::cout << "MSPT: " << frameTime*1000 << std::endl; }
        //else a++;

        // Drawing
        if (drawing)
        {
            const int cellX = sf::Mouse::getPosition().x / conf::cell_size;
            const int cellY = sf::Mouse::getPosition().y / conf::cell_size;
            cellGrid[cellY][cellX] = 1;
            dirtyGrid[cellY][cellX] = true;
        }


        if (frameTime > (1.0f / conf::updates_per_second) && !paused)
        {
            updateGrid(cellGrid, nextGrid, dirtyGrid, rows, cols);
            fpsClock.restart();
        }
        window.clear();
        renderGrid(window, va, cols, rows, cellGrid, dirtyGrid);
        window.display();
    }

    for (int i = 0; i < rows; ++i) {
        delete[] nextGrid[i];
        delete[] cellGrid[i];
        delete[] dirtyGrid[i];
    }
    delete[] nextGrid;
    delete[] cellGrid;
    delete[] dirtyGrid;
}

