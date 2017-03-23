#include "stdincludes.h"
#include "HMPPlatform.h"

/* 
 * Start Here! 
 */
int main(int argc, char* argv[]){

	HMPPlatform* simulator = new HMPPlatform();

	try	{
		simulator->setup();
	}
	catch (const std::exception& e)	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	simulator->run();

	return 0;
}
