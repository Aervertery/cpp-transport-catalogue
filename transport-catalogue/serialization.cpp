#include "serialization.h"

namespace serialization {
	transport_catalogue_serialize::TransportCatalogue SaveCatalogue(const transportcatalogue::TransportCatalogue& tc) {
		std::set<std::string_view> buses = tc.GetDrawableBuses();
		auto wrappedBuses = tc.WrapBuses(buses);
		std::unordered_set<std::string_view> stops;
		auto wrappedStops = tc.WrapStops();

		transport_catalogue_serialize::TransportCatalogue tc_out;

		for (const auto& stop : wrappedStops) {
			auto* pbStop = tc_out.add_stops();
			pbStop->set_name(stop.name_);
			pbStop->mutable_coordinates()->set_latitude(stop.coordinates_.lat);
			pbStop->mutable_coordinates()->set_longitude(stop.coordinates_.lng);
			for (const auto& dist : stop.distances_) {
				auto* pbDistances = pbStop->add_distances();
				pbDistances->set_name(dist.first);
				pbDistances->set_distance(dist.second);
			}
		}

		for (const auto& bus : wrappedBuses) {
			auto* pbBus = tc_out.add_buses();
			pbBus->set_name(bus.name_);
			for (const auto& stop : bus.stops_) {
				pbBus->add_stop_names(stop);
			}
			pbBus->set_is_circle(bus.IsCircle_);
		}
		return tc_out;
	}

	transport_catalogue_serialize::RenderSettings SaveRenderSettings(const transportcatalogue::json_reader::input::RenderSettings render_settings) {
		transport_catalogue_serialize::RenderSettings pbRenderSettings;
		pbRenderSettings.set_width(render_settings.width_);
		pbRenderSettings.set_height(render_settings.height_);
		pbRenderSettings.set_padding(render_settings.padding_);
		pbRenderSettings.set_line_width(render_settings.line_width_);
		pbRenderSettings.set_stop_radius(render_settings.stop_radius_);
		pbRenderSettings.set_bus_label_font_size(render_settings.bus_label_font_size_);
		pbRenderSettings.mutable_bus_label_offset()->set_x(render_settings.bus_label_offset_.x);
		pbRenderSettings.mutable_bus_label_offset()->set_y(render_settings.bus_label_offset_.y);
		pbRenderSettings.set_stop_label_font_size(render_settings.stop_label_font_size_);
		pbRenderSettings.mutable_stop_label_offset()->set_x(render_settings.stop_label_offset_.x);
		pbRenderSettings.mutable_stop_label_offset()->set_y(render_settings.stop_label_offset_.y);
		pbRenderSettings.set_underlayer_width(render_settings.underlayer_width_);

		*pbRenderSettings.mutable_underlayer_color() = GetColor(render_settings.underlayer_color_);
		for (const auto& color : render_settings.color_palette_) {
			*pbRenderSettings.add_color_palette() = GetColor(color);
		}
		return pbRenderSettings;
	}

	transport_catalogue_serialize::Router SaveRouter(const transportcatalogue::transport_router::TransportRouter& router)
	{
		transport_catalogue_serialize::Router pbRouter;
		auto settings = router.GetSettings();
		pbRouter.mutable_settings()->set_velocity(settings.velocity_);
		pbRouter.mutable_settings()->set_wait_time(settings.wait_time_);
		for (int i = 0; i < router.GetEdgeCount(); ++i) {
			graph::Edge<double> edge = router.GetEdgeById(i);
			transport_catalogue_serialize::Edge pbEdge;
			pbEdge.set_from(edge.from);
			pbEdge.set_to(edge.to);
			pbEdge.set_weight(edge.weight);
			pbEdge.set_id(i);
			*pbRouter.add_graph() = pbEdge;
			transport_catalogue_serialize::Item pbItem;
			auto item = router.GetItem(i);
			pbItem.set_bus_id(item.bus_id);
			pbItem.set_bus_name(item.bus_name);
			pbItem.set_span_count(item.span_count);
			uint32_t key = i;
			pbRouter.mutable_items()->insert({ key, pbItem });
		}
		return pbRouter;
	}

	transport_catalogue_serialize::Color GetColor(const svg::Color& color) {
		transport_catalogue_serialize::Color pbColor;
		if (std::holds_alternative<std::string>(color)) {
			pbColor.set_color_name(std::get<std::string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb = std::get<svg::Rgb>(color);
			pbColor.add_rgb_color(rgb.red);
			pbColor.add_rgb_color(rgb.green);
			pbColor.add_rgb_color(rgb.blue);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba = std::get<svg::Rgba>(color);
			pbColor.add_rgb_color(rgba.red);
			pbColor.add_rgb_color(rgba.green);
			pbColor.add_rgb_color(rgba.blue);
			pbColor.add_rgb_color(rgba.opacity);
		}
		return pbColor;
	}

	transportcatalogue::transport_router::TransportRouter LoadDB(transportcatalogue::TransportCatalogue& tc, renderer::MapRenderer& renderer,
			transportcatalogue::json_reader::input::SerializationSettings settings) {
		std::ifstream inp(settings.file_name_, std::ios::binary);

		transport_catalogue_serialize::TransportCatalogue tc_;
		tc_.ParseFromIstream(&inp);
		auto pbStops = tc_.stops();
		auto pbBuses = tc_.buses();
		transport_catalogue_serialize::RenderSettings pbRenderSettings = tc_.render_settings();
		transport_catalogue_serialize::Router pbRouter = tc_.router();
		tc_.Clear();
		
		for (const auto& pbStop : pbStops) {
			std::string name = pbStop.name();
			transportcatalogue::geo::Coordinates coords;
			coords.lat = pbStop.coordinates().latitude();
			coords.lng = pbStop.coordinates().longitude();
			std::vector<std::pair<std::string, int>> distances;
			for (auto& elem : pbStop.distances()) {
				distances.push_back({ std::move(elem.name()), std::move(elem.distance()) });
			}
			tc.AddStop({ name, coords, distances });
		}
		pbStops.Clear();

		for (const auto& pbBus : pbBuses) {
			std::string name = pbBus.name();
			std::vector<std::string> stop_names;
			for (const auto& stop : pbBus.stop_names()) {
				stop_names.push_back(stop);
			}
			bool isCircle = pbBus.is_circle();
			tc.AddBus({ name, stop_names, isCircle });
		}
		pbBuses.Clear();
		tc.ProcessDistances();

		transportcatalogue::json_reader::input::RenderSettings render_settings;
		render_settings.width_ = pbRenderSettings.width();
		render_settings.height_ = pbRenderSettings.height();
		render_settings.padding_ = pbRenderSettings.padding();
		render_settings.line_width_ = pbRenderSettings.line_width();
		render_settings.stop_radius_ = pbRenderSettings.stop_radius();
		render_settings.bus_label_font_size_ = pbRenderSettings.bus_label_font_size();
		render_settings.bus_label_offset_ = { pbRenderSettings.bus_label_offset().x(), pbRenderSettings.bus_label_offset().y() };
		render_settings.stop_label_font_size_ = pbRenderSettings.stop_label_font_size();
		render_settings.stop_label_offset_ = { pbRenderSettings.stop_label_offset().x(), pbRenderSettings.stop_label_offset().y() };
		render_settings.underlayer_width_ = pbRenderSettings.underlayer_width();

		render_settings.underlayer_color_ = ParseColor(pbRenderSettings.underlayer_color());
		std::vector<svg::Color> palette;
		for (const auto& elem : pbRenderSettings.color_palette()) {
			palette.push_back(ParseColor(elem));
		}
		render_settings.color_palette_ = std::move(palette);
		renderer.SetSettings(render_settings);

		graph::DirectedWeightedGraph<double> graph(tc.GetStopCount());
		std::map<graph::EdgeId, transportcatalogue::transport_router::TransportRouter::Item> items;
		for (const auto& edge : pbRouter.graph()) {
			graph::Edge<double> edg = { edge.from(), edge.to(), edge.weight() };
			graph.AddEdge(edg);
			auto pbItem = pbRouter.items().at(edge.id());
			transportcatalogue::transport_router::TransportRouter::Item item = { pbItem.bus_name(), pbItem.bus_id(), pbItem.span_count() };
			items[edge.id()] = item;
		}
		transportcatalogue::json_reader::input::RoutingSettings routing_settings = { pbRouter.mutable_settings()->wait_time(), pbRouter.mutable_settings()->velocity() };
		pbRouter.Clear();
		return transportcatalogue::transport_router::TransportRouter(routing_settings, tc, std::move(graph), std::move(items));
	}

	svg::Color ParseColor(transport_catalogue_serialize::Color color) {
		switch (color.rgb_color_size()) {
			case 0:	{
				return color.color_name();
			}
			case 3: {
				svg::Rgb col = { static_cast<uint8_t>(color.rgb_color()[0]), 
								static_cast<uint8_t>(color.rgb_color()[1]), 
								static_cast<uint8_t>(color.rgb_color()[2]) };
				return col;
			}
			case 4: {
				svg::Rgba col = { static_cast<uint8_t>(color.rgb_color()[0]), 
								static_cast<uint8_t>(color.rgb_color()[1]), 
								static_cast<uint8_t>(color.rgb_color()[2]), 
								color.rgb_color()[3] };
				return col;
			}
		}
	}
}// namespace serialization