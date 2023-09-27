#include "json_reader.h"
#include "request_handler.h"
#include <fstream>

int main() {
	transportcatalogue::TransportCatalogue tc;
	transportcatalogue::json_reader::input::Requests inp;
	transportcatalogue::json_reader::stat_read::Requests stats;
	std::ifstream in("test_13_1.txt");
	std::ofstream outp("test_13_out.txt");
	auto res = json::Load(in);
	inp.LoadRequests(res);
	renderer::MapRenderer renderer(inp.LoadRenderSettings(res));
	inp.ProcessRequests(tc);
	stats.Load(res);
	transportcatalogue::transport_router::TransportRouter router(inp.LoadRoutingSettings(res), tc, tc.GetStopCount());
	RequestHandler handler(tc, renderer, router);
	json::Print(handler.ProcessRequests(stats.GetRequests()), outp);
	return 0;
}