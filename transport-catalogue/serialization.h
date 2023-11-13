#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "domain.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>

namespace serialization {
    using SerializedStops = google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Stop>;
    using SerializedBuses = google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Bus>;

    transport_catalogue_serialize::TransportCatalogue SaveCatalogue(const transportcatalogue::TransportCatalogue& tc);

    transport_catalogue_serialize::Color GetColor(const svg::Color& color);

    transport_catalogue_serialize::RenderSettings SaveRenderSettings(const transportcatalogue::json_reader::input::RenderSettings render_settings);

    transport_catalogue_serialize::Router SaveRouter(const transportcatalogue::transport_router::TransportRouter& router);

    transportcatalogue::transport_router::TransportRouter LoadDB(transportcatalogue::TransportCatalogue& tc, renderer::MapRenderer& renderer, 
        transportcatalogue::json_reader::input::SerializationSettings settings);

    svg::Color ParseColor(transport_catalogue_serialize::Color color);
    
    void LoadStops(transportcatalogue::TransportCatalogue& tc, SerializedStops& pbStops);

    void LoadBuses(transportcatalogue::TransportCatalogue& tc, SerializedBuses& pbBuses);

    void LoadRenderSettings(renderer::MapRenderer& renderer, transport_catalogue_serialize::RenderSettings& pbRenderSettings);

    transportcatalogue::transport_router::TransportRouter LoadRouter(transportcatalogue::TransportCatalogue& tc, transport_catalogue_serialize::Router& pbRouter);
} // namespace serialization