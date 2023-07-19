#pragma once
#include "json.h"

namespace json {

	class Builder {
		class DictValueContext;

		class DictItemContext;

		class ArrayItemContext;

		class BaseContext {
			Builder& builder_;
		public:
			explicit BaseContext(Builder& builder);

			Builder& GetBuilder() const;

			DictValueContext Key(std::string key);

			BaseContext Value(NodeValue value);

			DictItemContext StartDict();

			BaseContext EndDict();

			ArrayItemContext StartArray();

			BaseContext EndArray();

			Node Build();
		};

		class DictItemContext final : public BaseContext {
		public:
			DictItemContext(BaseContext context) :
				BaseContext(context.GetBuilder()) {}

			BaseContext Value(NodeValue) = delete;
			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			BaseContext EndArray() = delete;
			Node Build() = delete;
		};

		class DictValueContext final : public BaseContext {
		public:
			DictValueContext(BaseContext context) :
				BaseContext(context.GetBuilder()) {}

			DictItemContext Value(NodeValue value) {
				return BaseContext::Value(std::move(value));
			}

			DictValueContext Key(std::string) = delete;
			BaseContext EndDict() = delete;
			BaseContext EndArray() = delete;
			Node Build() = delete;
		};

		class ArrayItemContext final : public BaseContext {
		public:
			ArrayItemContext(BaseContext context) :
				BaseContext(context.GetBuilder()) {}

			ArrayItemContext Value(NodeValue value) {
				return BaseContext::Value(std::move(value));
			}

			DictValueContext Key(std::string) = delete;
			BaseContext EndDict() = delete;
			Node Build() = delete;
		};

		Node root_;
		std::vector<Node*> node_stacks_;
		bool isKey = false;
	public:
		DictValueContext Key(std::string key);

		BaseContext Value(NodeValue value);

		DictItemContext StartDict();

		BaseContext EndDict();

		ArrayItemContext StartArray();

		BaseContext EndArray();

		Node Build();

		bool IsValueRoot();

		bool IsKeyValue();

		template<typename Type>
		void StartContainerConstruction();
	};

	template<typename Type>
	void Builder::StartContainerConstruction() {
		if (node_stacks_.empty() && root_.IsNull()) {
			Node* tmp_ptr = reinterpret_cast<Node*>(&root_.GetValue().emplace<Type>());
			node_stacks_.push_back(tmp_ptr);
			return;
		}
		else if ((!node_stacks_.empty() && node_stacks_.back()->IsNull()) && isKey) {
			node_stacks_.back()->GetValue() = Type{};
			isKey = false;
			return;
		}
		else if (!node_stacks_.empty() && node_stacks_.back()->IsArray()) {
			Node* tmp_ptr = static_cast<Node*> (&std::get<Array>(node_stacks_.back()->GetValue()).emplace_back(Type{}));
			node_stacks_.push_back(tmp_ptr);
			return;
		}
	}
}