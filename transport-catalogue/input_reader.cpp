#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace transport_catalogue {

namespace input_reader {

namespace parse {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
transport_catalogue::geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

} // namespace parse

namespace detail {

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

} // namespace detail

namespace parse {

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return detail::Split(route, '>');
    }

    auto stops = detail::Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

std::pair<std::string_view, std::string_view> ParseStopDescription(std::string_view line) {
    std::pair<std::string_view, std::string_view> res;
    size_t second_comma_pos = line.find(",", line.find(",") + 1);
    if (second_comma_pos != std::string::npos) {
        res.first = line.substr(0, second_comma_pos);
    } else {
        res = {line, ""};
        return res;
    }
    res.second = line.substr(second_comma_pos);
    return res;
}

std::vector<std::pair<int, std::string_view>> ParseDistances(std::string_view line) {
    std::vector<std::pair<int, std::string_view>> distances;
    size_t start = 0;
    while (start < line.length()) {
        size_t to_pos = line.find("to", start);
        if (to_pos == std::string_view::npos) break;
        std::string_view distance_str = line.substr(start + 1, to_pos - 1 - start);
        std::size_t d_start = distance_str.find_first_not_of(", ");
        std::size_t d_end = distance_str.rfind('m');
        int distance = std::stoi(std::string(distance_str.substr(d_start, d_end - d_start)));
        start = to_pos + 3;
        size_t comma_pos = line.find(',', start);
        std::string_view stop_name;
        if (comma_pos != std::string_view::npos) {
            stop_name = line.substr(start, comma_pos - start);
            start = comma_pos + 1;
        } else {
            stop_name = line.substr(start);
            distances.emplace_back(distance, stop_name);
            break;
        }
        distances.emplace_back(distance, stop_name);
    }
    return distances;
}

} // namespace parse

void InputReader::ParseLine(std::string_view line) {
    auto command_description = parse::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    std::vector<std::pair<std::string_view, std::pair<std::string_view, int>>> distances;
    for (const CommandDescription& com: commands_) {
        if (com.command == "Stop") {
            Stop stop;
            stop.name = com.id;
            auto sotp_parse = parse::ParseStopDescription(com.description);
            stop.coordinates = parse::ParseCoordinates(sotp_parse.first);
            catalogue.AddStop(std::move(stop));
            if (!sotp_parse.second.empty()) {
                for (auto distance_and_stop: parse::ParseDistances(sotp_parse.second)) {
                    distances.push_back({com.id, {distance_and_stop.second, distance_and_stop.first}});
                }
            }
        }
    }
    for (const auto& dist: distances) {
        catalogue.AddDistance(dist.first, dist.second.first, dist.second.second);
    }
    for (const CommandDescription& com: commands_) {
        if (com.command == "Bus") {
            Bus bus;
            bus.name = com.id;
            std::vector<const Stop*> stops;
            for (auto stop_name: parse::ParseRoute(com.description)) {
                stops.push_back(catalogue.GetStop(stop_name));
            }
            bus.stops = stops;
            catalogue.AddBus(std::move(bus));
        }
    }
}

} // namespace input_reader

} // namespace transport_catalogue