#include "TravelAgency.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "json.hpp"

using json = nlohmann::json;

namespace {
struct ObjectRange {
    std::size_t start;
    std::size_t end;
};

std::vector<ObjectRange> extractTopLevelArrayObjects(const std::string &content) {
    std::vector<ObjectRange> ranges;
    bool inString = false;
    bool escape = false;
    int arrayDepth = 0;
    int braceDepth = 0;
    bool insideObject = false;
    std::size_t objectStart = 0;

    for (std::size_t i = 0; i < content.size(); ++i) {
        char ch = content[i];
        if (escape) {
            escape = false;
            continue;
        }
        if (ch == '\\') {
            if (inString) {
                escape = true;
            }
            continue;
        }
        if (ch == '"') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }

        switch (ch) {
        case '[':
            ++arrayDepth;
            break;
        case ']':
            if (arrayDepth > 0) {
                --arrayDepth;
                if (arrayDepth == 0) {
                    braceDepth = 0;
                    insideObject = false;
                }
            }
            break;
        case '{':
            if (arrayDepth == 1 && !insideObject) {
                insideObject = true;
                braceDepth = 1;
                objectStart = i;
            } else if (insideObject) {
                ++braceDepth;
            }
            break;
        case '}':
            if (insideObject) {
                --braceDepth;
                if (braceDepth == 0) {
                    insideObject = false;
                    ranges.push_back({objectStart, i});
                }
            }
            break;
        default:
            break;
        }
    }

    return ranges;
}

std::size_t computeLineNumber(const std::string &content, std::size_t position) {
    std::size_t line = 1;
    for (std::size_t i = 0; i < position && i < content.size(); ++i) {
        if (content[i] == '\n') {
            ++line;
        }
    }
    return line;
}

std::string requireString(const json &value, const std::string &key, const std::string &path,
                          std::size_t lineNumber) {
    if (!value.contains(key)) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Missing attribute '" + key + "'.");
    }
    if (!value[key].is_string()) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Attribute '" + key + "' must be a string.");
    }
    std::string result = value[key].get<std::string>();
    if (result.empty()) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Attribute '" + key + "' must not be empty.");
    }
    return result;
}

double requirePrice(const json &value, const std::string &path, std::size_t lineNumber) {
    if (!value.contains("price")) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Missing attribute 'price'.");
    }
    if (!value["price"].is_number()) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Attribute 'price' must be numeric.");
    }
    double price = value["price"].get<double>();
    if (!std::isfinite(price)) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Attribute 'price' must be finite.");
    }
    return price;
}

std::vector<std::string> requireStringArray(const json &value, const std::string &key, const std::string &path,
                                            std::size_t lineNumber) {
    if (!value.contains(key)) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Missing attribute '" + key + "'.");
    }
    if (!value[key].is_array()) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Attribute '" + key + "' must be an array.");
    }
    std::vector<std::string> result;
    for (const auto &entry : value[key]) {
        if (!entry.is_string()) {
            throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Entries in '" + key + "' must be strings.");
        }
        result.push_back(entry.get<std::string>());
    }
    return result;
}

} // namespace

TravelAgency::~TravelAgency() {
    clear();
}

void TravelAgency::addBooking(std::unique_ptr<Booking> booking) {
    bookings_.push_back(std::move(booking));
}

void TravelAgency::readFile(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Could not open JSON file: " + path);
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string content = buffer.str();

    json root;
    try {
        root = json::parse(content);
    } catch (const json::parse_error &ex) {
        throw std::runtime_error(path + ":" + std::to_string(ex.byte) + ": JSON parse error: " + std::string(ex.what()));
    }

    const json *bookingsNode = nullptr;
    if (root.is_array()) {
        bookingsNode = &root;
    } else if (root.is_object() && root.contains("bookings")) {
        bookingsNode = &root["bookings"];
    }

    if (!bookingsNode || !bookingsNode->is_array()) {
        throw std::runtime_error(path + ":1: JSON must contain an array of bookings.");
    }

    auto ranges = extractTopLevelArrayObjects(content);
    if (ranges.size() != bookingsNode->size()) {
        ranges.resize(bookingsNode->size());
    }

    clear();

    for (std::size_t index = 0; index < bookingsNode->size(); ++index) {
        const auto &bookingNode = (*bookingsNode)[index];
        std::size_t lineNumber = ranges.empty() ? 1 : computeLineNumber(content, ranges[index].start);

        if (!bookingNode.is_object()) {
            throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Booking entry must be an object.");
        }

        std::string id = requireString(bookingNode, "id", path, lineNumber);
        if (existsId(id)) {
            throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Duplicate booking id '" + id + "'.");
        }

        std::string type = requireString(bookingNode, "type", path, lineNumber);
        double price = requirePrice(bookingNode, path, lineNumber);
        std::string fromDate = requireString(bookingNode, "fromDate", path, lineNumber);
        std::string toDate = requireString(bookingNode, "toDate", path, lineNumber);

        if (type == "Flight") {
            std::string fromAirport = requireString(bookingNode, "fromAirport", path, lineNumber);
            std::string toAirport = requireString(bookingNode, "toAirport", path, lineNumber);
            if (!isAirportCode(fromAirport) || !isAirportCode(toAirport)) {
                throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Airport codes must have exactly three alphabetic characters.");
            }
            std::string airline = requireString(bookingNode, "airline", path, lineNumber);
            addBooking(std::make_unique<FlightBooking>(id, price, fromDate, toDate, fromAirport, toAirport, airline));
        } else if (type == "Hotel") {
            std::string hotel = requireString(bookingNode, "hotel", path, lineNumber);
            std::string city = requireString(bookingNode, "city", path, lineNumber);
            addBooking(std::make_unique<HotelReservation>(id, price, fromDate, toDate, hotel, city));
        } else if (type == "RentalCar") {
            std::string pickup = requireString(bookingNode, "pickupLocation", path, lineNumber);
            std::string dropoff = requireString(bookingNode, "returnLocation", path, lineNumber);
            std::string company = requireString(bookingNode, "company", path, lineNumber);
            addBooking(std::make_unique<RentalCarReservation>(id, price, fromDate, toDate, pickup, dropoff, company));
        } else if (type == "Train") {
            std::string fromStation = requireString(bookingNode, "fromStation", path, lineNumber);
            std::string toStation = requireString(bookingNode, "toStation", path, lineNumber);
            std::string departure = requireString(bookingNode, "departureTime", path, lineNumber);
            std::string arrival = requireString(bookingNode, "arrivalTime", path, lineNumber);
            auto viaStations = bookingNode.contains("viaStations")
                                   ? requireStringArray(bookingNode, "viaStations", path, lineNumber)
                                   : std::vector<std::string>{};
            addBooking(std::make_unique<TrainTicket>(id, price, fromDate, toDate, fromStation, toStation, departure,
                                                     arrival, viaStations));
        } else {
            throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": Unknown booking type '" + type + "'.");
        }
    }
}

void TravelAgency::readBinaryFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open binary file: " + path);
    }

    clear();

    auto readFixedString = [](std::ifstream &stream, std::size_t length) {
        std::string value(length, '\0');
        stream.read(value.data(), static_cast<std::streamsize>(length));
        if (!stream) {
            throw std::runtime_error("Unexpected end of file while reading binary data.");
        }
        return trimSpaces(value);
    };

    auto readDouble = [](std::ifstream &stream) {
        double value = 0.0;
        stream.read(reinterpret_cast<char *>(&value), sizeof(double));
        if (!stream) {
            throw std::runtime_error("Unexpected end of file while reading binary data.");
        }
        return value;
    };

    auto readInt32 = [](std::ifstream &stream) {
        std::int32_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof(std::int32_t));
        if (!stream) {
            throw std::runtime_error("Unexpected end of file while reading binary data.");
        }
        return value;
    };

    while (in.peek() != EOF) {
        char type = '\0';
        in.read(&type, sizeof(char));
        if (!in) {
            break;
        }

        std::string id = readFixedString(in, 38);
        if (id.empty()) {
            throw std::runtime_error("Binary record contains empty id.");
        }
        if (existsId(id)) {
            throw std::runtime_error("Duplicate booking id '" + id + "' in binary file.");
        }

        double price = readDouble(in);
        if (!std::isfinite(price)) {
            throw std::runtime_error("Binary record contains invalid price value.");
        }

        std::string fromDate = readFixedString(in, 8);
        std::string toDate = readFixedString(in, 8);

        switch (type) {
        case 'F': {
            std::string fromAirport = readFixedString(in, 3);
            std::string toAirport = readFixedString(in, 3);
            std::string airline = readFixedString(in, 15);
            addBooking(std::make_unique<FlightBooking>(id, price, fromDate, toDate, fromAirport, toAirport, airline));
            break;
        }
        case 'H': {
            std::string hotel = readFixedString(in, 15);
            std::string city = readFixedString(in, 15);
            addBooking(std::make_unique<HotelReservation>(id, price, fromDate, toDate, hotel, city));
            break;
        }
        case 'R': {
            std::string pickup = readFixedString(in, 15);
            std::string dropoff = readFixedString(in, 15);
            std::string company = readFixedString(in, 15);
            addBooking(std::make_unique<RentalCarReservation>(id, price, fromDate, toDate, pickup, dropoff, company));
            break;
        }
        case 'T': {
            std::string fromStation = readFixedString(in, 15);
            std::string toStation = readFixedString(in, 15);
            std::string departure = readFixedString(in, 5);
            std::string arrival = readFixedString(in, 5);
            std::int32_t countVia = readInt32(in);
            if (countVia < 0) {
                throw std::runtime_error("Binary record contains negative via station count.");
            }
            std::vector<std::string> viaStations;
            viaStations.reserve(static_cast<std::size_t>(countVia));
            for (std::int32_t i = 0; i < countVia; ++i) {
                viaStations.push_back(readFixedString(in, 15));
            }
            addBooking(std::make_unique<TrainTicket>(id, price, fromDate, toDate, fromStation, toStation, departure,
                                                     arrival, viaStations));
            break;
        }
        default:
            throw std::runtime_error("Unknown record type in binary file: " + std::string(1, type));
        }
    }
}

void TravelAgency::printAllDetails() const {
    for (const auto &booking : bookings_) {
        booking->showDetails();
    }
}

void TravelAgency::printStatistics() const {
    struct Stats {
        int count = 0;
        double sum = 0.0;
    };

    Stats flights;
    Stats rentals;
    Stats hotels;
    Stats trains;

    for (const auto &booking : bookings_) {
        if (dynamic_cast<const FlightBooking *>(booking.get())) {
            ++flights.count;
            flights.sum += booking->getPrice();
        } else if (dynamic_cast<const RentalCarReservation *>(booking.get())) {
            ++rentals.count;
            rentals.sum += booking->getPrice();
        } else if (dynamic_cast<const HotelReservation *>(booking.get())) {
            ++hotels.count;
            hotels.sum += booking->getPrice();
        } else if (dynamic_cast<const TrainTicket *>(booking.get())) {
            ++trains.count;
            trains.sum += booking->getPrice();
        }
    }

    auto formatStats = [](const Stats &stats) {
        std::ostringstream oss;
        oss << stats.count << " (" << std::fixed << std::setprecision(2) << stats.sum << " Euro)";
        return oss.str();
    };

    std::cout << "Flights: " << formatStats(flights) << ", "
              << "RentalCars: " << formatStats(rentals) << ", "
              << "Hotels: " << formatStats(hotels) << ", "
              << "Trains: " << formatStats(trains) << '\n';
}

void TravelAgency::clear() {
    bookings_.clear();
}

bool TravelAgency::existsId(const std::string &id) const {
    for (const auto &booking : bookings_) {
        if (booking->getId() == id) {
            return true;
        }
    }
    return false;
}
