#include "transport_catalogue.h"

int main() {
	transportcatalogue::input::Requests inp;
	transportcatalogue::stat_read::Requests stats;
	std::ifstream input("tsC_case1_input.txt");
	inp.Load(input);
	stats.Load(input);
	transportcatalogue::TransportCatalogue tc;
	tc.ProcessInput(inp);
	tc.ProcessStats(stats);
	stats.Print();
	return 0;
}