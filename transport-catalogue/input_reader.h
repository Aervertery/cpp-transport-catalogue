#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_set>

namespace input {

	enum class RequestType {
		BUS,
		STOP,
	};

	struct Request {
		Request(RequestType type_, std::string&text_);

		RequestType type;
		std::string text;
	};
	class Requests {
	public:
		const std::vector<Request>& GetRequests() const;

		void AddRequest(RequestType type, std::string& text);

		void Load(std::istream& input);

	private:
		std::vector<Request> requests;
	};

	
}