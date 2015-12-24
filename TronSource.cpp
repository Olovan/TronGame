#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>

int windowHeight = 1080;
int windowWidth = 1920;

bool gameOver = false;
bool scoreChanged = false;

sf::Text text;
sf::Font font;

int playerTextSize = 20;
sf::Text player1Text;
sf::Vector2f player1TextPosition(150,150);
sf::Text player2Text;
sf::Vector2f player2TextPosition(150,180);

int player1Score = 0;
int player2Score = 0;

bool player1IsAI = true;
float player1Size = 1;
float player1Velocity = 10;
sf::Vector2f player1InitialPos( 20, windowHeight / 2 );
sf::Vector2f player1InitialVelocity{ player1Velocity, 0 };
sf::Color player1Color(sf::Color::Yellow);

bool player2IsAI = true;
float player2Size = 1;
float player2Velocity = 10;
sf::Vector2f player2InitialPos(windowWidth - 20 , windowHeight / 2);
sf::Vector2f player2InitialVelocity{ -player2Velocity, 0 };
sf::Color player2Color(sf::Color::Green);


int aiRandomTurns = 120; //higher means it's more rare for the AI to turn



class Trail
{
public:
	sf::RectangleShape shape;
	sf::Vector2f startingPosition;
	sf::Vector2f middleOrigin;
	float size = player1Size;
	void InitializeTrail(sf::Color declareColor, sf::Vector2f sPos) //Used for controlling the main Trail
	{
		shape.setFillColor(declareColor);
		startingPosition = sPos;
	}
	Trail() //Empty Constructor for convenience. Will use Initialize Trail to set variables
	{

	}

	Trail(sf::Color color, sf::Vector2f origin, sf::Vector2f position, sf::Vector2f size) //Constructor for building fully finished trails
	{
		shape.setFillColor(color);
		shape.setOrigin(origin);
		shape.setPosition(position);
		shape.setSize(size);
		middleOrigin = sf::Vector2f(shape.getSize().x / 2, shape.getSize().y / 2);
	}

	void SetPosition(sf::Vector2f pos)
	{
		shape.setPosition(pos); //Move the trail over to the point
		if (pos.x - startingPosition.x == 0) //If you're moving in the Y direction
		{
			shape.setSize(sf::Vector2f(size, abs(startingPosition.y - pos.y))); //Set the trail size to stretch to new position
			if (pos.y > startingPosition.y) //If you're moving Down
			{
				shape.setOrigin(size / 2, pos.y - startingPosition.y); //Set origin to correct location
			}
			else // If you're moving Up
			{
				shape.setOrigin(size / 2, 0); //Set origin to correct location
			}
		}
		else //Otherwise you must be moving in the X direction
		{
			shape.setSize(sf::Vector2f(abs(startingPosition.x - pos.x), size)); //Set the size to stretch to the point given
			if (pos.x > startingPosition.x) //If you're moving Right
			{
				shape.setOrigin(pos.x - startingPosition.x, size / 2);
			}
			else //If you're moving left
			{
				shape.setOrigin(0, size / 2);
			}
		}
	}

	float top() { return shape.getPosition().y - shape.getOrigin().y; }
	float bottom() { return shape.getPosition().y - shape.getOrigin().y + shape.getSize().y; }
	float left() { return shape.getPosition().x - shape.getOrigin().x; }
	float right() { return shape.getPosition().x - shape.getOrigin().x + shape.getSize().x; }

};

std::vector<Trail> oldTrails; //All old trails for all players

class Player{
public:
	float referenceVelocity;
	sf::Color color = sf::Color::Cyan;
	bool velocityHasChanged = false;
	sf::Vector2f aiCheckOffset = sf::Vector2f(0, 0);
	bool isDead = false;
	bool isPlayer2 = false;
	bool isAI = true;
	bool turnedLeft = false;
	Trail trail;
	//std::vector<Trail> oldTrails;
	sf::RectangleShape shape;
	sf::Vector2f velocity{ player2InitialVelocity };

	Player()
	{
		shape.setSize(sf::Vector2f(player2Size, player2Size));
		shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);
		shape.setPosition(player2InitialPos);
		shape.setFillColor(color);
		trail.InitializeTrail(color, sf::Vector2f(shape.getPosition().x - shape.getSize().x / 2, shape.getPosition().y));
		referenceVelocity = player2Velocity;
	}

	Player(float playerSize, sf::Vector2f playerInitialPos, sf::Color icolor, float refVelocity, sf::Vector2f initVel)
	{
		shape.setSize(sf::Vector2f(playerSize, playerSize));
		shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);
		shape.setPosition(playerInitialPos);
		color = icolor;
		shape.setFillColor(icolor);
		trail.size = playerSize;
		trail.InitializeTrail(icolor, sf::Vector2f(shape.getPosition().x - shape.getSize().x / 2, shape.getPosition().y));
		referenceVelocity = refVelocity;
		velocity = initVel;
	}

	void reset(float playerSize, sf::Vector2f playerInitialPos, sf::Color icolor, float refVelocity, sf::Vector2f initVel)
	{
		shape.setSize(sf::Vector2f(playerSize, playerSize));
		shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);
		shape.setPosition(playerInitialPos);
		color = icolor;
		shape.setFillColor(icolor);
		trail.InitializeTrail(icolor, sf::Vector2f(shape.getPosition().x - shape.getSize().x / 2, shape.getPosition().y));
		referenceVelocity = refVelocity;
		velocity = initVel;
	}

	void Update()
	{
		if (gameOver)
		{
			if (isDead)
				shape.setFillColor(sf::Color::White);
			return; //Don't move or do anything if the player has died
		}

		if (!isAI)
		{
			if (!isPlayer2)
				velocity = CheckInput1(velocityHasChanged); //Input for Human Players
			else
				velocity = CheckInput2(velocityHasChanged); //Input for Human Players
		}
		shape.move(velocity);
		ManageTrail();

	}

	sf::Vector2f CheckInput1(bool& vChanged)
	{
		if (!gameOver)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && velocity != sf::Vector2f(0, -referenceVelocity) && velocity != sf::Vector2f(0, referenceVelocity))
			{
				velocity = sf::Vector2f(0, -referenceVelocity);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && velocity != sf::Vector2f(0, -referenceVelocity) && velocity != sf::Vector2f(0, referenceVelocity))
			{
				velocity = sf::Vector2f(0, referenceVelocity);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && velocity != sf::Vector2f(-referenceVelocity, 0) && velocity != sf::Vector2f(referenceVelocity, 0))
			{
				velocity = sf::Vector2f(-referenceVelocity, 0);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && velocity != sf::Vector2f(-referenceVelocity, 0) && velocity != sf::Vector2f(referenceVelocity, 0))
			{
				velocity = sf::Vector2f(referenceVelocity, 0);
				vChanged = true;
			}
			else
			{
				vChanged = false;
			}
		}
		else
		{
			velocity = sf::Vector2f(0, 0);
		}

		return velocity;
	}

	sf::Vector2f CheckInput2(bool& vChanged)
	{
		if (!gameOver)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && velocity != sf::Vector2f(0, -referenceVelocity) && velocity != sf::Vector2f(0, referenceVelocity))
			{
				velocity = sf::Vector2f(0, -referenceVelocity);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && velocity != sf::Vector2f(0, -referenceVelocity) && velocity != sf::Vector2f(0, referenceVelocity))
			{
				velocity = sf::Vector2f(0, referenceVelocity);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && velocity != sf::Vector2f(-referenceVelocity, 0) && velocity != sf::Vector2f(referenceVelocity, 0))
			{
				velocity = sf::Vector2f(-referenceVelocity, 0);
				vChanged = true;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && velocity != sf::Vector2f(-referenceVelocity, 0) && velocity != sf::Vector2f(referenceVelocity, 0))
			{
				velocity = sf::Vector2f(referenceVelocity, 0);
				vChanged = true;
			}
			else
			{
				vChanged = false;
			}
		}
		else //If you are dead then don't move
		{
			velocity = sf::Vector2f(0, 0);
		}

		return velocity;
	}

	void ManageTrail()
	{
		if (velocityHasChanged)
		{
			//Copy the current trail into an oldTrail and then reposition the trail
			oldTrails.emplace_back(color, trail.shape.getOrigin(), trail.shape.getPosition(), trail.shape.getSize()); //Generate the clone oldTrail
			if (velocity.x > 0) //If you're moving Right
				trail.startingPosition = sf::Vector2f(shape.getPosition().x - shape.getSize().x / 2 - velocity.x, shape.getPosition().y - velocity.y);
			else if (velocity.x < 0) //If you're moving Left
				trail.startingPosition = sf::Vector2f(shape.getPosition().x + shape.getSize().x / 2 - velocity.x, shape.getPosition().y - velocity.y);
			else if (velocity.y > 0) //If you're moving Down
				trail.startingPosition = sf::Vector2f(shape.getPosition().x - velocity.x, shape.getPosition().y - shape.getSize().y / 2 - velocity.y);
			else if (velocity.y < 0) //If you're moving Up
				trail.startingPosition = sf::Vector2f(shape.getPosition().x - velocity.x, shape.getPosition().y + shape.getSize().y / 2 - velocity.y);

			extendTrail();
		}
		else
		{
			//Extend current trail
			extendTrail();

		}
	}

	void extendTrail()
	{
		if (velocity.x != 0) //If you're moving Right or Left
		{
			if (velocity.x > 0) //If you're moving right
			{
				trail.SetPosition(sf::Vector2f(shape.getPosition().x - shape.getSize().x / 2, shape.getPosition().y));
			}
			else if (velocity.x < 0) //If you're moving left
			{
				trail.SetPosition(sf::Vector2f(shape.getPosition().x + shape.getSize().x / 2, shape.getPosition().y));
			}
		}
		else if (velocity.y != 0) //If you're moving Up or Down
		{
			if (velocity.y > 0) //If you're moving Down
			{
				trail.SetPosition(sf::Vector2f(shape.getPosition().x, shape.getPosition().y - shape.getSize().y / 2));
			}
			else if (velocity.y < 0) //If you're moving Up
			{
				trail.SetPosition(sf::Vector2f(shape.getPosition().x, shape.getPosition().y + shape.getSize().y / 2));
			}
		}
	}

	float top() { return shape.getPosition().y - shape.getOrigin().y; }
	float bottom() { return shape.getPosition().y - shape.getOrigin().y + shape.getSize().y; }
	float left() { return shape.getPosition().x - shape.getOrigin().x; }
	float right() { return shape.getPosition().x - shape.getOrigin().x + shape.getSize().x; }
};

template <class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() > mB.left() && mA.left() < mB.right() && mA.top() < mB.bottom() && mA.bottom() > mB.top();
}

template <class T1> bool mapCheck(T1& player)
{
	if (player.left() < 0 || player.right() > windowWidth || player.top() < 0 || player.bottom() > windowHeight)
	{
		return true;
	}
	else { return false; }
}

bool aiCheck(Player& aiPlayer, sf::Vector2f aiOffset, Player& otherPlayer) //Return true for hitting something or false for not hitting something
{
	sf::Vector2f oldPosition = aiPlayer.shape.getPosition(); //Save the player's original position
	aiPlayer.shape.setPosition(oldPosition + aiOffset); //Move the player to the position you want to test
	if (mapCheck(aiPlayer))
	{
		aiPlayer.shape.setPosition(oldPosition); //Put the player back where he was originally
		return true;
	}
	bool futureDeath = false;
	futureDeath = (isIntersecting(aiPlayer, otherPlayer.trail) || isIntersecting(aiPlayer,otherPlayer)); //Test against other player's trail and shape
	if (futureDeath)
	{
		aiPlayer.shape.setPosition(oldPosition); //Put the player back where he was originally
		return true;
	}
	for (Trail trail : oldTrails)
	{
		futureDeath = isIntersecting(aiPlayer, trail);
		if (futureDeath)
		{
			aiPlayer.shape.setPosition(oldPosition); //Put the player back where he was originally
			return true;
		}
	}
	aiPlayer.shape.setPosition(oldPosition); //Put the player back where he was originally
	return false;
}

void collisionCheck(Player& player1, Player& player2)
{
	if (!player1.isDead && !player2.isDead)
	{
		player1.isDead = mapCheck(player1);
		player2.isDead = mapCheck(player2);
	}
	if (!player1.isDead && !player2.isDead)
	{
		player1.isDead = (isIntersecting(player1, player2.trail)||isIntersecting(player1,player2)); //check to see if player 1 hit Player 2's trail or body
		player2.isDead = (isIntersecting(player2, player1.trail) || isIntersecting(player2, player1)); //check to see if player 1 hit Player 2's trail or body
		if (!player1.isDead && !player2.isDead) //If nobody is dead then keep checking
		{
			for (Trail& trail : oldTrails)
			{
				player1.isDead = isIntersecting(player1, trail);
				player2.isDead = isIntersecting(player2, trail);
				if (player1.isDead || player2.isDead) //If the player is dead stop checking
					break;
			}
		}
	}
	if (player1.isDead || player2.isDead)
		gameOver = true;
}

void victoryCheck(sf::RenderWindow&	window, Player& player1, Player player2)
{
	if (gameOver)
	{
		if (player1.isDead) //If player 1 did not win then change the string to say that player 2 won
		{
			if (!scoreChanged)
			{
				player2Score += 1;
				scoreChanged = true;
			}
			text.setString("Player 2 Wins");
		}
		if (player2.isDead)
		{
			if (!scoreChanged)
			{
				player1Score += 1;
				scoreChanged = true;
			}
			text.setString("Player 1 Wins");
		}
		if (player2.isDead && player1.isDead)
			text.setString("Both Players Dead");
		window.draw(text);
	}
}

void aiTurn(sf::Vector2f& aiOffset, bool goLeft, Player& aiPlayer)
{
	aiPlayer.turnedLeft = goLeft;
	if (aiOffset.x > 0)
	{
		aiOffset = goLeft ? sf::Vector2f(0, -aiOffset.x) : sf::Vector2f(0, aiOffset.x); //If moving right turn either up or down
	}
	else if (aiOffset.x < 0)
	{
		aiOffset = goLeft ? sf::Vector2f(0, aiOffset.x) : sf::Vector2f(0, -aiOffset.x);
	}
	else if (aiOffset.y > 0)
	{
		aiOffset = goLeft ? sf::Vector2f(aiOffset.y, 0) : sf::Vector2f(-aiOffset.y, 0); //If moving down then go either right or left
	}
	else
	{
		aiOffset = goLeft ? sf::Vector2f(-aiOffset.y, 0) : sf::Vector2f(aiOffset.y, 0);
	}
}

void aiManager(Player& aiPlayer, Player& otherPlayer)
{
	aiPlayer.velocityHasChanged = false;
	sf::Vector2f aiOffset = aiPlayer.velocity;
	bool willHitWall = aiCheck(aiPlayer, aiOffset, otherPlayer);
	if (willHitWall)
	{
		aiPlayer.velocityHasChanged = true; //Make sure the player object knows it has turned
		bool goLeft = rand() % 2; //Randomly decide to turn right or left
		aiTurn(aiOffset, !aiPlayer.turnedLeft, aiPlayer); //Go opposite direction of most recent turn
		bool willHitWall2 = aiCheck(aiPlayer, aiOffset, otherPlayer); //Check again with new position
		if (willHitWall2)
		{
			aiPlayer.turnedLeft = !aiPlayer.turnedLeft;
			aiOffset = sf::Vector2f(-aiOffset.x, -aiOffset.y); //If you hit AGAIN then choose the only remaining direction and continue
		}
	}

	if (!willHitWall && rand() % aiRandomTurns == 0) //If the AI is safe then let him randomly decide to turn sometimes
	{
		aiTurn(aiOffset, !aiPlayer.turnedLeft, aiPlayer); //Generate new direction

		if (aiCheck(aiPlayer, aiOffset, otherPlayer)) //Make sure new direction doesn't kill AI
		{
			aiPlayer.turnedLeft = !aiPlayer.turnedLeft;
			aiOffset = aiPlayer.velocity; //If the random turn would kill the player then ignore and resume going forward
		}
		else //If the new direction is safe then go ahead and turn
		{
			aiPlayer.velocityHasChanged = true; //Make sure the player object knows it has turned
		}
	}

	aiPlayer.velocity = aiOffset;
}

void reset(Player& player1, Player& player2)
{
	player1.reset(player1Size, player1InitialPos, player1Color, player1Velocity, player1InitialVelocity);
	player1.isDead = false;
	player2.reset(player2Size, player2InitialPos, player2Color, player2Velocity, player2InitialVelocity);
	player2.isDead = false;
	oldTrails.clear();
	gameOver = false;
	scoreChanged = false;
}

void drawScore(sf::RenderWindow& window)
{
	player1Text.setString("Player1 Score: " + std::to_string(player1Score));
	player2Text.setString("Player2 Score: " + std::to_string(player2Score));
	window.draw(player1Text);
	window.draw(player2Text);
}

int main()
{
	//create Window
	sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "TronGame v.1", sf::Style::None); //Create Window
	window.setFramerateLimit(60); //Limit Framerate to 60
	window.setMouseCursorVisible(false); //Hide Mouse

	//assign text stuff
	font.loadFromFile("arial.ttf");
	text.setFont(font);
	text.setCharacterSize(50);
	text.setPosition(windowWidth / 2 - 150, 100);
	text.setColor(sf::Color::White);
	text.setString("Player 1 Wins"); //Default value
	player1Text.setFont(font); //Player1Text
	player1Text.setCharacterSize(playerTextSize);
	player1Text.setPosition(player1TextPosition);
	player1Text.setColor(sf::Color::White);
	player2Text.setFont(font); //Player2Text
	player2Text.setCharacterSize(playerTextSize);
	player2Text.setPosition(player2TextPosition);
	player2Text.setColor(sf::Color::White);

	//create Player1 and Player 2
	Player player1(player1Size, player1InitialPos, player1Color, player1Velocity, player1InitialVelocity);
	Player player2(player2Size, player2InitialPos, player2Color, player2Velocity, player2InitialVelocity);

	//Assign Player1 and Player 2
	player1.isPlayer2 = false;
	player2.isPlayer2 = true;

	//Set AIs
	player1.isAI = player1IsAI;
	player2.isAI = player2IsAI;

	//Seed Random
	srand(time(0));

	//Game Loop
	while (window.isOpen())
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) window.close(); //Close Window and Exit Loop		
		window.clear(sf::Color::Black); //clear Last Frame

		//Call AI and update for AI
		if (!gameOver)
		{
			if (player1.isAI)
			{
				aiManager(player1, player2);
				player1.Update();
			}
			if (player2.isAI)
			{
				aiManager(player2, player1);
				player2.Update();
			}
			
		}

		//Call Update Functions for manual control
		if (!player1.isAI)
			player1.Update();
		if (!player2.isAI)
			player2.Update();

		//Check for Collisions
		collisionCheck(player1, player2);

		//Draw Objects
		window.draw(player1.trail.shape); //Draw Player1 Trail
		window.draw(player2.trail.shape);//Draw Player2 Trail
		for (Trail& trail : oldTrails)
			window.draw(trail.shape);
		window.draw(player1.shape); //Draw Player1 last so they are on top
		window.draw(player2.shape); //Draw Player2 last so they are on top
		//VictoryCheck
		victoryCheck(window, player1, player2);

		//Draw Player Scores
		drawScore(window);

		if (gameOver)
			reset(player1, player2);

		if (gameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Return)) //Reset the Game
		{
			reset(player1, player2);
		}

		//check for events
		sf::Event event;
		window.pollEvent(event);
		//Display Window
		window.display();
	}

	return 0;
}

