#include "json_reader.h"
#include "request_handler.h"

int main() {
	transportcatalogue::TransportCatalogue tc;
	//setlocale(0, "");
	transportcatalogue::json_reader::input::Requests inp;
	transportcatalogue::json_reader::stat_read::Requests stats;
	//std::ifstream input("tsC_case1_input.txt");
	//std::ofstream output("test_out_json_1.txt");
	std::ofstream output("test_out_json.txt");
	std::ifstream input("input1.json");
	std::ofstream map_out("map.svg");
	//std::ifstream input("JsonCase1Input.txt");
	auto res = json::Load(std::cin);
	auto t = res.GetRoot();
	inp.LoadRequests(res);
	renderer::MapRenderer renderer(inp.LoadSettings(res), std::cout);
	inp.ProcessRequests(tc);
	stats.Load(res);
	RequestHandler handler(tc, renderer);
	json::Print(handler.ProcessRequests(stats.GetRequests()), std::cout);
	//json::Print(res, output);
	return 0;
}