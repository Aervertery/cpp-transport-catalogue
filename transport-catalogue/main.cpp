#include "json_reader.h"
#include "request_handler.h"

int main() {
	transportcatalogue::TransportCatalogue tc;
	transportcatalogue::json_reader::input::Requests inp;
	transportcatalogue::json_reader::stat_read::Requests stats;
	auto res = json::Load(std::cin);
	inp.LoadRequests(res);
	renderer::MapRenderer renderer(inp.LoadSettings(res));
	inp.ProcessRequests(tc);
	stats.Load(res);
	RequestHandler handler(tc, renderer);
	json::Print(handler.ProcessRequests(stats.GetRequests()), std::cout);
	return 0;
}