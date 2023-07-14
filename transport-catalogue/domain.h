#pragma once
#include <string>
#include <vector>
#include <variant>
#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

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
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
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
				Object(std::string name) :
					name_(name) {}
				std::string name_;
			};

			struct Stop : public Object {
				Stop(std::string name, geo::Coordinates coordinates, std::vector<std::pair<std::string, int>> distances) :
					Object(name),
					coordinates_(coordinates),
					distances_(distances) {}

				//std::string name_;
				geo::Coordinates coordinates_;
				std::vector<std::pair<std::string, int>> distances_;
			};

			struct Bus : public Object {
				Bus(std::string name, std::vector<std::string> stops, bool IsCircle) :
					Object(name),
					stops_(stops),
					IsCircle_(IsCircle) {}

				//std::string name_;
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
		} //namespace input

		namespace stat_read {
			struct Bus {
				std::string name;
				int stops;
				size_t unique_stops;
				double length;
				double curvature;
				bool isFound;
			};

			struct Stop {
				std::string name;
				std::vector<std::string_view> buses;
				bool isFound;
			};
		} //namespace stat_read
	} //namespace json_reader
} //namespace transportcatalogue