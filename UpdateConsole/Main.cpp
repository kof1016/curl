// Main.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <conio.h>
#include "../Logic/UpdateLauncher.h"


int main(int argc, char* argv[])
{
	UpdateLogic::UpdateLauncher launcher;

	launcher.Start();

	launcher.OnDownloadProgress([=](int total_size, int downloaded_size)
	{
		const auto percent = downloaded_size * 100.0 / total_size;
		std::cout << "percent=" << percent << "\r";
		// game using interface
	});

	launcher.OnUpdateSuccessEvent([=]()
	{
		std::cout << "all download success" << std::endl;
		std::cout << "ready to game" << std::endl;
		// game using interface
	});

	launcher.OnNotNeedEvent([=]()
	{
		std::cout << "remote ver equally local ver" << std::endl;
		std::cout << "need not update" << std::endl;
		std::cout << "ready to game" << std::endl;
		// game using interface
	});


	int n = _kbhit();
	std::cout << "input 1 to exit" << std::endl;

	while (true)
	{
		if (_kbhit() != 0)
		{
			if (_getch() == 0x31)
			{
				std::cout << "exit" << std::endl;
				launcher.Shutdown();
				break;
			}
		}
		launcher.Update();
	}

	return 0;
}
