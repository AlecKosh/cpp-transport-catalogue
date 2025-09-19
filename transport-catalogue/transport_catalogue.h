#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>

namespace transport_catalogue {

struct Stop {
	std::string name;
    transport_catalogue::geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
    std::vector<const Stop*> stops;
};

struct BusStat {
	size_t total_stops = 0;
    size_t unique_stops = 0;
    double route_length = 0.;

};

struct CompareBusesByName {
    bool operator()(const Bus* lhs, const Bus* rhs) const;
};

class TransportCatalogue {
	public:

	//добавление маршрута в базу
	void AddBus(Bus&& bus);

	// добавление остановки в базу
	void AddStop(Stop&& stop);

	// поиск маршрута по имени
	const Bus* GetBus(std::string_view bus_name_) const;

	// поиск остановки по имени
	const Stop* GetStop(std::string_view stop_name) const;

	// получение информации о маршруте
	BusStat GetBusStat(const Bus& bus) const;

	// получение списка автобусов по остановке
	const std::set<const Bus*, CompareBusesByName> GetBusesByStop(std::string_view stop_name) const;

	private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stops_name_to_stop_;
	std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;
	std::unordered_map<std::string_view, std::set<const Bus*, CompareBusesByName>> bus_by_stop_;
};

} // namespace transport_catalogue