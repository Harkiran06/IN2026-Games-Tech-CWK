#ifndef __ASTEROID_H__
#define __ASTEROID_H__

#include "GameObject.h"

class Asteroid : public GameObject
{
public:
	Asteroid(void);
	~Asteroid(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);

	void SetIsMenuAsteroid(bool value) { mIsMenuAsteroid = value; }
	bool IsMenuAsteroid() const { return mIsMenuAsteroid; }

private:

	bool mIsMenuAsteroid = false;
};

#endif
