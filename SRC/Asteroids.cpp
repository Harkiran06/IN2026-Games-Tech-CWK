#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mMenuSelection = 0;
	for (int i = 0; i < 4; i++)
	{
		mMenuOptions[i] = nullptr;
	}
	mInstructionsPage = false;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	
	// show the start screen
	CreateMenuAsteroids(8);
	mGameStarted = false;
	
	const char* menuTexts[4] = { "Game Start", "Difficulty", "Instructions","Leaderboard" };
	float menuPositions[4] = { 0.6f,0.5f,0.4f,0.3f };

	for (int i = 0; i < 4; i++)
	{
		mMenuOptions[i] = make_shared<GUILabel>(menuTexts[i]);
		mMenuOptions[i]->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		mMenuOptions[i]->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
		shared_ptr<GUIComponent> component = static_pointer_cast<GUIComponent>(mMenuOptions[i]);
		mGameDisplay->GetContainer()->AddComponent(component, GLVector2f(0.5f, menuPositions[i]));
	}

	mMenuSelection = 0;
	UpdateMenuSelect();

	// Start the game
	GameSession::Start();
}

void Asteroids::StartGame() 
{
	ClearMenuAsteroids();
	for (int i = 0; i < 4; i++)
	{
		mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mMenuOptions[i]));

	}
	CreateGUI();
	mGameWorld->AddObject(CreateSpaceship());
	CreateAsteroids(10);
	mGameWorld->AddListener(&mPlayer);
	mPlayer.AddListener(shared_ptr<Asteroids>(this));
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	switch (key)
	{
	//menu
	case '\r':
		if (mInstructionsPage) {
			HideInstructions();
		}
		else if (!mGameStarted) {
			switch (mMenuSelection) {
			case 0:
				mGameStarted = true;
				StartGame();
				break;
			case 1:
				break;
			case 2:
				ShowInstructions();
				break;
			case 3:
				break;
			}
		}
		break;
	//when started
	case ' ':
		if (mGameStarted) mSpaceship->Shoot();
		break;
	default:
		break;
	} 
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {
}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (!mGameStarted)
	{
		switch (key) 
		{
		case GLUT_KEY_UP:
			mMenuSelection = (mMenuSelection - 1 + 4) % 4;
			UpdateMenuSelect();
			break;
		case GLUT_KEY_DOWN:
			mMenuSelection = (mMenuSelection + 1) % 4;
			UpdateMenuSelect();
			break;
		default: break;
		}
	}
	else {
		switch (key)
		{
			// If up arrow key is pressed start applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If left arrow key is pressed start rotating anti-clockwise
		case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
			// If right arrow key is pressed start rotating clockwise
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
			// Default case - do nothing
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (!mGameStarted) return;
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if(object->GetType() == GameObjectType("Asteroid") && mGameStarted)
	{
		shared_ptr<Asteroid> asteroid = static_pointer_cast<Asteroid>(object);

		if (asteroid->IsMenuAsteroid()) return;

		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);

		mAsteroidCount--;

		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

}

void Asteroids::OnScoreChanged(int score)
{
	if (!mScoreLabel) return;
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

//class for creating the asteroids that float in the menu of the background
void Asteroids::CreateMenuAsteroids(const uint num_asteroids)
{
	mMenuAsteroids.clear();

	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

		asteroid_sprite->SetLoopAnimation(true);

		shared_ptr<Asteroid> asteroid = make_shared<Asteroid>();
		asteroid->SetIsMenuAsteroid(true);
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.12f); 

		mGameWorld->AddObject(asteroid);
		mMenuAsteroids.push_back(asteroid);
	}
}

//for clearing the menu asteroids before the game starts
void Asteroids::ClearMenuAsteroids()
{
	for (uint i = 0; i < mMenuAsteroids.size(); i++)
	{
		mGameWorld->RemoveObject(mMenuAsteroids[i]);
	}
	mMenuAsteroids.clear();
}

void Asteroids::UpdateMenuSelect() {
	const char* menuTexts[4] = { "Game Start", "Difficulty", "Instructions","Leaderboard" };
	for (int i = 0; i < 4; i++)
	{
		if (i == mMenuSelection)
			mMenuOptions[i]->SetText(std::string("> ") + menuTexts[i]);
		else
			mMenuOptions[i]->SetText(menuTexts[i]);
	}
}

void Asteroids::ShowInstructions()
{
	mInstructionsPage = true;

	for (int i = 0; i < 4; i++)
		mMenuOptions[i]->SetVisible(false);

	// Title
	mInstructionsHeading = make_shared<GUILabel>("-- INSTRUCTIONS --");
	mInstructionsHeading->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsHeading->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsHeading), GLVector2f(0.5f, 0.75f));

	// Separator at top
	mInstructionsLine1 = make_shared<GUILabel>("----------------------");
	mInstructionsLine1->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLine1->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsLine1), GLVector2f(0.5f, 0.68f));

	// Controls yay :D
	mInstructionsThrust = make_shared<GUILabel>("Arrow Up      -  Thrust");
	mInstructionsThrust->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsThrust->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsThrust), GLVector2f(0.5f, 0.61f));

	mInstructionsLeftRot = make_shared<GUILabel>("Arrow Left    -  Rotate Left");
	mInstructionsLeftRot->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLeftRot->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsLeftRot), GLVector2f(0.5f, 0.54f));

	mInstructionsRightRot = make_shared<GUILabel>("Arrow Right   -  Rotate Right");
	mInstructionsRightRot->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsRightRot->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsRightRot), GLVector2f(0.5f, 0.47f));

	mInstructionsShoot = make_shared<GUILabel>("Space         -  Shoot");
	mInstructionsShoot->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsShoot->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsShoot), GLVector2f(0.5f, 0.40f));

	// Separator at bottom 
	mInstructionsLine2 = make_shared<GUILabel>("----------------------");
	mInstructionsLine2->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLine2->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsLine2), GLVector2f(0.5f, 0.33f));

	// Back to menu
	mInstructionsBackBtn = make_shared<GUILabel>("> BACK");
	mInstructionsBackBtn->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsBackBtn->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsBackBtn), GLVector2f(0.5f, 0.25f));
}

void Asteroids::HideInstructions()
{
	mInstructionsPage = false;

	// Remove all instructions labels
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsHeading));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsLine1));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsThrust));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsLeftRot));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsRightRot));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsShoot));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsLine2));
	mGameDisplay->GetContainer()->RemoveComponent(static_pointer_cast<GUIComponent>(mInstructionsBackBtn));

	for (int i = 0; i < 4; i++)
		mMenuOptions[i]->SetVisible(true);

	UpdateMenuSelect();
}



