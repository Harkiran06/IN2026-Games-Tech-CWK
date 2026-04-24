#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"

#include <vector>

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	void StartGame();
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	uint mLevel;
	uint mAsteroidCount;

	bool mGameStarted;
	
	shared_ptr<GUILabel> mMenuOptions[4];
	int mMenuSelection;
	void UpdateMenuSelect();

	shared_ptr<GUILabel> mInstructionsHeading;
	shared_ptr<GUILabel> mInstructionsLine1;
	shared_ptr<GUILabel> mInstructionsThrust;
	shared_ptr<GUILabel> mInstructionsLeftRot;
	shared_ptr<GUILabel> mInstructionsRightRot;
	shared_ptr<GUILabel> mInstructionsShoot;
	shared_ptr<GUILabel> mInstructionsLine2;
	shared_ptr<GUILabel> mInstructionsBackBtn;

	bool mInstructionsPage;
	void ShowInstructions();
	void HideInstructions();

	std::vector<shared_ptr<GameObject>> mMenuAsteroids;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	void CreateMenuAsteroids(const uint num_asteroids);
	void ClearMenuAsteroids();
	shared_ptr<GameObject> CreateExplosion();
	
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif
