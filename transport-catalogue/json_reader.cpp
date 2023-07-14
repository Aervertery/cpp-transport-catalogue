#include "json_reader.h"

namespace transportcatalogue {
	namespace json_reader {
		namespace input {
			Request::Request(RequestType type, std::variant<Stop, Bus>&& object) :
				type_(type),
				object_(std::move(object)) {}

			Stop Request::AsStop() const {
				return std::get<Stop>(object_);
			}

			Bus Request::AsBus() const {
				return std::get<Bus>(object_);
			}

			void Requests::LoadRequests(json::Document& document) {
				json::Array input_base = document.GetRoot().AsMap().at("base_requests").AsArray();
				for (const auto& element : input_base) {
					json::Dict elem = element.AsMap();
					if (elem.at("type").AsString() == "Stop") {
						AddRequest(RequestType::STOP, std::move(AddStop(elem)));
					}
					else if (elem.at("type").AsString() == "Bus") {
						AddRequest(RequestType::BUS, std::move(AddBus(elem)));
					}
				}
			}

			RenderSettings Requests::LoadSettings(json::Document& document) {
				json::Dict input_render = document.GetRoot().AsMap().at("render_settings").AsMap();
				double width = input_render.at("width").AsDouble();
				double height = input_render.at("height").AsDouble();
				double padding = input_render.at("padding").AsDouble();
				double line_width = input_render.at("line_width").AsDouble();
				double stop_radius = input_render.at("stop_radius").AsDouble();
				int bus_label_font_size = input_render.at("bus_label_font_size").AsInt();
				svg::Point bus_label_offset = { input_render.at("bus_label_offset").AsArray()[0].AsDouble(),
													input_render.at("bus_label_offset").AsArray()[1].AsDouble() };
				int stop_label_font_size = input_render.at("stop_label_font_size").AsInt();
				svg::Point stop_label_offset = { input_render.at("stop_label_offset").AsArray()[0].AsDouble(),
													input_render.at("stop_label_offset").AsArray()[1].AsDouble() };
				svg::Color underlayer_color;
				if (input_render.at("underlayer_color").IsString()) {
					underlayer_color = input_render.at("underlayer_color").AsString();
				}
				else if (input_render.at("underlayer_color").IsArray()) {
					if (input_render.at("underlayer_color").AsArray().size() == 3) {
						svg::Rgb col = { static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[0].AsInt()),
										static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[1].AsInt()),
										static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[2].AsInt()) };
						underlayer_color = col;
					}
					else if (input_render.at("underlayer_color").AsArray().size() == 4) {
						svg::Rgba col = { static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[0].AsInt()),
										static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[1].AsInt()),
										static_cast<uint8_t>(input_render.at("underlayer_color").AsArray()[2].AsInt()),
										input_render.at("underlayer_color").AsArray()[3].AsDouble() };
						underlayer_color = col;
					}
				}
				double underlayer_width = input_render.at("underlayer_width").AsDouble();
				std::vector<svg::Color> color_palette;
				for (auto& elem : input_render.at("color_palette").AsArray()) {
					if (elem.IsString()) {
						color_palette.push_back(elem.AsString());
					}
					else if (elem.IsArray()) {
						if (elem.AsArray().size() == 3) {
							svg::Rgb col = { static_cast<uint8_t>(elem.AsArray()[0].AsInt()),
											static_cast<uint8_t>(elem.AsArray()[1].AsInt()),
											static_cast<uint8_t>(elem.AsArray()[2].AsInt()) };
							color_palette.push_back(col);
						}
						else if (elem.AsArray().size() == 4) {
							svg::Rgba col = { static_cast<uint8_t>(elem.AsArray()[0].AsInt()),
											static_cast<uint8_t>(elem.AsArray()[1].AsInt()),
											static_cast<uint8_t>(elem.AsArray()[2].AsInt()),
											elem.AsArray()[3].AsDouble() };
							color_palette.push_back(col);
						}
					}
				}
				RenderSettings settings{ width, height, padding, line_width, stop_radius, bus_label_font_size, bus_label_offset, stop_label_font_size, stop_label_offset,
										underlayer_color, underlayer_width, color_palette };
				return settings;
			}

			void Requests::AddRequest(RequestType type, std::variant<Stop, Bus>&& object) {
				requests.push_back({ type, std::move(object) });
			}

			Stop Requests::AddStop(json::Dict& stop_info) {
				std::string name = stop_info.at("name").AsString();
				geo::Coordinates coords = { stop_info.at("latitude").AsDouble(), stop_info.at("longitude").AsDouble() };
				std::vector<std::pair<std::string, int>> distances;
				if (!stop_info.at("road_distances").AsMap().empty()) {
					for (auto& elem : stop_info.at("road_distances").AsMap()) {
						distances.push_back({ elem.first, elem.second.AsInt() });
					}
				}
				return { name, coords, distances };
			}

			Bus Requests::AddBus(json::Dict& bus_info) {
				std::string name = bus_info.at("name").AsString();
				bool IsCircle = bus_info.at("is_roundtrip").AsBool();
				std::vector<std::string> stops;
				if (!bus_info.at("stops").AsArray().empty()) {
					for (auto& elem : bus_info.at("stops").AsArray()) {
						stops.push_back(elem.AsString());
					}
				}
				if (!IsCircle) {
					std::vector<std::string> rev;
					for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
						rev.push_back(*it);
					}
					for (auto& elem : rev) {
						stops.push_back(std::move(elem));
					}
				}
				return { name, stops, IsCircle };
			}

			void Requests::ProcessRequests(TransportCatalogue& catalogue) {
				std::sort(requests.begin(), requests.end(), [&](auto& lhs, auto& rhs) {
					return lhs.type_ > rhs.type_;
					});
				bool stops_check = false;
				for (const auto& request : requests) {
					switch (request.type_) {
					case RequestType::STOP:
						catalogue.AddStop(request.AsStop());
						continue;
					case RequestType::BUS:
						if (!stops_check) {
							catalogue.ProcessDistances();
							stops_check = true;
						}
						catalogue.AddBus(request.AsBus());
						continue;
					case RequestType::MAP:
						continue;
					}
				}
			}
		} //namespace input

		namespace stat_read {

			Request::Request(RequestType type, std::string& text, int id) :
				type_(type),
				text_(text),
				id_(id) {}

			std::vector<Request>& Requests::GetRequests() {
				return requests;
			}

			void Requests::Load(json::Document& document) {
				json::Array outp = document.GetRoot().AsMap().at("stat_requests").AsArray();
				for (const auto& element : outp) {
					json::Dict elem = element.AsMap();
					std::string name;
					int id = elem.at("id").AsInt();
					if (elem.at("type").AsString() == "Stop") {
						name = elem.at("name").AsString();
						AddRequest(RequestType::STOP, name, id);
					}
					else if (elem.at("type").AsString() == "Bus") {
						std::string name = elem.at("name").AsString();
						AddRequest(RequestType::BUS, name, id);
					}
					else if (elem.at("type").AsString() == "Map") {
						AddRequest(RequestType::MAP, name, id);
					}
				}
			}

			void Requests::AddRequest(RequestType type, std::string& name, int id) {
				requests.push_back({ type, name, id });
			}
		} //namespace stat_read
	} //namespace json_reader

} //namespace transportcatalogue