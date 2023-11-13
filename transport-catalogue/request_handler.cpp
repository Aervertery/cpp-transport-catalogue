#include "request_handler.h"

RequestHandler::RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer,
				const transportcatalogue::transport_router::TransportRouter& router, 
				transportcatalogue::json_reader::input::SerializationSettings serialization_settings) :
	db_(db),
	renderer_(renderer),
	router_(router),
	serialization_settings_(serialization_settings) {}

json::Document RequestHandler::ProcessRequests(std::vector<transportcatalogue::json_reader::stat_read::Request>& requests) {
	json::Builder builder;
	builder.StartArray();
	for (auto& request : requests) {
		std::string not_found = "not found";
		builder.StartDict().Key("request_id").Value(request.id_);
		switch (request.type_) {
		case transportcatalogue::json_reader::RequestType::BUS: {
			std::string text = request.AsString();
			auto bus = db_.GetBusInfo(text);
			if (!bus.isFound) {
				builder.Key("error_message").Value(not_found);
			}
			else {
				builder.Key("curvature").Value(bus.curvature)
					.Key("route_length").Value(bus.length)
					.Key("stop_count").Value(bus.stops)
					.Key("unique_stop_count").Value(static_cast<int>(bus.unique_stops));
			}
			builder.EndDict();
			continue;
		}
		case transportcatalogue::json_reader::RequestType::STOP: {
			std::string text = request.AsString();
			auto stop = db_.GetStopInfo(text);
			if (!stop.isFound) {
				builder.Key("error_message").Value(not_found);
			}
			else {
				json::Array buses;
				builder.Key("buses").StartArray();
				for (auto& bus : stop.buses) {
					std::string name(bus);
					builder.Value(name);
					buses.push_back(json::Node(name));
				}
				builder.EndArray();
			}
			builder.EndDict();
			continue;
		}
		case transportcatalogue::json_reader::RequestType::MAP:
		{
			auto map_res = RenderMap();
			std::ostringstream str;
			map_res.Render(str);
			builder.Key("map").Value(str.str()).EndDict();
			continue;
		}
		case transportcatalogue::json_reader::RequestType::ROUTING:
			auto [from, to] = request.AsPair();
			auto route = router_.GetRouteInfo(from, to);
			int wait_time = router_.GetWaitTime();
			if (!route) {
				builder.Key("error_message").Value(not_found);
			}
			else {
				builder.Key("items").StartArray();
				for (const auto& id : route.value().edges) {
					auto edge = router_.GetEdgeById(id);
					std::string stop_name(db_.GetStopNameById(edge.from));
					builder.StartDict().Key("type").Value(std::string("Wait"))
						.Key("stop_name").Value(stop_name)
						.Key("time").Value(wait_time).EndDict();
					auto item = router_.GetItem(id);
					double move_time = edge.weight - wait_time;
					builder.StartDict().Key("type").Value(std::string("Bus"))
						.Key("bus").Value(std::string(item.bus_name))
						.Key("span_count").Value(item.span_count)
						.Key("time").Value(move_time).EndDict();
				}
				builder.EndArray().Key("total_time").Value(route.value().weight);
			}
			builder.EndDict();
			continue;
		}
	}
	builder.EndArray();
	json::Document doc(builder.Build().AsArray());
	return doc;
}

void RequestHandler::SetSerializationSettings(transportcatalogue::json_reader::input::SerializationSettings& serialization_settings) {
	serialization_settings_ = serialization_settings;
}

bool RequestHandler::SaveDB() const {
	std::ofstream outp(serialization_settings_.file_name_, std::ios::binary);

	if (!outp) {
		return false;
	}
	transport_catalogue_serialize::TransportCatalogue tc = serialization::SaveCatalogue(db_);
	*tc.mutable_render_settings() = serialization::SaveRenderSettings(renderer_.GetSettings());
	*tc.mutable_router() = serialization::SaveRouter(router_);
	tc.SerializePartialToOstream(&outp);
	return true;
}

svg::Document RequestHandler::RenderMap() const {
	svg::Document doc;
	std::set<std::string_view> buses = db_.GetDrawableBuses();
	std::vector<transportcatalogue::geo::Coordinates> all_coords;
	for (auto bus_name : buses) {
		for (auto coords : db_.GetStopsCoordinates(bus_name)) {
			all_coords.push_back(coords);
		}
	}
	const renderer::SphereProjector proj({ all_coords.begin(), all_coords.end(), renderer_.GetSettings().width_,
										renderer_.GetSettings().height_, renderer_.GetSettings().padding_ });
	all_coords.clear();
	all_coords.resize(0);
	std::vector<std::vector<svg::Point>> svg_coords;
	for (auto bus_name : buses) {
		std::vector<svg::Point> crds;
		for (auto coords : db_.GetStopsCoordinates(bus_name)) {
			crds.push_back(proj(coords));
		}
		svg_coords.push_back(crds);
	}

	size_t color_count = 0;
	auto bus_it = buses.begin();
	std::set<renderer::ObjectData, std::less<renderer::ObjectData>> _stop_marks;
	std::set<renderer::ObjectData, std::less<renderer::ObjectData>> _stop_names;
	std::vector<std::pair<renderer::ObjectData, size_t>> _bus_names;
	for (auto& line : svg_coords) {
		svg::Polyline line_;
		std::string name(*bus_it);
		bool isRoundtrip = db_.IsRoundtrip(name);
		auto stop_names = db_.GetStopNames(name);
		size_t stop_count = isRoundtrip ? stop_names.size() : (stop_names.size() / 2) + 1; //кол-во остановок в одну сторону
		auto stop_name_it = stop_names.begin();
		size_t count = 0;
		size_t stops_total = db_.GetBusInfo(name).stops;
		bool check_first = true;
		for (auto& point : line) {
			++count;
			std::string stop_name_;
			if (stop_name_it != stop_names.end()) {
				stop_name_ = (*stop_name_it);
				++stop_name_it;
			}
			line_.AddPoint(point);
			if (check_first) {
				_bus_names.push_back({ { name, point }, color_count });
				_stop_marks.insert({ stop_name_, point });
				_stop_names.insert({ stop_name_, point });
				check_first = false;
				continue;
			}
			if (count <= stop_count) {
				_stop_marks.insert({ stop_name_, point });
				_stop_names.insert({ stop_name_, point });
			}
			if (!isRoundtrip && count == stop_count) {
				if (stop_name_ == db_.GetStopNames(name)[0]) {
					continue;
				}
				_bus_names.push_back({ { name, point }, color_count });
			}
			if (count == stops_total && db_.GetLastStopName(name) != stop_names.begin()->data()) {
				_stop_names.insert({ stop_name_, point });
			}
		}
		line_.SetStrokeColor(renderer_.GetSettings().color_palette_[color_count])
			.SetFillColor(svg::NoneColor)
			.SetStrokeWidth(renderer_.GetSettings().line_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc.Add(line_);
		if (color_count == renderer_.GetSettings().color_palette_.size() - 1) {
			color_count = 0;
		}
		else {
			++color_count;
		}
		++bus_it;
	}
	for (auto& elem : _bus_names) {
		doc.Add(renderer_.DrawBusNameUnderlayer(elem.first));
		doc.Add(renderer_.DrawBusName(elem.first, elem.second));
	}
	for (auto& elem : _stop_marks) {
		auto t = elem;
		doc.Add(renderer_.DrawStopMark(t));
	}
	for (auto& elem : _stop_names) {
		auto t = elem;
		doc.Add(renderer_.DrawStopNameUnderlayer(t));
		doc.Add(renderer_.DrawStopName(t));
	}
	return doc;
}