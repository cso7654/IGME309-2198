#include "Main.h"

int main(void)
{
	AppClass* pApp = new AppClass("Shaders");
	std::cout << "Press [F] to activate the compliment filter" << std::endl;
	pApp->Run();
	SafeDelete(pApp);
	return 0;
}