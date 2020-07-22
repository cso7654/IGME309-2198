#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 0.0f, 100.0f), //Position
		vector3(0.0f, 0.0f, 99.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

#ifdef DEBUG
	uint uInstances = 900;
#else
	uint uInstances = 1849;
#endif
	int nSquare = static_cast<int>(std::sqrt(uInstances));
	m_uObjects = nSquare * nSquare;
	uint uIndex = -1;
	for (int i = 0; i < nSquare; i++)
	{
		for (int j = 0; j < nSquare; j++)
		{
			uIndex++;
			m_pEntityMngr->AddEntity("Minecraft\\Cube.obj");
			vector3 v3Position = vector3(glm::sphericalRand(34.0f));
			matrix4 m4Position = glm::translate(v3Position);
			m_pEntityMngr->SetModelMatrix(m4Position);
		}
	}

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
	m_pMeshMngr->AddCubeToRenderList(IDENTITY_M4, C_RED, RENDER_WIRE);
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui,
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release the octree


	//release GUI
	ShutdownGUI();
}