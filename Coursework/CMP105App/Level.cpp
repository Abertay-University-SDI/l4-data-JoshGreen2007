#include "Level.h"

Level::Level(sf::RenderWindow& hwnd, Input& in) :
    BaseLevel(hwnd, in), m_timerText(m_font), m_winText(m_font), m_highScoreText(m_font)
{
    m_isGameOver = false;

    sf::Vector2f levelSize = { 800.f, 500.f };
    m_levelBounds = { {0.f, 0.f}, {levelSize } };

    // Initialise the Player (Rabbit)
    m_playerRabbit = new Rabbit();
    m_playerRabbit->setPosition({ 40.f, 40.f });  // hardcoded. FOR NOW!
    if (!m_rabbitTexture.loadFromFile("gfx/rabbit_sheet.png")) std::cerr << "no rabbit texture";
    if (!m_sheepTexture.loadFromFile("gfx/sheep_sheet.png")) std::cerr << "no sheep texture";
    m_playerRabbit->setSize({ 32,32 });
    m_playerRabbit->setInput(&m_input);
    m_playerRabbit->setTexture(&m_rabbitTexture);
    m_playerRabbit->setWorldSize(levelSize.x, levelSize.y);


    // Initialise Sheep (initial position AND pointer to the rabbit
    for (int i = 0; i < 3; i++)
    {
        m_sheepList.push_back(new Sheep(sf::Vector2f(200.f + 100 * i, 400.f - 100 * i), m_playerRabbit));
        m_sheepList[i]->setTexture(&m_sheepTexture);
        m_sheepList[i]->setSize({ 32,32 });
        m_sheepList[i]->setWorldSize(levelSize.x, levelSize.y);
    }

    // Initialise Timer
    m_gameTimer.restart();

    // UI Setup
    if (!m_font.openFromFile("font/arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }

    m_timerText.setFont(m_font);
    m_timerText.setCharacterSize(24);
    m_timerText.setFillColor(sf::Color::White);

    // "Game Over" text setup
    m_winText.setFont(m_font);
    m_winText.setString("ROUND COMPLETE!");
    m_winText.setCharacterSize(50);
    m_winText.setFillColor(sf::Color::Blue);
    m_winText.setPosition({ -1000.f, 100.f });  // outside of view

    m_highScoreText.setFont(m_font);
    m_highScoreText.setString("High Score: ");
    m_highScoreText.setCharacterSize(50);
    m_highScoreText.setFillColor(sf::Color::Blue);
    m_highScoreText.setPosition({ 100.f, 100.f });


    // setup goal
    m_goal.setSize({ 50, 50 });
    m_goal.setFillColor(sf::Color::Blue);
    m_goal.setPosition({ 250, 250 });
    m_goal.setCollisionBox({ { 0,0 }, { 50,50 } });

    // setup walls
    for (int i = 0; i < 2; i++)
    {
        GameObject wall;
        wall.setPosition({ 100.f + i * 600, 100.f});
        wall.setSize({ 50,300 });
        wall.setFillColor(sf::Color::Black);
        wall.setCollisionBox({ { 0,0 }, { 50,300 } });
        m_walls.push_back(wall);
    }

    m_bgFarm.setFillColor(sf::Color::Green);
    m_bgFarm.setSize(levelSize);
    

}

Level::~Level()
{
	// We made a lot of **new** things, allocating them on the heap
	// So now we have to clean them up
	delete m_playerRabbit;
	for (Sheep* s : m_sheepList)
	{
		delete s;
	}
	m_sheepList.clear();
}

void Level::UpdateCamera()
{
    sf::View view = m_window.getView();
    sf::Vector2f targetPos = m_playerRabbit->getPosition(); 

    sf::Vector2f viewSize = view.getSize();
    sf::Vector2f halfSize = { viewSize.x / 2.f, viewSize.y / 2.f };

    sf::Vector2f newCenter;

    newCenter.x = std::clamp(targetPos.x,
        m_levelBounds.position.x + halfSize.x,
        m_levelBounds.size.x - halfSize.x);

    newCenter.y = std::clamp(targetPos.y,
        m_levelBounds.position.y + halfSize.y,
        m_levelBounds.size.y - halfSize.y);

    view.setCenter(newCenter);
    
    // top-left corner of the current view in world space
    sf::Vector2f viewTopLeft = newCenter - halfSize;
    // Position text at the bottom-left of the current view with a small margin (e.g., 30px)
    float margin = 30.f;
    m_timerText.setPosition({ viewTopLeft.x + margin, viewTopLeft.y + viewSize.y - margin });

    m_window.setView(view);
}

bool Level::CheckWinCondition()
{
    for (auto s : m_sheepList) if (s->isAlive()) return false;
    m_winText.setPosition({ 100.f, 100.f });

    displayScoreboard();
 
    return true;
}


// handle user input
void Level::handleInput(float dt)
{
	m_playerRabbit->handleInput(dt);
}

// checks and manages sheep-sheep, sheep-wall, sheep-goal, player-wall
void Level::manageCollisions()
{
    for (int i = 0; i < m_sheepList.size(); i++)
    {
        if (!m_sheepList[i]->isAlive()) continue;   // ignore scored sheep.

        // sheep collide with walls, with other sheep and with the goal.
        for (auto wall : m_walls)
        {
            if (Collision::checkBoundingBox(wall, *m_sheepList[i]))
            {
                m_sheepList[i]->collisionResponse(wall);
            }
        }
        for (int j = i + 1; j < m_sheepList.size(); j++)
        {
            if(!m_sheepList[j]->isAlive()) continue; // ignore scored sheep here too
            if (Collision::checkBoundingBox(*m_sheepList[i], *m_sheepList[j]))
            {
                // DANGER check i and j carefully here team.
                m_sheepList[i]->collisionResponse(*m_sheepList[j]);
                m_sheepList[j]->collisionResponse(*m_sheepList[i]);
            }
        }
        if (Collision::checkBoundingBox(*m_sheepList[i], m_goal))
            m_sheepList[i]->collideWithGoal(m_goal);
    }
    for (auto wall : m_walls)
    {
        if (Collision::checkBoundingBox(wall, *m_playerRabbit))
        {
            m_playerRabbit->collisionResponse(wall);
        }
    }
}

void Level::writeHighScore(float time)
{

    std::ofstream file_to_write_to("data/highScore.txt", std::ios::app);
    if (file_to_write_to.is_open())
        file_to_write_to << std::fixed << std::setprecision(2) << time << "\n";
    else
        std::cerr << "ERROR: Failed to open highScore.txt";
    file_to_write_to.close();

}

void Level::displayScoreboard()
{

    std::ifstream inputFile("data/highScore.txt");
    std::vector<std::string> highScores;
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open highScore.txt";
        m_highScoreText.setString("No highscores yet!");
        return;

    }


    std::string line;
    std::string scoreboardString = "High Scores:\n";

    while (std::getline(inputFile, line))
    {
        scoreboardString += line + "\n";
    }

    inputFile.close();

    m_highScoreText.setString(scoreboardString);
    m_highScoreText.setPosition({ 100.f, 160.f });

}

// Update game objects
void Level::update(float dt)
{
    if (m_isGameOver) return;   // if the game is over, don't continue trying to process game logic

    m_playerRabbit->update(dt);
    for (Sheep* s : m_sheepList)
    {
        if (s->isAlive()) s->update(dt);
    }

    // Timer 
    float timeElapsed = m_gameTimer.getElapsedTime().asSeconds();
    m_timerText.setString("Time: " + std::to_string(static_cast<int>(timeElapsed)));


    manageCollisions();
    UpdateCamera();
    if (CheckWinCondition() && !m_isGameOver)
    {

        m_isGameOver = true;
        writeHighScore(timeElapsed);

    }

}

// Render level
void Level::render()
{
	beginDraw();
    m_window.draw(m_bgFarm);
    m_window.draw(m_goal);
    for (auto w : m_walls) m_window.draw(w);
    for (auto s : m_sheepList)
    {
        if (s->isAlive()) m_window.draw(*s);
    }
    m_window.draw(*m_playerRabbit);
    m_window.draw(m_timerText);
    if (m_isGameOver)
    {
        m_window.draw(m_winText);
        m_window.draw(m_highScoreText);
    }
    
	endDraw();
}


