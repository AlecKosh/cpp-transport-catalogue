#include "stat_reader.h"

#include <iostream>

namespace transport_catalogue {

namespace stat_reader {

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    // Находим позицию пробела в строке запроса
    auto space_pos = request.find(' ');
    std::string_view command_name = request.substr(0, space_pos);
    if (command_name == "Bus") {
        std::string_view bus_name = request.substr(space_pos + 1);
        // Ищем автобус по имени
        auto bus = tansport_catalogue.GetBus(bus_name);
        if (bus) {
            BusStat stat = tansport_catalogue.GetBusStat(*bus);
            output << "Bus " << bus_name << ": " << stat.total_stops << " stops on route, " << stat.unique_stops << " unique stops, " 
            << stat.route_length << " route length" << std::endl;
        } else {
            output << command_name << " " << bus_name << ": not found" << std::endl;
        }
    }
    if (command_name == "Stop") {
        std::string_view stop_name = request.substr(space_pos + 1);
        auto stop = tansport_catalogue.GetStop(stop_name);
        if (stop) {
            auto buses = tansport_catalogue.GetBusesByStop(stop_name);
            if (!buses.empty()) {
                output << command_name << " " << stop_name << ": buses ";
                for (const auto& bus : buses) {
                    output << bus->name << " ";
                }
                output << std::endl;
            } else {
                output << command_name << " " << stop_name << ": no buses" << std::endl;
            }
        } else {
            output << command_name << " " << stop_name << ": not found" << std::endl;
        }
    }
}

} // namespace stat_reader

} // namespace transport_catalogue