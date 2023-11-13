cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(JSON_FILES json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h)
set(BASE_FILES domain.cpp domain.h geo.cpp geo.h graph.h ranges.h router.h svg.cpp svg.h)
set(MAIN_FILES main.cpp map_renderer.cpp map_renderer.h request_handler.cpp request_handler.h serialization.cpp serialization.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h)

add_executable(tc ${PROTO_SRCS} ${PROTO_HDRS} ${JSON_FILES} ${BASE_FILES} ${MAIN_FILES})
target_include_directories(tc PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(tc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(tc "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)