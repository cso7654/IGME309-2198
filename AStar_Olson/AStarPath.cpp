#include "AStarPath.h"

std::string Simplex::AStarPath::nodeToStr(AStarNode* node)
{
	return std::to_string(node->x) + "," + std::to_string(node->y);
}

Simplex::AStarPath::AStarPath(int sX, int sY, int eX, int eY, int world[25][25])
{
	//Generate path on object construction. Not great to do for a real-world application, but this just needs to work for the assignment
	//Code based off of A* assignment from GDAPS 2

	//Keep track of the current cost from the start of the path
	int cost = 0;
	//Store all neighbors of current point
	std::vector<vector2> neighbors;
	//Keep a priority queue of the points not part of the path yet, but that are possible, and a set for easy recall of whether or not that point is in the queue
	std::priority_queue<AStarNode*, std::vector<AStarNode*>, std::greater<AStarNode*>> openQueue;
	std::set<AStarNode*> openSet;
	//Keep all the points that are in the path in a vector
	std::set<AStarNode*> closedSet;
	//Keep a map from points to their respective AStarNodes
	std::map<std::string, AStarNode*> nodeMap;
	//Insert the start and end nodes into the map
	nodeMap[std::to_string(sX) + "," + std::to_string(sY)] = new AStarNode(sX, sY);
	nodeMap[std::to_string(sX) + "," + std::to_string(sY)]->costFromStart = 0;
	nodeMap[std::to_string(eX) + "," + std::to_string(eY)] = new AStarNode(eX, eY);
	//Add start point to closed set
	closedSet.insert(nodeMap[std::to_string(sX) + "," + std::to_string(sY)]);
	//Keep track of the current point in the path. Initialiye with the start point
	AStarNode* current = nodeMap[std::to_string(sX) + "," + std::to_string(sY)];

	//Loop until the current point is equal to the end
	while (current != nodeMap[std::to_string(eX) + "," + std::to_string(eY)]) {
		std::vector<std::vector<int>> neighbors = getNeighbors(current, world);

		if (neighbors.capacity() > 0) {
			//Loop through neighbors
			for (std::vector<int> neighborPt : neighbors) {
				std::string key = std::to_string(neighborPt.at(0)) + "," + std::to_string(neighborPt.at(1));
				//If the node map does not contain a node at the current point, add one
				if (nodeMap[key] == nullptr) {
					nodeMap[key] = new AStarNode(neighborPt.at(0), neighborPt.at(1));
				}

				if (openSet.find(nodeMap[key]) != openSet.end() && closedSet.find(nodeMap[key]) == closedSet.end()) {
					//If the point is in the open set, calculate a new cost from start
					int newCost = nodeMap[nodeToStr(current)]->costFromStart + 1;
					//Check if the cost stored at that neighbor is higher than the new cost
					if (nodeMap[key]->costFromStart > newCost) {
						nodeMap[key]->setNeighbor(current->x, current->y);
						//Update the cost to the start of the neighbor point
						nodeMap[key]->setCostFromStart(newCost);
						//Update the cost to the end of the neighbor point
						nodeMap[key]->setCostFromEnd(heuristic(nodeMap[std::to_string(eX) + "," + std::to_string(eY)], nodeMap[key]));
						//Update the total cost of the neighbor point
						nodeMap[key]->setTotalCost(nodeMap[key]->costFromStart + nodeMap[key]->costFromEnd);
						//Remake the queue

						std::set<AStarNode*> nodes;
						for (int i = 0; i < openQueue.size(); i++) {
							nodes.insert(openQueue.top());
							openQueue.pop();
						}
						for (AStarNode* node : nodes) {
							openQueue.push(node);
						}
					}

				}
				else if (closedSet.find(nodeMap[key]) == closedSet.end()) {
					//std::cout << "   NOT IN SET" << std::endl;
					//If the point is not in the open set and also not in the closed set
					nodeMap[key]->setNeighbor(current->x, current->y);
					//Update the cost to the start of the neighbor point

					//std::cout << nodeToStr(current) << "   " << current->x << ", " << current->y << std::endl;
					//std::cout << nodeMap[nodeToStr(current)] << std::endl;
					//std::cout << nodeMap[nodeToStr(current)]->costFromStart << std::endl;

					nodeMap[key]->setCostFromStart(nodeMap[nodeToStr(current)]->costFromStart + 1);
					//Update the cost to the end of the neighbor point
					nodeMap[key]->setCostFromEnd(heuristic(nodeMap[std::to_string(eX) + "," + std::to_string(eY)], nodeMap[key]));
					//Update the total cost of the neighbor point
					nodeMap[key]->setTotalCost(nodeMap[key]->costFromEnd + nodeMap[key]->costFromStart);
					//Update the open set and queue
					if (openSet.find(nodeMap[key]) == openSet.end()) {
						openQueue.push(nodeMap[key]);
					}
					openSet.insert(nodeMap[key]);
				}
			}
		}


		//Check if the open set is now empty
		if (openSet.size() == 0) {
			current = nodeMap[std::to_string(eX) + "," + std::to_string(eY)];
			break;
		}

		//Update the current point
		current = openQueue.top();
		openSet.erase(current);
		openQueue.pop();
		//Update closed set
		closedSet.insert(nodeMap[nodeToStr(current)]);
	}

	//std::cout << openSet.size() << "   " << closedSet.size() << std::endl;

	//Construct path and delete nodes
	bool finished = false;
	AStarNode* compileNode = nodeMap[std::to_string(eX) + "," + std::to_string(eY)];

	std::vector<vector2> newPath;
	while (!finished) {
		newPath.push_back(vector2(compileNode->x, compileNode->y));
		if (compileNode == nodeMap[std::to_string(sX) + "," + std::to_string(sY)]) {
			//If the current point's position matches the start point's position, path has been constructed
			finished = true;
			break;
		}
		compileNode = nodeMap[std::to_string(compileNode->nX) + "," + std::to_string(compileNode->nY)];
	}
	//Reverse newPath into final path
	for (int i = newPath.size() - 1; i >= 0; i--) {
		path.push_back(newPath.at(i));
	}


	//Clear node map
	for (std::map<std::string, AStarNode*>::iterator i = nodeMap.begin(); i != nodeMap.end(); i++) {
		delete i->second;
	}
	nodeMap.clear();
}

Simplex::AStarPath::~AStarPath()
{
}

int Simplex::AStarPath::heuristic(AStarNode* end, AStarNode* current)
{
	return std::abs(end->x - current->x) + std::abs(end->y - current->y);
}

std::vector<std::vector<int>> Simplex::AStarPath::getNeighbors(AStarNode* point, int world[25][25])
{
	int x = point->x;
	int y = point->y;
	std::vector<std::vector<int>> neighbors;

	//point above current is below top edge and future point in world is 0
	if (y + 1 < 25 && world[x][y + 1] == 0)
	{
		neighbors.push_back({ x, y + 1 });
	}
	//point below current is above bottom edge and future point in world is 0
	if (y - 1 >= 0 && world[x][y - 1] == 0)
	{
		neighbors.push_back({ x, y - 1 });
	}
	//point right of current is below right edge and future point in world is 0
	if (x + 1 < 25 && world[x + 1][y] == 0)
	{
		neighbors.push_back({ x + 1, y });
	}
	//point left of current is above left edge and future point in world is 0
	if (x - 1 >= 0 && world[x - 1][y] == 0)
	{
		neighbors.push_back({ x - 1, y });
	}

	return neighbors;
}

std::vector<Simplex::vector2> Simplex::AStarPath::getPath()
{
	return path;
}

void Simplex::AStarPath::display(Simplex::MeshManager* meshManager)
{
	//Loop through each point of the path
	for (int i = 0; i < path.size() - 1; i++)
	{
		meshManager->AddLineToRenderList(IDENTITY_M4, vector3(path.at(i).x + 0.5, 1.5f, path.at(i).y + 0.5),
													vector3(path.at(i + 1).x + 0.5, 1.5f, path.at(i + 1).y + 0.5), C_PURPLE, C_PURPLE);
	}
}

Simplex::vector2 Simplex::AStarPath::traverse(float position)
{
	if (position >= 1) {
		return path.at(path.size() - 1);
	}
	else {
		//Traverse the path via LERP

		//Get which two points the traveler is between
		int segment = (int)(position * path.size());
		//Get how far between the two segments the traveler is
		float dist = segment - (position * path.size());
		//Get the vectors of the path
		vector2 v1 = path.at(segment);
		vector2 v2 = path.at(segment + 1);
		//LERP
		vector2 diff = v1 - v2;
		return vector2(v1.x + diff.x * dist, v1.y + diff.y * dist);
	}
}
