#include <iostream>
#include <string>
#include<ctime> // used to work with date and time
#include "MemoryAccess.h"
#include "AlbumManager.h"

#include "DatabaseAccess.h"

int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");
	
	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}


void displayCurrentTime()
{
	time_t t; // t passed as argument in function time()
	struct tm* tt; // decalring variable for localtime()
	time(&t); //passing argument to time()
	tt = localtime(&t);
	std::cout << "\t" << asctime(tt) << std::endl;
}


int main(void)
{
	// initialization data access
	DatabaseAccess dataAccess;

	// initialize album manager
	AlbumManager albumManager(dataAccess);


	std::string albumName;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "By Neta Orevi " << std::endl;
	displayCurrentTime();
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do {
		int commandNumber = getCommandNumberFromUser();
		
		try	{
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} catch (std::exception& e) {	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);
}


