#include "request_handler.h"

RequestHandler::RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer) :
	db_(db),
	renderer_(renderer) {}

json::Document RequestHandler::ProcessRequests(std::vector< transportcatalogue::json_reader::stat_read::Request>& requests) {
	json::Array reply;
	for (auto& request : requests) {
		json::Dict res;
		res["request_id"] = json::Node(request.id_);
		switch (request.type_) {
		case transportcatalogue::json_reader::RequestType::BUS: {
			auto bus = db_.GetBusInfo(request.text_);
			if (!bus.isFound) {
				res["error_message"] = json::Node("not found");
			}
			else {
				res["curvature"] = json::Node(bus.curvature);
				res["route_length"] = json::Node(bus.length);
				res["stop_count"] = json::Node(bus.stops);
				res["unique_stop_count"] = json::Node(static_cast<int>(bus.unique_stops));
			}
			reply.push_back(json::Node(res));
			continue;
		}
		case transportcatalogue::json_reader::RequestType::STOP: {
			auto stop = db_.GetStopInfo(request.text_);
			if (!stop.isFound) {
				res["error_message"] = json::Node("not found");
			}
			else {
				json::Array buses;
				for (auto& bus : stop.buses) {
					std::string name(bus);
					buses.push_back(json::Node(name));
				}
				res["buses"] = json::Node(buses);
			}
			reply.push_back(json::Node(res));
			continue;
		}
		case transportcatalogue::json_reader::RequestType::MAP:
			auto map_res = RenderMap();
			std::ostringstream str;
			map_res.Render(str);
			res["map"] = json::Node(str.str());
			reply.push_back(json::Node(res));
			continue;
		}
	}
	json::Document doc(reply);
	return doc;
}

svg::Document RequestHandler::RenderMap() const {
	svg::Document doc;
	auto buses = db_.GetDrawableBuses();
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