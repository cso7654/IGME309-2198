#ifndef __OCTREE_H_
#define __OCTREE_H_

#include <vector>
#include "MyEntity.h"
#include "MyRigidBody.h"

namespace Simplex
{
	class Octree
	{

	private:
		std::vector<Octree*> nodes;
		std::vector<Octree*> collisionNodes;
		std::vector<MyEntity*> entities;
		MyRigidBody* rigidBody;
		int currentLevels;

	public:
		//Constructor for root node
		Octree(std::vector<MyEntity*>, int);
		//Constructor for child node
		Octree(Octree* root, MyRigidBody*, std::vector<MyEntity*>, int, int);
		~Octree();
		MyRigidBody* getRigidBody();
		void Display();
		std::vector<Octree*> getNodes();
		std::vector<Octree*> getCollisionNodes();
		std::vector<MyEntity*> getEntities();

		void regenerate(std::vector<MyEntity*>, int);

		std::vector<MyRigidBody*> subdivideBounds(MyRigidBody*);

		int getCurrentLevels();
	};
}

#endif