#pragma once
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include "geo.h"

namespace svg {

	struct Rgb {
		Rgb() = default;

		Rgb(uint8_t rd, uint8_t grn, uint8_t bl);

		uint8_t red = 0;
		uint8_t green = 0;
		uint8_t blue = 0;
	};

	struct Rgba {
		Rgba() = default;

		Rgba(uint8_t rd, uint8_t grn, uint8_t bl, double opcty);

		uint8_t red = 0;
		uint8_t green = 0;
		uint8_t blue = 0;
		double opacity = 1.0;
	};

	struct Point {
		Point() = default;
		Point(double x, double y);
		double x = 0;
		double y = 0;
	};

	using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

	inline const Color NoneColor{ "none" };
} //namespace svg

namespace transportcatalogue {

	namespace json_reader {
		namespace input {

			struct Object {
				Object(std::string name);
				std::string name_;
			};

			struct Stop : public Object {
				Stop(std::string name, geo::Coordinates coordinates, std::vector<std::pair<std::string, int>> distances);

				geo::Coordinates coordinates_;
				std::vector<std::pair<std::string, int>> distances_;
			};

			struct Bus : public Object {
				Bus(std::string name, std::vector<std::string> stops, bool IsCircle);

				std::vector<std::string> stops_;
				bool IsCircle_;
			};

			struct RenderSettings {
				double width_;
				double height_;
				double padding_;
				double line_width_;
				double stop_radius_;
				int bus_label_font_size_;
				svg::Point bus_label_offset_;
				int stop_label_font_size_;
				svg::Point stop_label_offset_;
				svg::Color underlayer_color_;
				double underlayer_width_;
				std::vector<svg::Color> color_palette_;
			};

			struct RoutingSettings {
				int wait_time_;
				double velocity_;
			};
		} //namespace input

		namespace stat_read {
			struct BusInfo {
				std::string name;
				int stops;
				size_t unique_stops;
				double length;
				double curvature;
				bool isFound;
			};

			struct StopInfo {
				std::string name;
				std::vector<std::string_view> buses;
				bool isFound;
			};
		} //namespace stat_read
	} //namespace json_reader
} //namespace transportcatalogue