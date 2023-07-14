#pragma once

#include <algorithm>
#include <string>
#include <deque>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "domain.h"

namespace transportcatalogue {

	using DistancesForStops = std::unordered_map<std::string_view, std::vector<std::pair<std::string, int>>>;

	class TransportCatalogue {
	public:

		void AddStop(const json_reader::input::Stop& stop);

		void AddBus(const json_reader::input::Bus& bus);

		void ProcessDistances();

		json_reader::stat_read::Bus GetBusInfo(std::string& name) const;

		json_reader::stat_read::Stop GetStopInfo(std::string& name) const;

		std::set<std::string_view> GetDrawableBuses() const;

		std::vector<geo::Coordinates> GetStopsCoordinates(std::string_view bus_name) const;

		bool IsRoundtrip(std::string& name) const;

		size_t GetUniqueStopCount(std::string& name) const;

		std::vector<std::string_view> GetStopNames(std::string& bus_name) const;

		std::string_view GetLastStopName(std::string& bus_name) const;

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

		Stop* GetStop(const std::string& name) const;

		Bus* GetBus(const std::string& name) const;

		std::pair<double, double> ComputeRouteLength(std::string& name) const;

	};
}