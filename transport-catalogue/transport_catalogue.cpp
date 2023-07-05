#include "transport_catalogue.h"

using BusStops = std::pair<std::vector<std::string>, bool>;

void TransportCatalogue::ProcessInput(input::Requests& requests) {
	std::vector<input::Request> inprequests = requests.GetRequests();
	std::sort(inprequests.begin(), inprequests.end(), [&](auto& lhs, auto& rhs) {
		return lhs.type > rhs.type;
		});
	bool stops_check = false;
	for (auto& request : inprequests) {
		switch (request.type) {
			case input::RequestType::STOP:
				AddStop(request.text);
				continue;
			case input::RequestType::BUS:
				if (!stops_check) {
					ProcessDistances();
					stops_check = true;
				}
				AddBus(request.text);
				continue;
		}
	}
}

void TransportCatalogue::ProcessStats(stat_read::Requests& requests) {
	std::vector<stat_read::Request> statrequests = requests.GetRequests();
	for (auto& request : statrequests) {
		switch (request.type) {
		case stat_read::RequestType::BUS: {
			stat_read::ReplyStop rep2;
			auto res = busname_to_bus.find(request.text);
			if (res == busname_to_bus.end()) {
				stat_read::ReplyBus rep1 = { request.text, 0, 0, 0.0, 0.0 };
				//requests.AddReplyBus(request.text, 0, 0, 0.0);
				requests.AddReply(request.type, rep1, rep2);
				continue;
			}
			std::string name = res->second->name;
			int stops = res->second->stops.size();
			std::unordered_set<std::string_view> uniques;
			for (auto& stop : res->second->stops) {
				uniques.insert(stop->name);
			}
			double length = ComputeRouteLength(name);
			double curvature = (length * 1.0) / res->second->distance;
			stat_read::ReplyBus rep1 = { name, stops, uniques.size(), length, curvature };
			//requests.AddReplyBus(name, stops, uniques.size(), distance);
			requests.AddReply(stat_read::RequestType::BUS, rep1, rep2);
			continue; }
		case stat_read::RequestType::STOP: {
			stat_read::ReplyBus rep1;
			auto res = stopname_to_stop.find(request.text);
			std::vector<std::string_view> result;
			if (res == stopname_to_stop.end()) {
				stat_read::ReplyStop rep2 = { request.text, result, false };
				//requests.AddReplyStop(request.text, result, false);
				requests.AddReply(request.type, rep1, rep2);
				continue;
			}
			std::string name = res->second->name;
			for (auto& bus : buses) {
				auto it = busname_to_bus.find(bus.name);
				auto d = it->second->stops;
				for (auto elem : d) {
					if (elem->name == name) {
						result.push_back(bus.name);
						break;
					}
				}
			}
			std::sort(result.begin(), result.end());
			stat_read::ReplyStop rep2 = { request.text, result, true };
			//requests.AddReplyStop(request.text, result, true);
			requests.AddReply(request.type, rep1, rep2);
			continue; }
		}
	}
}

void TransportCatalogue::AddStop(std::string& text) {
	Coordinates coords;
	auto tmp = SplitNameText(std::move(text));
	std::string name = std::move(tmp.first);
	coords = ParseCoordinates(std::move(tmp.second));
	stops.push_back({ name, coords });
	while (!tmp.second.empty()) {
		size_t l = tmp.second.find_first_of('m');
		int length = std::stoi(tmp.second.substr(0, l));
		tmp.second = tmp.second.substr(l + 5, tmp.second.size());
		size_t n = tmp.second.find_first_of(',');
		if (n == std::string::npos) {
			std::string s_name = tmp.second;
			distances[stops.back().name].push_back({ s_name, length });
			break;
		}
		std::string s_name = tmp.second.substr(0, n);
		distances[stops.back().name].push_back({ s_name, length });
		tmp.second = tmp.second.substr(n + 2, tmp.second.size());
	}
	stopname_to_stop[stops.back().name] = &stops.back();
}

void TransportCatalogue::AddBus(std::string& text) {
	auto tmp = SplitNameText(std::move(text));
	std::string name = std::move(tmp.first);
	BusStops stopnames = ParseStops(std::move(tmp.second));
	std::vector<Stop*> stops;
	bool check = false;
	Stop* prev_ = nullptr;
	Stop* cur_ = nullptr;
	double distance = 0.0;
	double c_distance = 0.0;
	double c_total = 0.0;
	for (auto& stop : stopnames.first) { //Этот цикл в отдельный метод
		auto res = stopname_to_stop.find(stop);
		cur_ = res->second;
		stops.push_back(res->second);
		if (!check) {
			check = true;
			prev_ = res->second;
			continue;
		}
		if (stops_to_distance.find(std::make_pair(prev_, cur_)) == stops_to_distance.end()) {
			if (stops_to_distance.find(std::make_pair(cur_, prev_)) == stops_to_distance.end()) {
				distance = ComputeDistance(prev_->coordinates, cur_->coordinates);
			}
			else {
				auto it = stops_to_distance.find(std::make_pair(cur_, prev_));
				distance = it->second;
			}
			stops_to_distance.emplace(std::make_pair(prev_, cur_), distance);
		}
		c_distance = ComputeDistance(prev_->coordinates, cur_->coordinates);
		c_total += c_distance;
		prev_ = cur_;
	}
	if (!stopnames.second) {
		check = false;
		for (auto it = stopnames.first.rbegin(); it != stopnames.first.rend(); ++it) {
			auto res = stopname_to_stop.find(*it);
			cur_ = res->second;
			if (!check) {
				check = true;
				prev_ = res->second;
				continue;
			}
			stops.push_back(res->second);
			if (stops_to_distance.find(std::make_pair(prev_, cur_)) == stops_to_distance.end()) {
				if (stops_to_distance.find(std::make_pair(cur_, prev_)) == stops_to_distance.end()) {
					distance = ComputeDistance(prev_->coordinates, cur_->coordinates);
				}
				else {
					auto it = stops_to_distance.find(std::make_pair(cur_, prev_));
					distance = it->second;
				}
				stops_to_distance.emplace(std::make_pair(prev_, cur_), distance);
			}
			c_distance = ComputeDistance(prev_->coordinates, cur_->coordinates);
			c_total += c_distance;
			prev_ = cur_;
		}
	}
	buses.push_back({ name, stops, c_total, stopnames.second });
	busname_to_bus[buses.back().name] = &buses.back();
}

std::pair<std::string, std::string> TransportCatalogue::SplitNameText(std::string&& line) {
	std::string name, text;
	size_t dd = line.find_first_of(':');
	size_t ns = line.find_first_not_of(' ');
	name = line.substr(ns, dd - ns);
	text = line.substr(std::min(line.size() - 1, line.find_first_not_of(' ', dd + 1)), line.size());
	return std::make_pair(name, text);
}

Coordinates TransportCatalogue::ParseCoordinates(std::string&& line) {
	size_t dd = line.find_first_of(',');
	std::string tst = line.substr(0, std::min(line.size() - 1, dd));
	double lat = std::stod(tst);
	line = line.substr(dd + 2, line.size());
	size_t nn = line.find_first_of(',');
	tst = line.substr(0, nn);
	double lng = std::stod(tst);
	if (nn == std::string::npos) {
		line.clear();
		return { lat, lng };
	}
	line = line.substr(nn + 2, line.size());
	return { lat, lng };
}

BusStops TransportCatalogue::ParseStops(std::string&& line) {
	const char* pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789";
	std::vector<std::string> stops;
	bool IsCircle = false;
	size_t dd = line.find_first_of(">-");
	if (line[dd] == '>') {
		IsCircle = true;
	}
	while (!line.empty()) {
		size_t nn = line.find_last_of(pattern, dd);
		stops.push_back(std::move(line.substr(0, std::min(line.size() - 1, nn + 1))));
		size_t new_stop = line.find_first_of(pattern, dd);
		line = std::move(line.substr(new_stop, line.size()));
		dd = line.find_first_of(">-");
		if (dd == std::string::npos) {
			stops.push_back(std::move(line));
			break;
		}
	}
	return std::make_pair(stops, IsCircle);
}

double TransportCatalogue::ComputeRouteLength(std::string& name) {
	bool check = false;
	Stop* prev_ = nullptr;
	Stop* cur_ = nullptr;
	double distance = 0.0;
	double total = 0.0;
	auto bus_it = busname_to_bus.find(name);
	auto bus = *bus_it->second;
	for (auto& stop : bus.stops) {
		auto res = stopname_to_stop.find(stop->name);
		cur_ = res->second;
		if (!check) {
			check = true;
			prev_ = res->second;
			continue;
		}
		auto it = stops_to_distance.find(std::make_pair(prev_, cur_));
		if (it == stops_to_distance.end()) {
			auto it_ = stops_to_distance.find(std::make_pair(cur_, prev_));
			if (it_ != stops_to_distance.end()) {
				distance = it_->second;
			}
		}
		else {
			distance = it->second;
		}
		total += distance;
		prev_ = cur_;
	}
	/*if (!bus.IsCircle) {
		check = false;
		for (auto it = bus.stops.rbegin(); it != bus.stops.rend(); ++it) {
			auto stop = *it;
			auto res = stopname_to_stop.find(stop->name);
			cur_ = res->second;
			if (!check) {
				check = true;
				prev_ = res->second;
				continue;
			}
			auto _it = stops_to_distance.find(std::make_pair(prev_, cur_));
			if (_it == stops_to_distance.end()) {
				auto it_ = stops_to_distance.find(std::make_pair(cur_, prev_));
				if (it_ != stops_to_distance.end()) {
					distance = it_->second;
				}
			}
			else {
				distance = _it->second;
			}
			total += distance;
			prev_ = cur_;
		}
	}*/
	return total;
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