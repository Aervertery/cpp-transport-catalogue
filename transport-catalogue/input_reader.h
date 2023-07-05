#pragma once
#include <iostream>
#include <utility>
#include <string>
#include <vector>

namespace transportcatalogue {
	namespace input {

		enum class RequestType {
			BUS,
			STOP,
		};

		struct Request {
			Request(RequestType type_, std::string& text_);

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
}