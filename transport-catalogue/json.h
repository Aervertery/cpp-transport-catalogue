#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node : Value {
    public:
        /* Реализуйте Node, используя std::variant */

        /*Node() = default;

        template<typename Type>
        Node(Type value) :
            value_(std::move(value)) {}*/

        const Value& GetValue() const;

        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
    private:
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    template <typename Val>
    void PrintValue(const Val& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx);

    void PrintValue(const std::string& string, const PrintContext& ctx);

    void PrintValue(const bool b, const PrintContext& ctx);

    void PrintValue(const Array& array, const PrintContext& ctx);

    void PrintValue(const Dict& dict, const PrintContext& ctx);

    void PrintNode(const Node& node, const PrintContext& ctx);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);

}  // namespace json