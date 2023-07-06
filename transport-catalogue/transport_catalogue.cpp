#include "transport_catalogue.h"
namespace transportcatalogue {
	using BusStops = std::pair<std::vector<std::string>, bool>;

	void TransportCatalogue::AddStop(const input::Stop& stop) {
		stops.push_back({ stop.name, stop.coordinates });
		stopname_to_stop[stops.back().name] = &stops.back();
		distances[stops.back().name] = stop.distances;
	}

	void TransportCatalogue::AddBus(const input::Bus& bus) {
		std::vector<TransportCatalogue::Stop*> stops;		
		for (const auto& elem : bus.stops) {
			stops.push_back(GetStop(elem));
		}
		buses.push_back({ bus.name, stops, bus.IsCircle});
		busname_to_bus[buses.back().name] = &buses.back();
	}

	TransportCatalogue::Stop* TransportCatalogue::GetStop(const std::string& name) {
		return stopname_to_stop.find(name)->second;
	}

	TransportCatalogue::Bus* TransportCatalogue::GetBus(const std::string& name) {
		return busname_to_bus.find(name)->second;
	}

	stat_read::Stop TransportCatalogue::GetStopInfo(std::string& name) {
		stat_read::Bus rep1;
		auto res = stopname_to_stop.find(name);
		std::vector<std::string_view> result;
		if (res == stopname_to_stop.end()) {
			stat_read::Stop rep2 = { name, result, false };
			return rep2;
		}
		std::string name_ = res->second->name;
		for (auto& bus : buses) {
			auto it = busname_to_bus.find(bus.name);
			auto d = it->second->stops;
			for (auto elem : d) {
				if (elem->name == name_) {
					result.push_back(bus.name);
					break;
				}
			}
		}
		std::sort(result.begin(), result.end());
		stat_read::Stop rep2 = { name, result, true };
		return rep2;
	}

	stat_read::Bus TransportCatalogue::GetBusInfo(std::string& name) {
		stat_read::Stop rep2;
		auto res = busname_to_bus.find(name);
		if (res == busname_to_bus.end()) {
			stat_read::Bus rep1 = { name, 0, 0, 0.0, 0.0, false };
			return rep1;
		}
		std::string name_ = res->second->name;
		int stops = res->second->stops.size();
		std::unordered_set<std::string_view> uniques;
		for (auto& stop : res->second->stops) {
			uniques.insert(stop->name);
		}
		std::pair<double, double> length = ComputeRouteLength(name_);
		stat_read::Bus rep1 = { name, stops, uniques.size(), length.first, length.second, true };
		return rep1;
	}

	//length, curvature
	std::pair<double, double> TransportCatalogue::ComputeRouteLength(std::string& name) {
		Bus* bus = GetBus(name);
		double distance_str = 0.0;
		double distance_real = 0.0;
		bool check = false;
		Stop* cur = nullptr;
		Stop* prev = nullptr;
		for (auto stop : bus->stops) {
			if (!check) {
				check = true;
				prev = stop;
				continue;
			}
			cur = stop;
			distance_str += geo::ComputeDistance(prev->coordinates, cur->coordinates);
			auto it = stops_to_distance.find(std::make_pair(prev, cur));
			if (it == stops_to_distance.end()) {
				auto it_ = stops_to_distance.find(std::make_pair(cur, prev));
				if (it_ == stops_to_distance.end()) {
					distance_real += distance_str;
				}
				else {
					distance_real += it_->second;
				}
			}
			else {
				distance_real += it->second;
			}
			prev = cur;
		}
		return { distance_real, distance_real / distance_str };
	}

	void TransportCatalogue::ProcessDistances() {
		for (auto& stop : distances) {
			for (auto& stop_ : stop.second) {
				auto first = *stopname_to_stop.find(stop.first);
				auto second = *stopname_to_stop.find(stop_.first);
				stops_to_distance.emplace(std::make_pair(first.second, second.second), stop_.second);
			}
		}
	}
}