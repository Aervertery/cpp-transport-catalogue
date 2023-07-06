#include "input_reader.h"
#include "stat_reader.h"

int main() {
	transportcatalogue::input::Requests inp;
	transportcatalogue::stat_read::Requests stats;
	std::ifstream input("tsC_case1_input.txt");
	inp.Load(input);
	stats.Load(input);
	transportcatalogue::TransportCatalogue tc;
	inp.ProcessRequests(tc);
	stats.ProcessRequests(tc);
	return 0;
}