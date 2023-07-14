#include "map_renderer.h"

namespace renderer {

	bool operator<(const ObjectData& lhs, const ObjectData& rhs) {
		return lhs.name_ < rhs.name_;
	}

	MapRenderer::MapRenderer(transportcatalogue::json_reader::input::RenderSettings settings) :
		settings_(std::move(settings)) {}

	const transportcatalogue::json_reader::input::RenderSettings& MapRenderer::GetSettings() const {
		return settings_;
	}

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	svg::Point SphereProjector::operator()(transportcatalogue::geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

	svg::Circle MapRenderer::DrawStopMark(ObjectData& data) const {
		svg::Circle bus_stop;
		bus_stop.SetCenter(data.coordinates_)
			.SetRadius(settings_.stop_radius_)
			.SetFillColor("white");
		return bus_stop;
	}

	svg::Text MapRenderer::DrawStopName(ObjectData& data) const {
		svg::Text stop_name;
		stop_name.SetPosition(data.coordinates_)
			.SetOffset(settings_.stop_label_offset_)
			.SetFontSize(settings_.stop_label_font_size_)
			.SetFontFamily("Verdana")
			.SetData(data.name_)
			.SetFillColor("black");
		return stop_name;
	}

	svg::Text MapRenderer::DrawStopNameUnderlayer(ObjectData& data) const {
		svg::Text stop_name_underlayer;
		stop_name_underlayer.SetPosition(data.coordinates_)
			.SetOffset(settings_.stop_label_offset_)
			.SetFontSize(settings_.stop_label_font_size_)
			.SetFontFamily("Verdana")
			.SetData(data.name_)
			.SetFillColor(settings_.underlayer_color_)
			.SetStrokeColor(settings_.underlayer_color_)
			.SetStrokeWidth(settings_.underlayer_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		return stop_name_underlayer;
	}

	svg::Text MapRenderer::DrawBusName(ObjectData& data, size_t color_count)  const {
		svg::Text bus_name;
		bus_name.SetPosition(data.coordinates_)
			.SetOffset(settings_.bus_label_offset_)
			.SetFontSize(settings_.bus_label_font_size_)
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetData(data.name_)
			.SetFillColor(settings_.color_palette_[color_count]);
		return bus_name;
	}

	svg::Text MapRenderer::DrawBusNameUnderlayer(ObjectData& data) const {
		svg::Text bus_name_underlayer;
		bus_name_underlayer.SetPosition(data.coordinates_)
			.SetOffset(settings_.bus_label_offset_)
			.SetFontSize(settings_.bus_label_font_size_)
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetData(data.name_)
			.SetFillColor(settings_.underlayer_color_)
			.SetStrokeColor(settings_.underlayer_color_)
			.SetStrokeWidth(settings_.underlayer_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		return bus_name_underlayer;
	}
}