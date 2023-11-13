#pragma once
#include "transport_catalogue.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"
#include "domain.h"
#include "serialization.h"

class RequestHandler {
public:
    RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer,
                    const transportcatalogue::transport_router::TransportRouter& router, 
                    transportcatalogue::json_reader::input::SerializationSettings serialization_settings);

    svg::Document RenderMap() const;

    json::Document ProcessRequests(std::vector< transportcatalogue::json_reader::stat_read::Request>& requests);

    void SetSerializationSettings(transportcatalogue::json_reader::input::SerializationSettings& serialization_settings);

    bool SaveDB() const;

private:
    const transportcatalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const transportcatalogue::transport_router::TransportRouter& router_;
    transportcatalogue::json_reader::input::SerializationSettings serialization_settings_;
};
