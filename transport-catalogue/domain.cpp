#include "domain.h"

namespace svg {

	Rgb::Rgb(uint8_t rd, uint8_t grn, uint8_t bl) :
		red(rd),
		green(grn),
		blue(bl) {}

	Rgba::Rgba(uint8_t rd, uint8_t grn, uint8_t bl, double opcty) :
		red(rd),
		green(grn),
		blue(bl),
		opacity(opcty) {}

	Point::Point(double x, double y) : 
		x(x),
		y(y) {}
} //namespace svg

namespace transportcatalogue {

	namespace json_reader {
		namespace input {

			Object::Object(std::string name) :
				name_(name) {}

			Stop::Stop(std::string name, geo::Coordinates coordinates, std::vector<std::pair<std::string, int>> distances) :
				Object(name),
				coordinates_(coordinates),
				distances_(distances) {}

			Bus::Bus(std::string name, std::vector<std::string> stops, bool IsCircle) :
				Object(name),
				stops_(stops),
				IsCircle_(IsCircle) {}
		} //namespace input

		namespace stat_read {

		} //namespace stat_read
	} //namespace json_reader
} //namespace transportcatalogue