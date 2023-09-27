#pragma once

#include "router.h"
#include "graph.h"
#include "domain.h"
#include "transport_catalogue.h"
#include <memory>
#include <map>

namespace transportcatalogue {

	namespace transport_router {

		class TransportRouter {
			struct Item {
				std::string bus_name;
				int bus_id;
				int span_count;
			};

			using Graph = graph::DirectedWeightedGraph<double>;
			json_reader::input::RoutingSettings settings_;
			std::map<graph::EdgeId, Item> items_;
			TransportCatalogue& tc_;
			Graph graph_;
			std::unique_ptr<graph::Router<double>> router_;
		public:
			TransportRouter(json_reader::input::RoutingSettings settings, TransportCatalogue& tc, size_t stop_count);

			TransportRouter& SetVelocity(double velocity);

			TransportRouter& SetWaitTime(int wait_time);

			graph::Edge<double> GetEdgeById(graph::EdgeId id) const;

			Item GetItem(graph::EdgeId id) const;

			int GetWaitTime() const;

			void ConstructGraph();

			double CalcTime(std::string& from, std::string& to);

			std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(std::string& from, std::string& to) const;
		private:
		};
	} //namespace transport_router
} //namespace transportcatalogue