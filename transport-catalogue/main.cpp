#include <fstream>
#include <iostream>
#include <string_view>
#include "json_reader.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]); 

    if (mode == "make_base"sv) 
    {
        transportcatalogue::TransportCatalogue tc;
        transportcatalogue::json_reader::input::Requests inp;
        auto res = json::Load(std::cin);
        inp.LoadRequests(res);
        renderer::MapRenderer renderer(inp.LoadRenderSettings(res));
        inp.ProcessRequests(tc);
        transportcatalogue::transport_router::TransportRouter router(inp.LoadRoutingSettings(res), tc);
        RequestHandler handler(tc, renderer, router, inp.LoadSerializationSettings(res));
        handler.SaveDB();
    }
    else if (mode == "process_requests"sv) 
    {
        transportcatalogue::TransportCatalogue tc;
        transportcatalogue::json_reader::stat_read::Requests stats;
        auto res = json::Load(std::cin);
        stats.Load(res);
        renderer::MapRenderer renderer;
        transportcatalogue::transport_router::TransportRouter router(serialization::LoadDB(tc, renderer, stats.LoadSerializationSettings(res)));
        RequestHandler handler(tc, renderer, router, stats.LoadSerializationSettings(res));
        json::Print(handler.ProcessRequests(stats.GetRequests()), std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}