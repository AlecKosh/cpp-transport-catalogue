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
    return nullptr; // // Если автобус с таким именем не найден
}

BusStat TransportCatalogue::GetBusStat(const Bus& bus) const {
    BusStat stat;
    std::unordered_set<std::string_view> seen_stops;
    std::optional<transport_catalogue::geo::Coordinates> prev_pos;
    for (auto stop : bus.stops) {
        ++stat.total_stops;
        if (seen_stops.count(stop->name) == 0) {
            ++stat.unique_stops;
            seen_stops.insert(stop->name);
        }
        if (prev_pos) {
            stat.route_length += ComputeDistance(*prev_pos, stop->coordinates);
        }
        prev_pos = stop->coordinates;
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

} // namespace transport_catalogue
