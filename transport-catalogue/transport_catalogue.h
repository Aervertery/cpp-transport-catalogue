#pragma once

#include <algorithm>
#include <deque>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

namespace transportcatalogue {

using BusStops = std::pair<std::vector<std::string>, bool>;
using DistancesForStops = std::unordered_map<std::string_view, std::vector<std::pair<std::string, int>>>;

	class TransportCatalogue {
	public:
		void ProcessInput(input::Requests& requests);

		void ProcessStats(stat_read::Requests& requests);

	private:

		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		struct StopDistHasher {
			size_t operator() (std::pair<Stop*, Stop*> stops) const {
				std::uintptr_t address1 = reinterpret_cast<std::uintptr_t>(stops.first);
				std::uintptr_t address2 = reinterpret_cast<std::uintptr_t>(stops.second);
				return address1 * 37 + address2 * 37 * 37;
			}
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			double distance;
			bool IsCircle;
		};
		using StopsDistance = std::unordered_map<std::pair<Stop*, Stop*>, int, StopDistHasher>;

		std::deque<Stop> stops;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop;
		std::deque<Bus> buses;
		std::unordered_map<std::string_view, Bus*> busname_to_bus;
		StopsDistance stops_to_distance;
		DistancesForStops distances;

		void AddStop(std::string& text);

		void AddBus(std::string& text);

		double ComputeRouteLength(std::string& name);

		void ProcessDistances();

		void ComputeDistance(Stop* from, Stop* to);

	};
	
	namespace detail {

		std::pair<std::string, std::string> SplitNameText(std::string&& line);

		geo::Coordinates ParseCoordinates(std::string&& line);

		BusStops ParseStops(std::string&& line);
	}

}