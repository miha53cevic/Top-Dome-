#include "world.h"
#include <iostream>
#include "../Engine/App.h"
#include "../Math/Math.h"

World::World(we::App* app, int winX, int winY)
    : m_textbox(app)
{
    m_winSize = { winX, winY };

    // Default values
    m_map = "";
    m_lives = 3;
    m_round = 0;
    m_killedEnemies = 0;
    m_bGameOver = false;
    
    m_fGravity = 50.f;

    // Center Lives text
    m_textbox.setString("Lives: X");
    m_textbox.setPosition(winX - m_textbox.getText().getLocalBounds().width / 1.5f,
                          winY - m_textbox.getText().getLocalBounds().height);
}

void World::loadMap(std::string path)
{
    std::ifstream reader(path);
    if (reader.is_open())
    {
        std::string line;
        m_mapSize = { 0, 0 };
        while (std::getline(reader, line))
        {
            // Find out the width and height of the map
            m_mapSize.x = line.length();
            m_mapSize.y++;

            // Save the map layout
            m_map += line;
        }
        reader.close();

        // Calculate each block size
        m_blockSize.x = m_winSize.x / m_mapSize.x;
        m_blockSize.y = m_winSize.y / m_mapSize.y;

        // Find and save the spawnpoint location
        for (int i = 0; i < m_mapSize.y; i++)
        {
            for (int j = 0; j < m_mapSize.x; j++)
            {
                if (getTileFromLocal(j, i) == 'S')
                    m_spawnpoint = sf::Vector2f(j * m_blockSize.x, i * m_blockSize.y);
                else if (getTileFromLocal(j, i) == 'E')
                    m_spawnerPos = sf::Vector2f(j * m_blockSize.x, i * m_blockSize.y);
            }
        }
        m_spawner.setPosition(m_spawnerPos);
    }
    else std::cout << "Could not load map " << path << std::endl;
}

void World::Update(float deltaTime)
{
    // Remove bullets outside of the map
    if (m_vecBullets.size() > 0)
    {
        for (auto iter = m_vecBullets.begin(); iter != m_vecBullets.end();)
        {
            iter->Update(deltaTime);

            if (getTileFromGlobal(iter->getPosition()) == '#')
            {
                iter = m_vecBullets.erase(iter);
            }
            else iter++;
        }
    }

#ifdef DEBUG
    std::cout << "Bullets: " << m_bullets.size() << "\n";
#endif

    // Round Manager
    if (m_killedEnemies < (int)(m_round * 1.5f))
    {
        if (m_spawner.getEnemyCount() <= 1)
        {
            // Spawn enemies
            if (Math::iRandom(0, 256) >= 200)
                m_spawner.SpawnEnemy(std::make_unique<Enemy>(this));
        }
    }
    else
    {
        // If the killed quota is hit go to next round
        m_round++;
    }
    m_spawner.Update(deltaTime);

    // GameOver Manager
    if (m_lives <= 0)
        m_bGameOver = true;

    // Bullet VS Enemy Collision
    if (!m_vecBullets.empty() && !m_spawner.getEnemiesVec()->empty())
    {
        for (auto enemyIter = m_spawner.getEnemiesVec()->begin(); enemyIter != m_spawner.getEnemiesVec()->end();)
        {
            bool Collision = false;

            sf::FloatRect enemy = enemyIter->get()->getEnemy()->getGlobalBounds();

            for (auto bulletIter = m_vecBullets.begin(); bulletIter != m_vecBullets.end();)
            {
                sf::FloatRect bullet = bulletIter->getBullet()->getGlobalBounds();

                if (bullet.intersects(enemy))
                {
                    // Damage the enemy
                    enemyIter->get()->Damage(1);
                   
                    if (enemyIter->get()->isDead())
                    {
                        //Chance to spawn powerUp
                        if (Math::iRandom(0, 256) >= 250)
                        {
                            //m_powerUp.Spawn((*iter)->getEnemy()->getPosition().x, (*iter)->getEnemy()->getPosition().y);
                        }

                        //Delete Enemy
                        enemyIter = m_spawner.getEnemiesVec()->erase(enemyIter);
                    }

                    //Delete Bullet
                    bulletIter = m_vecBullets.erase(bulletIter);

                    Collision = true;
                }
                else
                {
                    bulletIter++;
                }
            }
            if (!Collision)
            {
                enemyIter++;
            }
        }
    }
}

char World::getTileFromLocal(int x, int y)
{
    return m_map[(y * m_mapSize.x) + x];
}

char World::getTileFromGlobal(sf::Vector2f pos)
{
    int x = pos.x / m_blockSize.x;
    int y = pos.y / m_blockSize.y;
    return m_map[(y * m_mapSize.x) + x];
}

void World::Draw(sf::RenderWindow& window)
{
    // Draw the map
    sf::RectangleShape rect;
    for (int i = 0; i < m_mapSize.y; i++)
    {
        for (int j = 0; j < m_mapSize.x; j++)
        {
            if (getTileFromLocal(j, i) == '#')
            {
                rect.setSize(sf::Vector2f(m_blockSize.x, m_blockSize.y));
                rect.setPosition(j * m_blockSize.x, i * m_blockSize.y);
                rect.setFillColor(sf::Color(128, 128, 128));
                window.draw(rect);
            }
        }
    }

    // Draw the GUI
    m_textbox.setString("Lives: " + std::to_string(m_lives));
    m_textbox.Draw(window);

    // Draw Bullets
    if (m_vecBullets.size() > 0)
    {
        for (auto& i : m_vecBullets)
            i.Draw(&window);
    }

    // Draw enemies
    m_spawner.Draw(&window);
}

void World::setLives(int lives)
{
    m_lives = abs(lives);
}

int World::getLives()
{
    return m_lives;
}

int World::getRounds()
{
    return m_round;
}

void World::setRounds(int round)
{
    m_round = abs(round);
}

bool World::GameOver()
{
    return m_bGameOver;
}

float World::getGravity()
{
    return m_fGravity;
}

void World::setGravity(float g)
{
    m_fGravity = g;
}

std::vector<Bullet>* World::getBulletVector()
{
    return &m_vecBullets;
}

sf::Vector2i World::getWindowSize()
{
    return m_winSize;
}

sf::Vector2f World::getSpawnpoint()
{
    return m_spawnpoint;
}

sf::Vector2f World::getSpawnerPos()
{
    return m_spawnerPos;
}

Spawner * World::getSpawner()
{
    return &m_spawner;
}
