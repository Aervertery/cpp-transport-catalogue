#pragma once
#include "transport_catalogue.h"
#include "json.h"

namespace transportcatalogue {
	namespace json_reader {

		enum class RequestType {
			BUS,
			STOP,
			MAP,
		};
		namespace input {
			using BusStops = std::pair<std::vector<std::string>, bool>;

			struct Request {
			public:
				Request(RequestType type, std::variant<Stop, Bus>&& object);

				RequestType type_;
				std::variant<Stop, Bus> object_;

				Stop AsStop() const;

				Bus AsBus() const;
			};

			class Requests {
			public:

				void AddRequest(RequestType type, std::variant<Stop, Bus>&& object);

				void LoadRequests(json::Document& document);

				RenderSettings LoadSettings(json::Document& document);

				void ProcessRequests(TransportCatalogue& catalogue);

			private:
				std::vector<Request> requests;

				Stop AddStop(json::Dict& stop_info);

				Bus AddBus(json::Dict& bus_info);
			};
		} //namespace input

		namespace stat_read {

			struct Request {
				Request(RequestType type, std::string& text, int id);

				RequestType type_;
				std::string text_;
				int id_;
			};

			class Requests {
			public:
				std::vector<Request>& GetRequests();

				void AddRequest(RequestType type, std::string& name, int id);

				void Load(json::Document& document);

			private:
				std::vector<Request> requests;
			};
		} //namespace stat_read
	} //namespace json_reader

} //namespace transportcatalogue