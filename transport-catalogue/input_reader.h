#pragma once
#include <iostream>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include "geo.h"
#include "transport_catalogue.h"

namespace transportcatalogue {
	namespace input {
		using BusStops = std::pair<std::vector<std::string>, bool>;

		enum class RequestType {
			BUS,
			STOP,
		};

		struct Request {
			Request(RequestType type_, std::string& text_);

			RequestType type;
			std::string text;
		};

		class Requests {
		public:
			const std::vector<Request>& GetRequests() const;

			void AddRequest(RequestType type, std::string& text);

			void Load(std::istream& input);

			void ProcessRequests(TransportCatalogue& catalogue);

		private:
			std::vector<Request> requests;

			Stop AddStop(std::string& text);

			Bus AddBus(std::string& text);
		};

		namespace detail {

			std::pair<std::string, std::string> SplitNameText(std::string&& line);

			geo::Coordinates ParseCoordinates(std::string&& line);

			BusStops ParseStops(std::string&& line);
		}

	}
}