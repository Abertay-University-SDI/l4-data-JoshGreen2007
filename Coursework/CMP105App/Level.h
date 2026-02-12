#pragma once

#include <iomanip>
#include <fstream>

#include "Framework/BaseLevel.h"
#include "Framework/GameObject.h"
#include "Framework/Collision.h"
#include "Sheep.h"
#include "Rabbit.h"

class Level : BaseLevel {
public:
	Level(sf::RenderWindow& window, Input& input);
	~Level() override;

	void handleInput(float dt) override;
	void update(float dt) override;
	void render() override;

private:
    void UpdateCamera();
    bool CheckWinCondition();
    void manageCollisions();

    void writeHighScore(float time);

    sf::View m_cameraView;

    // Entities
    Rabbit* m_playerRabbit;
    std::vector<Sheep*> m_sheepList;
    std::vector<GameObject> m_walls; 
    GameObject m_goal;
    sf::Texture m_sheepTexture;
    sf::Texture m_rabbitTexture;
    GameObject m_bgFarm;

    // Level Data
    sf::FloatRect m_levelBounds;
    bool m_isGameOver;

    // UI & Timer
    sf::Clock m_gameTimer;
    sf::Font m_font;
    sf::Text m_timerText;
    sf::Text m_winText;
	
};