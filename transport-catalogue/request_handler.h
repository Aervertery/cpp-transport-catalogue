#pragma once
#include "transport_catalogue.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"

class RequestHandler {
public:
    RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer,
                    const transportcatalogue::transport_router::TransportRouter& router);

    svg::Document RenderMap() const;

    json::Document ProcessRequests(std::vector< transportcatalogue::json_reader::stat_read::Request>& requests);

private:
    const transportcatalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const transportcatalogue::transport_router::TransportRouter& router_;
};
