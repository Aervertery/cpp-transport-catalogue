#include "json_reader.h"
#include "request_handler.h"
#include <fstream>

int main() {
	transportcatalogue::TransportCatalogue tc;
	transportcatalogue::json_reader::input::Requests inp;
	transportcatalogue::json_reader::stat_read::Requests stats;
	std::ifstream in("s10_final_opentest_2.json");
	std::ofstream outp("map1.svg");
	std::ofstream outp_("FINAL_FINAL_TEST_builder.txt");
	std::ofstream outp_1("FINAL_FINAL_TEST_no_builder.txt");
	auto res = json::Load(in);
	inp.LoadRequests(res);
	renderer::MapRenderer renderer(inp.LoadSettings(res));
	inp.ProcessRequests(tc);
	stats.Load(res);
	RequestHandler handler(tc, renderer);
	json::Print(handler.ProcessRequests(stats.GetRequests()), outp_);
	json::Print(handler.ProcessRequestsNoBuilder(stats.GetRequests()), outp_1);
	handler.RenderMap().Render(outp);
	return 0;
}