#include "transport_router.h"

const double METERS_IN_KM = 1000.0;
const double MINUTES_IN_HOUR = 60.0;

namespace transportcatalogue {
	namespace transport_router {
		TransportRouter::TransportRouter(json_reader::input::RoutingSettings settings, TransportCatalogue& tc) :
			settings_(settings),
			tc_(tc),
			graph_(tc.GetStopCount()) {
				ConstructGraph();
				router_ = std::make_unique<graph::Router<double>>(graph_);
		}

		TransportRouter::TransportRouter(json_reader::input::RoutingSettings settings, TransportCatalogue& tc, Graph&& graph, std::map<graph::EdgeId, Item>&& items) :
			settings_(settings),
			tc_(tc),
			items_(items) {
				graph_ = graph;
				router_ = std::make_unique<graph::Router<double>>(graph_);
		}

		TransportRouter& TransportRouter::SetVelocity(double velocity) {
			settings_.velocity_ = velocity;
			return *this;
		}

		TransportRouter::TransportRouter(TransportCatalogue& tc) :
			tc_(tc) {}

		TransportRouter& TransportRouter::SetWaitTime(int wait_time) {
			settings_.wait_time_ = wait_time;
			return *this;
		}

		void TransportRouter::ConstructGraph(){
			std::set<std::string_view> buses = tc_.GetDrawableBuses();
			for (const auto bus : buses) {
				auto stops = tc_.GetStopNames(bus);
				for (size_t from = 0; from < stops.size(); ++from) {
					double time = settings_.wait_time_;
					for (size_t to = from + 1; to < stops.size(); ++to) {
						std::string stop_from(stops[from]), stop_to(stops[to]);
						graph::VertexId vertex_from = tc_.GetStopIdByName(stop_from);
						graph::VertexId vertex_to = tc_.GetStopIdByName(stop_to);
						std::string stop_to_prev(stops[to - 1]);
						time += CalcTime(stop_to_prev, stop_to);
						graph::EdgeId edge = graph_.AddEdge({ vertex_from, vertex_to, time });
						int span_count = to - from;
						items_[edge] = { std::string(bus), tc_.GetBusIdByName(bus), span_count };
					}
				}
			}
		}

		json_reader::input::RoutingSettings TransportRouter::GetSettings() const
		{
			return settings_;
		}

		double TransportRouter::CalcTime(std::string& from, std::string& to) {
			int length = tc_.GetDistance(from, to);
			double velocity = METERS_IN_KM * settings_.velocity_ / MINUTES_IN_HOUR;
			double time = length / velocity;
			return time;
		}

		std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRouteInfo(std::string& from, std::string& to) const {
			graph::VertexId vertex_from = tc_.GetStopIdByName(from);
			graph::VertexId vertex_to = tc_.GetStopIdByName(to);
			auto res = router_->BuildRoute(vertex_from, vertex_to);
			return res;
		}

		graph::Edge<double> TransportRouter::GetEdgeById(graph::EdgeId id) const {
			return graph_.GetEdge(id);
		}

		size_t TransportRouter::GetEdgeCount() const
		{
			return graph_.GetEdgeCount();
		}

		TransportRouter::Item TransportRouter::GetItem(graph::EdgeId id) const {
			return items_.at(id);
		}

		int TransportRouter::GetWaitTime() const {
			return settings_.wait_time_;
		}
	} //namespace transport_router
} //namespace transportcatalogue