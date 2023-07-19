#include "json_builder.h"

namespace json {

    using namespace std::literals;

    //-------------------- BaseContext --------------------------------------

    Builder::BaseContext::BaseContext(Builder& builder) :
        builder_(builder) {}

    Builder& Builder::BaseContext::BaseContext::GetBuilder() const {
        return builder_;
    }

    Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder::BaseContext Builder::BaseContext::BaseContext::Value(NodeValue value) {
        return builder_.Value(std::move(value));
    }

    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::BaseContext Builder::BaseContext::BaseContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext Builder::BaseContext::BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::BaseContext Builder::BaseContext::BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Node Builder::BaseContext::BaseContext::Build() {
        return builder_.Build();
    }

    //----------------------- Builder --------------------------

    Builder::DictValueContext Builder::Key(std::string key) {
        if (root_.IsNull() || (!node_stacks_.empty() && !node_stacks_.back()->IsDict())) {
            throw std::logic_error("Dict construction hasn't started: can't assign Key"s);
        }
        if (IsValueRoot()) {
            throw std::logic_error("Dict is under constuction with Key assigned: Can't assign new Key"s);
        }
        Node* value_ptr_ = &std::get<Dict>(node_stacks_.back()->GetValue())[std::move(key)];
        node_stacks_.push_back(value_ptr_);
        isKey = true;
        return BaseContext(*this);
    }

    Builder::BaseContext Builder::Value(NodeValue value) {
        if (IsValueRoot()) {
            throw std::logic_error("Multivalued node is under construction: can't assign Value"s);
        }
        if (IsKeyValue()) {
            throw std::logic_error("Dict construction has started, but no Key assigned: can't assign Value"s);
        }
        if (node_stacks_.empty() && root_.IsNull()) {
            root_.GetValue() = std::move(value);
        }
        else if (node_stacks_.back()->IsNull() && isKey) {
            node_stacks_.back()->GetValue() = std::move(value);
            node_stacks_.pop_back();
            isKey = false;
        }
        else if (!node_stacks_.empty() && node_stacks_.back()->IsArray()) {
            std::get<Array>(node_stacks_.back()->GetValue()).emplace_back(std::move(value));
        }
        return BaseContext(*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (IsValueRoot()) {
            throw std::logic_error("Multivalued node is under construction: can't start Array"s);
        }
        if (IsKeyValue()) {
            throw std::logic_error("Dict is under construction: can't assign Value before Key"s);
        }
        StartContainerConstruction<Array>();
        return BaseContext(*this);
    }

    Builder::BaseContext Builder::EndArray() {
        if (root_.IsNull() || (!node_stacks_.empty() && !node_stacks_.back()->IsArray()) || IsValueRoot()) {
            throw std::logic_error("Array construction is incomplete or hasn't started"s);
        }
        node_stacks_.pop_back();
        return BaseContext(*this);
    }

    Builder::DictItemContext Builder::StartDict() {
        if (IsValueRoot()) {
            throw std::logic_error("Multivalued node is under construction: can't start Dict"s);
        }
        if (IsKeyValue()) {
            throw std::logic_error("Dict is under construction: can't assign Value before Key"s);
        }
        StartContainerConstruction<Dict>();
        return BaseContext(*this);
    }

    Builder::BaseContext Builder::EndDict() {
        if (root_.IsNull() || (!node_stacks_.empty() && !node_stacks_.back()->IsDict()) || IsValueRoot()) {
            throw std::logic_error("Dict construction is incomplete or hasn't started"s);
        }
        node_stacks_.pop_back();
        return BaseContext(*this);
    }

    json::Node Builder::Build() {
        if (!node_stacks_.empty()) {
            throw std::logic_error("Can't build: construction is incomplete"s);
        }
        if (root_.IsNull()) {
            throw std::logic_error("Can't build: root is empty"s);
        }
        return root_;
    }

    bool Builder::IsValueRoot() {
        return node_stacks_.empty() && !root_.IsNull();
    }

    bool Builder::IsKeyValue() {
        return !node_stacks_.empty() && node_stacks_.back()->IsDict();
    }

} // namespace json