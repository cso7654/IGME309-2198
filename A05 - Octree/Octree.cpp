#include "Octree.h"
using namespace Simplex;

//Octree is the root
Simplex::Octree::Octree(std::vector<MyEntity*> entityVector, int maxSub)
{
	regenerate(entityVector, maxSub);
}
//Octree is a leaf
Simplex::Octree::Octree(Octree* root, MyRigidBody* rigidBody, std::vector<MyEntity*> parentEntities, int octantLevels, int maxSub)
	: rigidBody(rigidBody)
{
	//Check which entities are contained in this rigidbody, and store the contained ones in a new vector
	std::vector<MyEntity*> leafEntities;
	for (MyEntity* entity : parentEntities) {
		if (rigidBody->IsColliding(entity->GetRigidBody())) {
			//Remove collisions since this is just a "check" and not an actual collision between objects
			entity->GetRigidBody()->RemoveCollisionWith(rigidBody);
			rigidBody->RemoveCollisionWith(entity->GetRigidBody());
			//Add the entity to the leaf entities list
			leafEntities.push_back(entity);
		}
	}

	if (leafEntities.size() > 1) {
		if (octantLevels < maxSub) {
			//There is more than one entity in the vector and the current level is below the maximum subdivision level, subdivide further

			//Get subdivisioned bounds
			std::vector<MyRigidBody*> subBounds = subdivideBounds(rigidBody);

			for (int i = 0; i < 8; i++) {
				//Create new leaf
				Octree* leaf = new Octree(root, subBounds.at(i), leafEntities, octantLevels + 1, maxSub);
				nodes.push_back(leaf);
			}
		}
		else {
			//the subdivision limit is reached and there is more than one entity in the vector. Set the entity vector and update the root's colliding nodes vector
			entities = leafEntities;
			//NOTE: collisionNodes holds any nodes where a collision is possible, not necessarily where there is one
			root->collisionNodes.push_back(this);
		}
	}

}


Simplex::Octree::~Octree()
{
	for (Octree* node: nodes) {
		delete node;
	}
	delete rigidBody;
	rigidBody = nullptr;
	//DONT delete entities, they are used elsewhere
}

MyRigidBody* Simplex::Octree::getRigidBody()
{
	return rigidBody;
}

void Simplex::Octree::Display()
{
	//Add rigidbody to render list
	rigidBody->AddToRenderList();
	//Loop through child nodes and call Display() on them
	for (Octree* leaf : nodes) {
		leaf->Display();
	}
}

std::vector<Octree*> Simplex::Octree::getNodes()
{
	return nodes;
}

std::vector<Octree*> Simplex::Octree::getCollisionNodes()
{
	return collisionNodes;
}

std::vector<MyEntity*> Simplex::Octree::getEntities()
{
	return entities;
}

void Simplex::Octree::regenerate(std::vector<MyEntity*> entityVector, int maxSub)
{
	//Reset values
	nodes.clear();
	collisionNodes.clear();
	entities.clear();
	delete rigidBody;
	rigidBody = nullptr;

	currentLevels = maxSub;
	//Make sure entity list is greater than 1, otherwise this is pointless
	//Get max and min vectors from entities
	vector3 max = entityVector.at(0)->GetRigidBody()->GetMaxGlobal();
	vector3 min = entityVector.at(0)->GetRigidBody()->GetMinGlobal();
	for (MyEntity* entity : entityVector) {
		vector3 entMax = entity->GetRigidBody()->GetMaxGlobal();
		if (entMax.x > max.x) {
			max.x = entMax.x;
		}
		if (entMax.y > max.y) {
			max.y = entMax.y;
		}
		if (entMax.z > max.z) {
			max.z = entMax.z;
		}

		vector3 entMin = entity->GetRigidBody()->GetMinGlobal();
		if (entMin.x < min.x) {
			min.x = entMin.x;
		}
		if (entMin.y < min.y) {
			min.y = entMin.y;
		}
		if (entMin.z < min.z) {
			min.z = entMin.z;
		}
	}

	//Get vertices of bounds
	std::vector<vector3> bodyVerts;
	bodyVerts.push_back(min);
	bodyVerts.push_back(vector3(max.x, min.y, min.z));
	bodyVerts.push_back(vector3(min.x, max.y, min.z));
	bodyVerts.push_back(vector3(max.x, max.y, min.z));
	bodyVerts.push_back(vector3(min.x, min.y, max.z));
	bodyVerts.push_back(vector3(max.x, min.y, max.z));
	bodyVerts.push_back(vector3(min.x, max.y, max.z));
	bodyVerts.push_back(max);

	rigidBody = new MyRigidBody(bodyVerts);

	//If the subdivision limit is greater than 0, subdivide. Otherwise, just make the root the only collidable node
	if (maxSub > 0) {
		//Get subdivisioned bounds
		std::vector<MyRigidBody*> subBounds = subdivideBounds(rigidBody);


		//Create 8 leaf nodes
		for (int i = 0; i < 8; i++) {
			Octree* leaf = new Octree(this, subBounds.at(i), entityVector, 1, maxSub);

			nodes.push_back(leaf);
		}
	}
	else {
		entities = entityVector;
		collisionNodes.push_back(this);
	}
}

std::vector<MyRigidBody*> Simplex::Octree::subdivideBounds(MyRigidBody* bounds)
{
	std::vector<MyRigidBody*> bodies;

	//Get all the vertices of the original bounds
	std::vector<vector3> bodyVerts;
	bodyVerts.push_back(bounds->GetMinGlobal());
	bodyVerts.push_back(vector3(bounds->GetMaxGlobal().x, bounds->GetMinGlobal().y, bounds->GetMinGlobal().z));
	bodyVerts.push_back(vector3(bounds->GetMinGlobal().x, bounds->GetMaxGlobal().y, bounds->GetMinGlobal().z));
	bodyVerts.push_back(vector3(bounds->GetMaxGlobal().x, bounds->GetMaxGlobal().y, bounds->GetMinGlobal().z));
	bodyVerts.push_back(vector3(bounds->GetMinGlobal().x, bounds->GetMinGlobal().y, bounds->GetMaxGlobal().z));
	bodyVerts.push_back(vector3(bounds->GetMaxGlobal().x, bounds->GetMinGlobal().y, bounds->GetMaxGlobal().z));
	bodyVerts.push_back(vector3(bounds->GetMinGlobal().x, bounds->GetMaxGlobal().y, bounds->GetMaxGlobal().z));
	bodyVerts.push_back(bounds->GetMaxGlobal());

	for (int i = 0; i < 8; i++) {
		//Create a new rigidbody with the bodyVerts and the center of the original body as the max and min
		std::vector<vector3> subVerts;
		subVerts.push_back(bodyVerts.at(i));
		subVerts.push_back(vector3(bounds->GetCenterLocal().x, bodyVerts.at(i).y, bodyVerts.at(i).z));
		subVerts.push_back(vector3(bodyVerts.at(i).x, bounds->GetCenterLocal().y, bodyVerts.at(i).z));
		subVerts.push_back(vector3(bounds->GetCenterLocal().x, bounds->GetCenterLocal().y, bodyVerts.at(i).z));
		subVerts.push_back(vector3(bodyVerts.at(i).x, bodyVerts.at(i).y, bounds->GetCenterLocal().z));
		subVerts.push_back(vector3(bounds->GetCenterLocal().x, bodyVerts.at(i).y, bounds->GetCenterLocal().z));
		subVerts.push_back(vector3(bodyVerts.at(i).x, bounds->GetCenterLocal().y, bounds->GetCenterLocal().z));
		subVerts.push_back(bounds->GetCenterLocal());

		bodies.push_back(new MyRigidBody(subVerts));
	}

	return bodies;
}

int Simplex::Octree::getCurrentLevels()
{
	return currentLevels;
}
