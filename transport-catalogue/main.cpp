//#include "transport_catalogue.h"
#include "tests.h"

int main() {
	//TestTransportCatalogue();
	input::Requests inp;
	stat_read::Requests stats;
	std::ifstream input("tsC_case1_input.txt");
	inp.Load(input);
	stats.Load(input);
	TransportCatalogue tc;
	tc.ProcessInput(inp);
	tc.ProcessStats(stats);
	stats.Print();
	return 0;
}