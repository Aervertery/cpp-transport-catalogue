#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transportcatalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    //transportcatalogue::Stop* GetStop(const std::string& name);

    // Возвращает маршруты, проходящие через
    //transportcatalogue::Bus* GetBus(const std::string& name);

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

    json::Document ProcessRequests(std::vector< transportcatalogue::json_reader::stat_read::Request>& requests);

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transportcatalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
