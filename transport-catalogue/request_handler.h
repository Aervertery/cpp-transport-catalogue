#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

class RequestHandler {
public:
    RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    svg::Document RenderMap() const;

    json::Document ProcessRequests(std::vector< transportcatalogue::json_reader::stat_read::Request>& requests);

private:
    const transportcatalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
