#include "stat_reader.h"

namespace stat_read {
	Request::Request(RequestType type_, std::string& text_) :
		type(type_),
		text(text_) {}

	const std::vector<Request>& Requests::GetRequests() const {
		return requests;
	}

	const std::vector<ReplyBus>& Requests::GetRepliesBus() const {
		return replies_bus;
	}

	const std::vector<ReplyStop>& Requests::GetRepliesStop() const {
		return replies_stop;
	}

	const std::vector<Reply>& Requests::GetReplies() const {
		return replies;
	}

	void Requests::AddRequest(RequestType type, std::string& text) {
		requests.push_back({ type, text });
		return;
	}

	void Requests::AddReply(std::string& name, int stops, size_t uniques, double length, double curvature) {
		replies_bus.push_back({ name, stops, uniques, length, curvature });
	}

	void Requests::AddReply(std::string& name, std::vector<std::string_view>& buses, bool isFound) {
		replies_stop.push_back({name, buses, isFound });
	}

	void Requests::AddReply(RequestType type, ReplyBus& rep1, ReplyStop& rep2) {
		replies.push_back({ type, rep1, rep2 });
	}

	void Requests::Load(std::istream& input) {
		int count = 0;
		for (std::string line; std::getline(input, line);) {
			std::string request_text;
			RequestType request_type;
			size_t pos_ = line.find_first_not_of(' ');
			if (pos_ == std::string::npos) {
				continue;
			}
			if (pos_ != 0) {
				line = line.substr(pos_, line.size() - pos_);
			}
			size_t pos = line.find_first_of(' ');
			if (line[0] == 'B') {
				request_type = RequestType::BUS;
			}
			else if (line[0] == 'S') {
				request_type = RequestType::STOP;
			}
			else {
				count = std::stoi(line);
				continue;
			}
			request_text = line.substr(pos + 1);
			size_t _pos = request_text.find_last_not_of(' ');
			if (_pos == std::string::npos) {
				continue;
			}
			request_text = request_text.substr(0, _pos + 1);
			AddRequest(request_type, request_text);
			--count;
			if (count == 0) {
				break;
			}
		}
	}

	void Requests::Print() {
		std::ofstream output("test4_out.txt");
		for (auto& reply : replies) {
			switch (reply.type) {
				case RequestType::BUS: 
					PrintBus(reply.rep1, output);
					continue;
				case RequestType::STOP:
					PrintStop(reply.rep2, output);
					continue;
			}
		}
	}

	void Requests::PrintBus(ReplyBus& reply, std::ofstream& output) {
		const auto default_precision{ std::cout.precision() };
		if (reply.stops == 0) {
			output << "Bus " << reply.name << ": not found" << std::endl;
			return;
		}
		output << "Bus " << reply.name << ": " << reply.stops << " stops on route, " << reply.unique_stops << " unique stops, " << std::setprecision(6) << reply.length << " route length, " << reply.curvature << " curvature" << std::endl;
		output << std::setprecision(default_precision);
	}

	void Requests::PrintStop(ReplyStop& reply, std::ofstream& output) {
		output << "Stop " << reply.name << ": ";
		if (!reply.isFound) {
			output << "not found" << std::endl;
			return;
		}
		else if (reply.buses.empty()) {
			output << "no buses" << std::endl;
			return;
		}
		else {
			output << "buses";
			for (auto& bus : reply.buses) {
				output << " " << bus;
			}
			output << std::endl;
		}
	}
}