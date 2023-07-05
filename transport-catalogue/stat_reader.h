#pragma once
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_set>
#include <iomanip>

namespace stat_read {

	enum class RequestType {
		BUS,
		STOP,
	};

	struct Request {
		Request(RequestType type_, std::string& text_);

		RequestType type;
		std::string text;
	};

	struct ReplyBus {

		std::string name;
		int stops;
		size_t unique_stops;
		double length;
		double curvature;
	};

	struct ReplyStop {

		std::string name;
		std::vector<std::string_view> buses;
		bool isFound;
	};

	struct Reply {
		RequestType type;
		ReplyBus rep1;
		ReplyStop rep2;
	};

	class Requests {
	public:
		const std::vector<Request>& GetRequests() const;

		const std::vector<ReplyBus>& GetRepliesBus() const;

		const std::vector<ReplyStop>& GetRepliesStop() const;

		const std::vector<Reply>& GetReplies() const;

		void AddRequest(RequestType type, std::string& text);

		void AddReply(std::string& name, int stops, size_t uniques, double length, double curvature);

		void AddReply(std::string& name, std::vector<std::string_view>& buses, bool isFound);

		void AddReply(RequestType type, ReplyBus& rep1, ReplyStop& rep2);

		void Load(std::istream& input);

		void Print();

		void PrintBus(ReplyBus& reply, std::ofstream& output);

		void PrintStop(ReplyStop& reply, std::ofstream& output);

	private:
		std::vector<Request> requests;
		std::vector<ReplyBus> replies_bus;
		std::vector<ReplyStop> replies_stop;
		std::vector<Reply> replies;
		
	};
}
