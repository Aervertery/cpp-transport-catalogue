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
		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
			size_t id_;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			bool IsCircle;
			size_t id_;
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
	public:

		void AddStop(const json_reader::input::Stop& stop);

		void AddBus(const json_reader::input::Bus& bus);

		void ProcessDistances();

		json_reader::stat_read::BusInfo GetBusInfo(std::string& name) const;

		json_reader::stat_read::StopInfo GetStopInfo(std::string& name) const;

		std::set<std::string_view> GetDrawableBuses() const;

		std::vector<geo::Coordinates> GetStopsCoordinates(std::string_view bus_name) const;

		bool IsRoundtrip(std::string& name) const;

		size_t GetUniqueStopCount(std::string& name) const;

		size_t GetStopCount() const;

		std::vector<std::string_view> GetStopNames(std::string& bus_name) const;

		std::vector<std::string_view> GetStopNames(std::string_view bus_name) const;

		std::string_view GetLastStopName(std::string& bus_name) const;

		std::string_view GetStopNameById(int id) const;

		int GetBusIdByName(std::string_view name) const;

		int GetStopIdByName(std::string& stop_name) const;

		int GetDistance(std::string& first, std::string& second) const;

		const std::vector<transportcatalogue::json_reader::input::Stop> WrapStops() const;

		const std::vector<transportcatalogue::json_reader::input::Bus> WrapBuses(std::set<std::string_view>& bus_names) const;

	private:

		Stop* GetStop(const std::string& name) const;

		Bus* GetBus(const std::string& name) const;

		transportcatalogue::json_reader::input::Stop PrepareStop(std::string_view stop_name) const;

		transportcatalogue::json_reader::input::Bus PrepareBus(std::string_view bus_name) const;

		std::pair<double, double> ComputeRouteLength(std::string& name) const;

	};
}