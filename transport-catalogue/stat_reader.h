#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <utility>
#include <fstream>
#include "transport_catalogue.h"

namespace transportcatalogue {
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

		class Requests {
		public:
			const std::vector<Request>& GetRequests() const;

			void AddRequest(RequestType type, std::string& text);

			void Load(std::istream& input);

			void PrintBus(Bus& reply, std::ofstream& output);

			void PrintStop(Stop& reply, std::ofstream& output);

			void ProcessRequests(TransportCatalogue& catalogue);

		private:
			std::vector<Request> requests;

		};
	}
}
