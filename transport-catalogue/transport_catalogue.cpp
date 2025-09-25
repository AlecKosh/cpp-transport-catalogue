#include "transport_catalogue.h"

#include <unordered_set>
#include <string_view>
#include <optional>


namespace transport_catalogue {

bool CompareBusesByName::operator()(const Bus* lhs, const Bus* rhs) const {
        return lhs->name < rhs->name;
    }

void TransportCatalogue::AddStop(Stop&& stop) {
    stops_.push_back(std::move(stop));
    stops_name_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(Bus&& bus) {
    buses_.push_back(std::move(bus));
    bus_name_to_bus_[buses_.back().name] = &buses_.back();
    for (const auto& stop : buses_.back().stops) {
        bus_by_stop_[stop->name].insert(&buses_.back());
    }
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
    auto it = stops_name_to_stop_.find(stop_name);
    if (it != stops_name_to_stop_.end()) {
        return it->second; // Возвращаем адрес объекта
    }
    return nullptr; // Если элемент не найден
}

const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const {
    auto it = bus_name_to_bus_.find(bus_name);
    if (it != bus_name_to_bus_.end()) {
        return it->second; // Возвращаем адрес объекта
    }
    return nullptr; // Если автобус с таким именем не найден
}

void TransportCatalogue::AddDistance(std::string_view dep_stop, std::string_view dest_stop, int distance) {
    distances_[{GetStop(dep_stop), GetStop(dest_stop)}] = distance;
    if (distances_.find({GetStop(dest_stop), GetStop(dep_stop)}) == distances_.end()) {
        distances_[{GetStop(dest_stop), GetStop(dep_stop)}] = distance;
    }
}

BusStat TransportCatalogue::GetBusStat(const Bus& bus) const {
    BusStat stat;
    std::unordered_set<const Stop*> seen_stops;
    const Stop* prev_stop = nullptr;
    double route_length_direct = 0.0;
    for (auto stop : bus.stops) {
        ++stat.total_stops;
        if (seen_stops.count(stop) == 0) {
            ++stat.unique_stops;
            seen_stops.insert(stop);
        }
        if (prev_stop != nullptr) {
            // Добавляем расстояние между остановками к общей длине маршрута
            stat.route_length += GetDistance(prev_stop->name, stop->name);
            // Вычисляем прямое географическое расстояние между остановками
            route_length_direct += ComputeDistance(prev_stop->coordinates, stop->coordinates);
        }
        prev_stop = stop;
    }
    if (route_length_direct > 0) {
        stat.curvature = stat.route_length / route_length_direct;
    } else {
        stat.curvature = 1.0;
    }
    return stat;
}

const std::set<const Bus*, CompareBusesByName> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
    auto it = bus_by_stop_.find(stop_name);
    if (it != bus_by_stop_.end()) {
        return it->second;
    } else {
        return {};
    }
}

int TransportCatalogue::GetDistance(std::string_view dep_stop, std::string_view dest_stop) const {
    auto dist = distances_.find({GetStop(dep_stop), GetStop(dest_stop)});
    if (dist != distances_.end()) {
        return dist->second;
    }
    return 0;
}

} // namespace transport_catalogue
