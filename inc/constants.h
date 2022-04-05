#include <string>

using namespace std;

#if !defined(MYLIB_CONSTANTS_H)
#define MYLIB_CONSTANTS_H

namespace Ports 
{
	const int ID = 536;
	const int serverA = 21000 + ID;
	const int serverB = 22000 + ID;
	const int serverC = 23000 + ID;
	const int serverM_S = 24000 + ID;
	const int serverM_A = 25000 + ID;
	const int serverM_B = 26000 + ID;
	const int backend_ports[3] = {serverA, serverB, serverC};
	const string convert[3] = {"A", "B", "C"};
}

#endif