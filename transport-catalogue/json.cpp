#include "json.h"

using namespace std;

namespace json {

    namespace {
        const std::string ProcessEscapes(const std::string& input) {
            std::stringstream output;
            bool check = false;
            if (input[0] != '\"') {
                output << '\"';
                check = true;
            }
            for (size_t i = 0; i < input.size(); ++i) {
                if (input[i] == '\\') {
                    if (i < input.size()) {
                        switch (input[i]) {
                        case 'n':
                            output << "\\n";
                            break;
                        case 'r':
                            output << "\\r";
                            break;
                        case '"':
                            output << "\\\"";
                            break;
                        case 't':
                            output << '\t';
                            break;
                        case '\\':
                            output << "\\\\";
                            break;
                        default:
                            output << input[i];
                            break;
                        }
                    }
                    else {
                        output << input[i];
                    }
                    continue;
                }
                if (i < input.size()) {
                    switch (input[i]) {
                    case '\n':
                        output << "\\n";
                        break;
                    case '\r':
                        output << "\\r";
                        break;
                    case '\"':
                        output << '\\' << '\"';
                        break;
                    case '\t':
                        output << "\t";
                        break;
                    case '\\':
                        output << '\\' << '\\';
                        break;
                    default:
                        output << input[i];
                        break;
                    }
                }
                else {
                    output << input[i];
                }
            }
            if (check) {
                output << '\"';
            }
            return output.str();
        }

        Node LoadNode(istream& input);

        void SkipWhitespace(std::istream& input) {
            char c;
            while (input.get(c)) {
                if (!std::isspace(c)) {
                    input.unget();
                    break;
                }
            }
        }

        Node LoadArray(std::istream& input) {
            using namespace std::literals;

            SkipWhitespace(input);

            char c;
            if (!input.get(c)) {
                throw ParsingError("Expected '['");
            }

            Array result;

            input.unget();

            while (input >> c && c != ']') {
                input.unget(); // возвращаем символ обратно во входной поток

                Node value = LoadNode(input); // считываем значение элемента массива
                result.push_back(value); // добавляем значение в массив

                // после считывания значения, ожидается запятая или закрывающая скобка
                SkipWhitespace(input);
                if (input.peek() == ',') {
                    input.ignore(); // пропускаем запятую
                }
                else if (input.peek() != ']') {
                    throw ParsingError("Expected ',' or ']'");
                }

            }

            return Node(move(result));
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                //это нужно, чтобы символом-разделителем целой и дробной части
                //считалась точка, а не запятая
                //const std::string oldLocale = std::setlocale(LC_NUMERIC, nullptr);
                //std::setlocale(LC_NUMERIC, "C");
                double res = std::stod(parsed_num);
                //std::setlocale(LC_NUMERIC, oldLocale.c_str());
                return res;
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(std::istream& input) {
            Dict result;

            char c;
            if (!(input >> c)) {
                throw ParsingError("Dictionary parsing error: expected '{'");
            }
            input.unget();
            while (input >> c && c != '}') {
                if (c == ' ') {
                    continue;
                }

                std::string key = LoadString(input);

                if (!(input >> c) || c != ':') {
                    throw ParsingError("Dictionary parsing error: expected ':'");
                }

                Node value = LoadNode(input);

                result[key] = std::move(value);

                if (input >> c && c == ',') {
                    continue;
                }
                else if (c == '}') {
                    break;
                }
                else {
                    throw ParsingError("Dictionary parsing error: expected ',' or '}'");
                }
            }

            return Node(std::move(result));
        }

        bool LoadBool(std::istream& input) {
            std::string value;

            char c;
            while (input.get(c) && c != ',') {
                if (c == ' ' || c == '\\' || c == '\t' || c == '\n' || c == '\r' || c == '}' || c == ']') {
                    break;
                }
                value += c;
            }
            input.putback(c);
            if (value == "true") {
                return true;
            }
            else if (value == "false") {
                return false;
            }
            else {
                throw ParsingError("Invalid boolean value: " + value);
            }
        }

        nullptr_t LoadNull(std::istream& input) {
            std::string value;

            char c;
            while (input.get(c) && c != ',') {
                if (c == ' ' || c == '\\' || c == '\t' || c == '\n' || c == '\r') {
                    break;
                }
                value += c;
            }
            input.putback(c);

            if (value != "null") {
                throw ParsingError("Invalid null value: " + value);
            }

            return nullptr;
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return Node(LoadString(input));
            }
            else if (c == 'n') {
                input.putback(c);
                return Node(LoadNull(input));
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return Node(LoadBool(input));
            }
            else {
                input.putback(c);
                auto value = LoadNumber(input);
                if (std::holds_alternative<int>(value)) {
                    int value_ = std::get<int>(value);
                    return Node(value_);
                }
                else {
                    double value_ = std::get<double>(value);
                    return Node(value_);;
                }
            }
        }

    }  // namespace

    // ------------------- Node -------------------------

    Node::Node(NodeValue value) :
        NodeValue(std::move(value)) {}

    const NodeValue& Node::GetValue() const {
        return *this;
    }

    NodeValue& Node::GetValue() {
        return *this;
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Invalid type: expected int");
        }
        return std::get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Invalid type: expected bool");
        }
        return std::get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Invalid type: expected double");
        }
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        return std::get<double>(*this);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Invalid type: expected string");
        }
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Invalid type: expected array");
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw std::logic_error("Invalid type: expected map");
        }
        return std::get<Dict>(*this);
    }

    bool Node::operator==(const Node& other) const {
        return !(*this != other);
    }

    bool Node::operator!=(const Node& other) const {
        return this->index() != other.index() || this->GetValue() != other.GetValue();
    }

    //------------------------- Document ------------------------------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        (void)&doc;
        (void)&output;
        PrintContext ctx{ output };
        PrintNode(doc.GetRoot(), ctx);
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const std::string& string, const PrintContext& ctx) {
        const std::string str = ProcessEscapes(string);
        ctx.out << str;
    }

    void PrintValue(const bool b, const PrintContext& ctx) {
        ctx.out << std::boolalpha << b << std::noboolalpha;
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& elem : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(elem.first, ctx);
            out << ": "sv;
            PrintNode(elem.second, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

}  // namespace json