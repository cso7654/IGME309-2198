#pragma once

#include <queue>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "Simplex\Mesh\Model.h"
#include "MyRigidBody.h"
#include "MyEntity.h"
#include "AStarNode.h"
#include "Simplex\Mesh\MeshManager.h"

namespace Simplex
{
	class AStarPath
	{
	private:
		//A list of points representing the path to follow
		std::vector<vector2> path;

		vector2 start;
		vector2 end;

		std::string nodeToStr(AStarNode* node);
	public:
		AStarPath(int sX, int sY, int eX, int eY, int world[25][25]);
		~AStarPath();

		int heuristic(AStarNode* end, AStarNode* current);
		std::vector<std::vector<int>> getNeighbors(AStarNode* point, int world[25][25]);

		std::vector<vector2> getPath();

		void display(Simplex::MeshManager* meshManager);

		vector2 traverse(float position);
	};
}

