#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include "domain.h"

namespace svg {

    /*struct Rgb {
        Rgb() = default;

        Rgb(uint8_t rd, uint8_t grn, uint8_t bl);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;

        Rgba(uint8_t rd, uint8_t grn, uint8_t bl, double opcty);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };*/

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct ColorPrinter {
        std::ostream& out_;

        void operator() (const std::monostate);

        void operator() (const std::string& color);

        void operator() (const Rgb rgb);

        void operator() (const Rgba rgba);
    };

    inline std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt";
            break;
        case StrokeLineCap::ROUND:
            os << "round";
            break;
        case StrokeLineCap::SQUARE:
            os << "square";
            break;
        }
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
        switch (line_join) {
        case StrokeLineJoin::MITER:
            os << "miter";
            break;
        case StrokeLineJoin::ROUND:
            os << "round";
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel";
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip";
            break;
        case StrokeLineJoin::ARCS:
            os << "arcs";
            break;
        }
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(ColorPrinter{ os }, color);
        return os;
    }

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:

        template <typename ObjectType>
        void Add(const ObjectType& object);

        virtual ~ObjectContainer() = default;

    protected:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    private:
        //std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:

        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };

    template <typename Owner>
    class PathProps {
    public:

        Owner& SetFillColor(const Color& color) {
            fill_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeColor(const Color& color) {
            stroke_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

        ~PathProps() = default;

    protected:

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;

        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */

    private:
        std::vector<Point> points_;

        void RenderObject(const RenderContext& context) const override;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>

    private:
        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";

        void RenderObject(const RenderContext& context) const override;

        void ParseData();

    };

    class Document final : public ObjectContainer {
    public:

        Document() = default;

        //template <typename ObjectType>
        //void Add(const ObjectType& object);

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    template <typename ObjectType>
    void ObjectContainer::Add(const ObjectType& object) {
        AddPtr(std::make_unique<ObjectType>(object));
    }
} // namespace svg
