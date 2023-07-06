#pragma once

#include <algorithm>
#include <string>
#include <deque>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "geo.h"

namespace transportcatalogue {

	using DistancesForStops = std::unordered_map<std::string_view, std::vector<std::pair<std::string, int>>>;

	namespace input {
		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
			std::vector<std::pair<std::string, int>> distances;
		};

		struct Bus {
			std::string name;
			std::vector<std::string> stops;
			bool IsCircle;
		};
	}

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
	}

	class TransportCatalogue {
	public:

		void AddStop(const input::Stop& stop);

		void AddBus(const input::Bus& bus);

		void ProcessDistances();

		stat_read::Bus GetBusInfo(std::string& name);

		stat_read::Stop GetStopInfo(std::string& name);

	private:

		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			bool IsCircle;
		};


		struct StopDistHasher {
			size_t operator() (std::pair<Stop*, Stop*> stops) const {
				std::uintptr_t address1 = reinterpret_cast<std::uintptr_t>(stops.first);
				std::uintptr_t address2 = reinterpret_cast<std::uintptr_t>(stops.second);
				return address1 * 37 + address2 * 37 * 37;
			}
		};

		using StopsDistance = std::unordered_map<std::pair<Stop*, Stop*>, int, StopDistHasher>;

		std::deque<Stop> stops;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop;
		std::deque<Bus> buses;
		std::unordered_map<std::string_view, Bus*> busname_to_bus;
		StopsDistance stops_to_distance;
		DistancesForStops distances;

		Stop* GetStop(const std::string& name);

		Bus* GetBus(const std::string& name);

		std::pair<double, double> ComputeRouteLength(std::string& name);

	};
}