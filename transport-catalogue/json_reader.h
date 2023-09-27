#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include <variant>

namespace transportcatalogue {
	namespace json_reader {

		enum class RequestType {
			BUS,
			STOP,
			MAP,
			ROUTING,
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

				RenderSettings LoadRenderSettings(json::Document& document);

				RoutingSettings LoadRoutingSettings(json::Document& document);

				void ProcessRequests(TransportCatalogue& catalogue);

			private:
				std::vector<Request> requests;

				Stop AddStop(json::Dict& stop_info);

				Bus AddBus(json::Dict& bus_info);
			};
		} //namespace input

		namespace stat_read {
			using RequestText = std::variant<std::string, std::pair<std::string, std::string>>;
			struct Request {
				Request(RequestType type, RequestText& text, int id);

				RequestType type_;
				RequestText text_;
				int id_;

				bool IsString() const;

				bool IsPair() const;

				std::pair<std::string, std::string> AsPair() const;

				std::string AsString() const;
			};

			class Requests {
			public:
				std::vector<Request>& GetRequests();

				void AddRequest(RequestType type, RequestText& name, int id);

				void Load(json::Document& document);

			private:
				std::vector<Request> requests;
			};
		} //namespace stat_read
	} //namespace json_reader

} //namespace transportcatalogue