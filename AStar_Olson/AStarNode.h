#pragma once

#include "Simplex\Mesh\Model.h"

namespace Simplex {
	struct AStarNode
	{
	public:
		int x = 0;
		int y = 0;
		int nX = 0;
		int nY = 0;
		int costFromStart = 9999999;
		int costFromEnd = 9999999;
		int totalCost = 9999999;

		AStarNode(int x, int y);

		void setNeighbor(int x, int y);
		void setCostFromStart(int cost);
		void setCostFromEnd(int cost);
		void setTotalCost(int cost);

	};
	inline bool operator<(const AStarNode a, const AStarNode b) {
		return a.totalCost < b.totalCost;
	}
	inline bool operator<=(const AStarNode a, const AStarNode b) {
		return a.totalCost <= b.totalCost;
	}
	inline bool operator>(const AStarNode a, const AStarNode b) {
		return a.totalCost > b.totalCost;
	}
	inline bool operator>=(const AStarNode a, const AStarNode b) {
		return a.totalCost >= b.totalCost;
	}
}



