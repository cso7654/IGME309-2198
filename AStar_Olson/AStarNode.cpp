#include "AStarNode.h"

Simplex::AStarNode::AStarNode(int x, int y)
{
	this->x = x;
	this->y = y;
}

void Simplex::AStarNode::setNeighbor(int x, int y)
{
	nX = x;
	nY = y;
}

void Simplex::AStarNode::setCostFromStart(int cost)
{
	costFromStart = cost;
}

void Simplex::AStarNode::setCostFromEnd(int cost)
{
	costFromEnd = cost;
}

void Simplex::AStarNode::setTotalCost(int cost)
{
	totalCost = cost;
}
