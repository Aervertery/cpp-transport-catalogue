#include "input_reader.h"
#include "stat_reader.h"
#include "json.h"
#include "svg.h"
#include <locale.h>

int main() {
	setlocale(0, "");
	transportcatalogue::input::Requests inp;
	transportcatalogue::stat_read::Requests stats;
	//std::ifstream input("tsC_case1_input.txt");
	std::ifstream input("input.json");
	auto res = json::Load(input);
	/*inp.Load(input);
	stats.Load(input);
	transportcatalogue::TransportCatalogue tc;
	inp.ProcessRequests(tc);
	stats.ProcessRequests(tc);*/
	json::Print(res, std::cout);
	return 0;
}