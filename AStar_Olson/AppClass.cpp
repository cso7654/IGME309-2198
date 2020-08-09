#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Size of grid for path generation
	int gridSize = 25;

	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(gridSize / 2.0f, 10.0f, -10.0f), //Position
		vector3(gridSize / 2.0f, 0.0f, gridSize / 2.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	//Generate a 25x25 grid of cubes based on the obstacle map
	for (int i = 0; i < gridSize; i++) {
		for (int j = 0; j < gridSize; j++) {
			//If there is no obstacle in the spot on the map, put a cube there
			if (obstacleMap[i][j] == 0) {
				m_pEntityMngr->AddEntity("Minecraft\\Cube.obj");
				vector3 v3Position = vector3(i, 0, j);
				matrix4 m4Position = glm::translate(v3Position);
				m_pEntityMngr->SetModelMatrix(m4Position);
				m_pEntityMngr->GetRigidBody()->SetVisibleARBB(false);
				m_pEntityMngr->GetRigidBody()->SetVisibleBS(false);
				m_pEntityMngr->GetRigidBody()->SetVisibleOBB(false);
			}
		}
	}

	//Create a path follower
	m_pEntityMngr->AddEntity("Minecraft\\Steve.obj");
	follower = m_pEntityMngr->GetEntity(m_pEntityMngr->GetEntityCount() - 1);
	vector3 v3Position = vector3(0.5, 1, 0.5);
	matrix4 m4Position = glm::translate(v3Position);
	m_pEntityMngr->SetModelMatrix(m4Position);
	m_pEntityMngr->GetRigidBody()->SetVisibleARBB(false);
	m_pEntityMngr->GetRigidBody()->SetVisibleBS(false);
	m_pEntityMngr->GetRigidBody()->SetVisibleOBB(false);

	//Create a target
	m_pEntityMngr->AddEntity("Minecraft\\Cow.obj");
	target = m_pEntityMngr->GetEntity(m_pEntityMngr->GetEntityCount() - 1);
	v3Position = vector3(24.5, 1, 24.5);
	m4Position = glm::translate(v3Position);
	m_pEntityMngr->SetModelMatrix(m4Position);
	m_pEntityMngr->GetRigidBody()->SetVisibleARBB(false);
	m_pEntityMngr->GetRigidBody()->SetVisibleBS(false);
	m_pEntityMngr->GetRigidBody()->SetVisibleOBB(false);

	//Create a new AStarPath from 0,0 to 24,24 to start
	path = new AStarPath(0, 0, 24, 24, obstacleMap);

	m_uOctantLevels = 4;
	//m_pRoot = new Octree(m_uOctantLevels, 5);
	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
	
	//Get input for octant level updates
	if (gui.io.KeysDown[sf::Keyboard::Equal]) {
		//Set equals (plus) to false to prevent constant pressing
		gui.io.KeysDown[sf::Keyboard::Equal] = false;
		m_uOctantLevels++;
	}
	if (gui.io.KeysDown[sf::Keyboard::Dash]) {
		//Set subtract to false to prevent constant pressing
		gui.io.KeysDown[sf::Keyboard::Dash] = false;
		m_uOctantLevels--;
		if (m_uOctantLevels < 0) {
			m_uOctantLevels = 0;
		}
	}
	//Get input for octree display toggle
	if (gui.io.KeysDown[sf::Keyboard::Tab]) {
		//Set Tab to false to prevent constant pressing
		gui.io.KeysDown[sf::Keyboard::Tab] = false;
		m_pEntityMngr->displayOctree = !m_pEntityMngr->displayOctree;
	}

	//Regenerate octree if the level was changed
	if (m_uOctantLevels != m_pEntityMngr->getOctree()->getCurrentLevels()) {
		m_pEntityMngr->regenerateOctree(m_uOctantLevels);
	}

	//Logic for follower and target

	//Make follower lerp over path
	pathDist += 0.005;
	if (pathDist > 1) {
		pathDist = 1;
	}
	vector2 pathPoint = path->traverse(pathDist);
	follower->SetModelMatrix(glm::translate(vector3(pathPoint.x + 0.5, 1, pathPoint.y + 0.5)));

	//Check if the follower and target are colliding
	if (follower->IsColliding(target)) {
		//Randomly pick a new position for the target
		int rX;
		int rY;
		bool valid = false;
		while (!valid) {
			rX = rand() % 25;
			rY = rand() % 25;
			//Make sure the target is not over a hole
			if (obstacleMap[rX][rY] == 0) {
				valid = true;
				break;
			}
		}

		//Set the target's new position
		target->SetModelMatrix(glm::translate(vector3(rX + 0.5, 1, rY + 0.5)));
		//Get old end position from previous path as start
		vector2 start = vector2(path->getPath().at(path->getPath().size() - 1));
		//Create a new AStar path
		delete path;
		path = new AStarPath(start.x, start.y, rX, rY, obstacleMap);

		//Update the distance traveled over path
		pathDist = 0;
	}

	//Update Entity Manager
	m_pEntityMngr->Update();

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//display octree
	//m_pRoot->Display(); <---
	//m_pMeshMngr->AddCubeToRenderList(IDENTITY_M4, C_RED, RENDER_WIRE);
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//Draw the AStar path
	path->display(m_pMeshMngr);

	//draw gui,
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release the octree
	delete m_pEntityMngr->getOctree();

	SafeDelete(follower);
	SafeDelete(target);

	delete path;

	//release GUI
	ShutdownGUI();
}