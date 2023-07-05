#include "input_reader.h"

namespace transportcatalogue {
	namespace input {
		Request::Request(RequestType type_, std::string& text_) :
			type(type_),
			text(text_) {}

		const std::vector<Request>& Requests::GetRequests() const {
			return requests;
		}

		void Requests::AddRequest(RequestType type, std::string& text) {
			requests.push_back({ type, text });
			return;
		}

		void Requests::Load(std::istream& input) {
			int count = 0;
			for (std::string line; std::getline(input, line);) {
				std::string request_text;
				RequestType request_type;
				size_t pos = line.find_first_of(' ');
				if (line[0] == 'B') {
					request_type = RequestType::BUS;
				}
				else if (line[0] == 'S') {
					request_type = RequestType::STOP;
				}
				else {
					count = std::stoi(line);
					if (count == 0) {
						break;
					}
					continue;
				}
				request_text = line.substr(pos + 1);
				AddRequest(request_type, request_text);
				--count;
				if (count == 0) {
					break;
				}
			}
		}
	}
}