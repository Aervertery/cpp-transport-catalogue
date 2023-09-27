#include "transport_catalogue.h"
namespace transportcatalogue {
	using BusStops = std::pair<std::vector<std::string>, bool>;

	void TransportCatalogue::AddStop(const json_reader::input::Stop& stop) {
		stops.push_back({ stop.name_, stop.coordinates_, stops.size()});
		stopname_to_stop[stops.back().name] = &stops.back();
		distances[stops.back().name] = stop.distances_;
	}

	void TransportCatalogue::AddBus(const json_reader::input::Bus& bus) {
		std::vector<TransportCatalogue::Stop*> stops;
		for (const auto& elem : bus.stops_) {
			stops.push_back(GetStop(elem));
		}
		buses.push_back({ bus.name_, stops, bus.IsCircle_, buses.size() });
		busname_to_bus[buses.back().name] = &buses.back();
	}

	TransportCatalogue::Stop* TransportCatalogue::GetStop(const std::string& name) const {
		return stopname_to_stop.find(name)->second;
	}

	TransportCatalogue::Bus* TransportCatalogue::GetBus(const std::string& name) const {
		return busname_to_bus.find(name)->second;
	}

	json_reader::stat_read::StopInfo TransportCatalogue::GetStopInfo(std::string& name) const {
		json_reader::stat_read::BusInfo rep1;
		auto res = stopname_to_stop.find(name);
		std::vector<std::string_view> result;
		if (res == stopname_to_stop.end()) {
			json_reader::stat_read::StopInfo rep2 = { name, result, false };
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
		json_reader::stat_read::StopInfo rep2 = { name, result, true };
		return rep2;
	}

	json_reader::stat_read::BusInfo TransportCatalogue::GetBusInfo(std::string& name) const {
		json_reader::stat_read::StopInfo rep2;
		auto res = busname_to_bus.find(name);
		if (res == busname_to_bus.end()) {
			json_reader::stat_read::BusInfo rep1 = { name, 0, 0, 0.0, 0.0, false };
			return rep1;
		}
		std::string name_ = res->second->name;
		int stops = res->second->stops.size();
		size_t uniques = GetUniqueStopCount(res->second->name);
		std::pair<double, double> length = ComputeRouteLength(name_);
		json_reader::stat_read::BusInfo rep1 = { name, stops, uniques, length.first, length.second, true };
		return rep1;
	}

	std::set<std::string_view> TransportCatalogue::GetDrawableBuses() const {
		std::set<std::string_view> result;
		for (auto& bus : busname_to_bus) {
			if (!bus.second->stops.empty()) {
				result.insert(bus.first);
			}
		}
		return result;
	}

	std::vector<geo::Coordinates> TransportCatalogue::GetStopsCoordinates(std::string_view bus_name) const {
		std::vector<geo::Coordinates> result;
		for (auto stop : busname_to_bus.at(bus_name)->stops) {
			result.push_back(stop->coordinates);
		}
		return result;
	}

	bool TransportCatalogue::IsRoundtrip(std::string& name) const {
		return busname_to_bus.at(name)->IsCircle;
	}

	size_t TransportCatalogue::GetUniqueStopCount(std::string& name) const {
		std::unordered_set<std::string_view> uniques;
		for (auto& stop : busname_to_bus.at(name)->stops) {
			uniques.insert(stop->name);
		}
		return uniques.size();
	}

	size_t TransportCatalogue::GetStopCount() const {
		return stops.size();
	}

	std::vector<std::string_view> TransportCatalogue::GetStopNames(std::string& bus_name) const {
		std::vector<std::string_view> uniques;
		size_t count = busname_to_bus.at(bus_name)->stops.size();
		size_t c = 0;
		while (c != count) {
			uniques.push_back(busname_to_bus.at(bus_name)->stops[c]->name);
			++c;
		}
		return uniques;
	}

	std::vector<std::string_view> TransportCatalogue::GetStopNames(std::string_view bus_name) const {
		std::vector<std::string_view> uniques;
		size_t count = busname_to_bus.at(bus_name)->stops.size();
		size_t c = 0;
		while (c != count) {
			uniques.push_back(busname_to_bus.at(bus_name)->stops[c]->name);
			++c;
		}
		return uniques;
	}

	std::string_view TransportCatalogue::GetLastStopName(std::string& bus_name) const {
		auto it = busname_to_bus.at(bus_name)->stops.rbegin();
		auto r = *it;
		return r->name;
	}

	int TransportCatalogue::GetDistance(std::string& first, std::string& second_) const {
		auto t = distances.find(first)->second;
		auto s = std::find_if(t.begin(), t.end(), [=](const auto& elem) 
							{ return elem.first == second_; });
		if (t.size() == 0 || s == t.end()) {
			t = distances.find(second_)->second;
			auto s = std::find_if(t.begin(), t.end(), [=](const auto& elem)
				{ return elem.first == first; });
			auto res = s->second;
			return res;
		}
		auto res = s->second;
		return res;
	}

	std::string_view TransportCatalogue::GetStopNameById(int id) const {
		return stops.at(id).name;
	}

	int TransportCatalogue::GetBusIdByName(std::string_view name) const {
		return busname_to_bus.at(name)->id_;
	}

	int TransportCatalogue::GetStopIdByName(std::string& stop_name) const {
		return stopname_to_stop.at(stop_name)->id_;
	}

	//length, curvature
	std::pair<double, double> TransportCatalogue::ComputeRouteLength(std::string& name) const {
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
		for (auto& stop : distances) {
			if (stop.second.empty()) {
				for (auto& stop_ : distances) {
					auto test = stop_.second;
					std::string name(stop.first);
					for (auto& el : stop_.second) {
						if (el.first == name) {
							auto first = *stopname_to_stop.find(stop.first);
							auto second = *stopname_to_stop.find(stop_.first);
							stops_to_distance.emplace(std::make_pair(first.second, second.second), el.second);
						}
					}
				}
			}
		}
	}
} //namespace transportcatalogue