#include "input_reader.h"

namespace transportcatalogue {
	namespace input {
		Request::Request(RequestType type_, std::string& text_) :
			type(type_),
			text(text_) {}

		const std::vector<Request>& Requests::GetRequests() const {
			return requests;
		}

		void Requests::AddRequest(RequestType type, std::string& text) {
			requests.push_back({ type, text });
			return;
		}

		void Requests::Load(std::istream& input) {
			int count = 0;
			for (std::string line; std::getline(input, line);) {
				std::string request_text;
				RequestType request_type;
				size_t pos = line.find_first_of(' ');
				if (line[0] == 'B') {
					request_type = RequestType::BUS;
				}
				else if (line[0] == 'S') {
					request_type = RequestType::STOP;
				}
				else {
					count = std::stoi(line);
					if (count == 0) {
						break;
					}
					continue;
				}
				request_text = line.substr(pos + 1);
				AddRequest(request_type, request_text);
				--count;
				if (count == 0) {
					break;
				}
			}
		}

		void Requests::ProcessRequests(TransportCatalogue& catalogue) {
			std::sort(requests.begin(), requests.end(), [&](auto& lhs, auto& rhs) {
				return lhs.type > rhs.type;
				});
			bool stops_check = false;
			for (auto& request : requests) {
				switch (request.type) {
				case input::RequestType::STOP:
					catalogue.AddStop(AddStop(request.text));
					continue;
				case input::RequestType::BUS:
					if (!stops_check) {
						catalogue.ProcessDistances();
						stops_check = true;
					}
					catalogue.AddBus(AddBus(request.text));
					continue;
				}
			}
		}

		Stop Requests::AddStop(std::string& text) {
			geo::Coordinates coords;
			auto tmp = detail::SplitNameText(std::move(text));
			std::string name = std::move(tmp.first);
			coords = detail::ParseCoordinates(std::move(tmp.second));
			std::vector<std::pair<std::string, int>> distances;
			while (!tmp.second.empty()) {
				size_t l = tmp.second.find_first_of('m');
				int length = std::stoi(tmp.second.substr(0, l));
				tmp.second = tmp.second.substr(l + 5, tmp.second.size());
				size_t n = tmp.second.find_first_of(',');
				if (n == std::string::npos) {
					std::string s_name = tmp.second;
					distances.push_back({ s_name, length });
					break;
				}
				std::string s_name = tmp.second.substr(0, n);
				distances.push_back({ s_name, length });
				tmp.second = tmp.second.substr(n + 2, tmp.second.size());
			}
			Stop stop = { name, coords, distances };
			return stop;
		}

		Bus Requests::AddBus(std::string& text) {
			auto tmp = detail::SplitNameText(std::move(text));
			std::string name = std::move(tmp.first);
			BusStops stopnames = detail::ParseStops(std::move(tmp.second));
			if (!stopnames.second) {
				std::vector<std::string> rev;
				for (auto it = stopnames.first.rbegin() + 1; it != stopnames.first.rend(); ++it) {
					rev.push_back(*it);
				}
				for (auto& elem : rev) {
					stopnames.first.push_back(std::move(elem));
				}
			}
			Bus bus = { name, stopnames.first, stopnames.second };
			return bus;
		}

		namespace detail {
			std::pair<std::string, std::string> SplitNameText(std::string&& line) {
				std::string name, text;
				size_t dd = line.find_first_of(':');
				size_t ns = line.find_first_not_of(' ');
				name = line.substr(ns, dd - ns);
				text = line.substr(std::min(line.size() - 1, line.find_first_not_of(' ', dd + 1)), line.size());
				return std::make_pair(name, text);
			}

			geo::Coordinates ParseCoordinates(std::string&& line) {
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

			BusStops ParseStops(std::string&& line) {
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
		}
	}
}