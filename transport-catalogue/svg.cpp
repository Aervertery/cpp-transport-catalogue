#include "svg.h"

namespace svg {

    using namespace std::literals;

    void ColorPrinter::operator() (const std::monostate) {
        out_ << NoneColor;
    }

    void ColorPrinter::operator() (const std::string& color) {
        out_ << color;
    }

    void ColorPrinter::operator() (const Rgb rgb) {
        out_ << "rgb("sv << static_cast<int>(rgb.red) << ","sv << static_cast<int>(rgb.green) << "," << static_cast<int>(rgb.blue) << ")"sv;
    }

    void ColorPrinter::operator() (const Rgba rgba) {
        out_ << "rgba("sv << static_cast<int>(rgba.red) << ","sv << static_cast<int>(rgba.green) << "," << static_cast<int>(rgba.blue) << "," << rgba.opacity << ")"sv;
    }

    // ------------------ RenderContext ----------------------

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // --------------------- Object -----------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline -------------------

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        if (points_.empty()) {
            out << "";
            out << "\"";
            out << " />";
            return;
        }
        for (auto it = points_.begin(); it != points_.end() - 1; ++it) {
            out << it->x << "," << it->y << " ";
        }
        out << points_.rbegin()->x << "," << points_.rbegin()->y;
        out << "\"";
        RenderAttrs(out);
        out << "/>";
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    // ---------- Text ------------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        ParseData();
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" ";
        out << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" ";
        out << "font-size=\"" << font_size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out << ">";
        out << data_;
        out << "</text>";
    }

    void Text::ParseData() {
        for (size_t i = 0; i < data_.size(); ++i) {
            switch (data_[i]) {
            case 39:
                data_[i] = '&';
                data_.insert(i + 1, "apos;");
                continue;
            case 34:
                data_[i] = '&';
                data_.insert(i + 1, "quot;");
                continue;
            case 38:
                data_[i] = '&';
                data_.insert(i + 1, "amp;");
                continue;
            case 60:
                data_[i] = '&';
                data_.insert(i + 1, "lt;");
                continue;
            case 62:
                data_[i] = '&';
                data_.insert(i + 1, "gt;");
                continue;
            }
        }
    }

    // ----------------- Document -----------------------

    void Document::AddPtr(std::unique_ptr<Object>&& object) {
        objects_.emplace_back(std::move(object));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context(out, 2, 2);
        for (const auto& object : objects_) {
            object->Render(context);
        }
        out << "</svg>";
    }
}  // namespace svg