#include "Booking.h"

#include <algorithm>
#include <cctype>

Booking::Booking(std::string id, double price, std::string fromDate, std::string toDate)
    : id_(std::move(id)), price_(price), fromDate_(std::move(fromDate)), toDate_(std::move(toDate)) {}

Booking::~Booking() = default;

FlightBooking::FlightBooking(std::string id, double price, std::string fromDate, std::string toDate,
                             std::string fromAirport, std::string toAirport, std::string airline)
    : Booking(std::move(id), price, std::move(fromDate), std::move(toDate)),
      fromAirport_(std::move(fromAirport)), toAirport_(std::move(toAirport)), airline_(std::move(airline)) {}

void FlightBooking::showDetails() const {
    std::cout << "Flight " << id_ << ": " << formatDate(fromDate_) << " - " << formatDate(toDate_)
              << ", " << fromAirport_ << " -> " << toAirport_ << ", Airline: " << airline_
              << ", Price: " << formatPrice(price_) << '\n';
}

HotelReservation::HotelReservation(std::string id, double price, std::string fromDate, std::string toDate,
                                   std::string hotel, std::string city)
    : Booking(std::move(id), price, std::move(fromDate), std::move(toDate)), hotel_(std::move(hotel)),
      city_(std::move(city)) {}

void HotelReservation::showDetails() const {
    std::cout << "Hotel " << id_ << ": " << formatDate(fromDate_) << " - " << formatDate(toDate_)
              << ", " << hotel_ << " in " << city_ << ", Price: " << formatPrice(price_) << '\n';
}

RentalCarReservation::RentalCarReservation(std::string id, double price, std::string fromDate, std::string toDate,
                                           std::string pickupLocation, std::string returnLocation,
                                           std::string company)
    : Booking(std::move(id), price, std::move(fromDate), std::move(toDate)),
      pickupLocation_(std::move(pickupLocation)), returnLocation_(std::move(returnLocation)),
      company_(std::move(company)) {}

void RentalCarReservation::showDetails() const {
    std::cout << "RentalCar " << id_ << ": " << formatDate(fromDate_) << " - " << formatDate(toDate_)
              << ", Pickup: " << pickupLocation_ << ", Return: " << returnLocation_ << ", Company: "
              << company_ << ", Price: " << formatPrice(price_) << '\n';
}

TrainTicket::TrainTicket(std::string id, double price, std::string fromDate, std::string toDate,
                         std::string fromStation, std::string toStation, std::string departureTime,
                         std::string arrivalTime, std::vector<std::string> viaStations)
    : Booking(std::move(id), price, std::move(fromDate), std::move(toDate)),
      fromStation_(std::move(fromStation)), toStation_(std::move(toStation)),
      departureTime_(std::move(departureTime)), arrivalTime_(std::move(arrivalTime)),
      viaStations_(std::move(viaStations)) {}

void TrainTicket::showDetails() const {
    std::cout << "Train " << id_ << ": " << formatDate(fromDate_) << " - " << formatDate(toDate_)
              << ", " << fromStation_ << " -> " << toStation_ << " (" << departureTime_ << " - "
              << arrivalTime_ << ")";
    if (!viaStations_.empty()) {
        std::cout << " über " << joinStrings(viaStations_, ", ");
    }
    std::cout << ", Price: " << formatPrice(price_) << '\n';
}

std::string formatDate(const std::string &isoDate) {
    if (isoDate.size() != 8) {
        return isoDate;
    }
    return isoDate.substr(6, 2) + "." + isoDate.substr(4, 2) + "." + isoDate.substr(0, 4);
}

std::string formatPrice(double price) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << price << " Euro";
    return oss.str();
}

std::string joinStrings(const std::vector<std::string> &values, const std::string &separator) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << separator;
        }
        oss << values[i];
    }
    return oss.str();
}

std::string trimSpaces(const std::string &value) {
    auto begin = value.find_first_not_of(' ');
    if (begin == std::string::npos) {
        return "";
    }
    auto end = value.find_last_not_of(' ');
    return value.substr(begin, end - begin + 1);
}

bool isAirportCode(const std::string &code) {
    if (code.size() != 3) {
        return false;
    }
    return std::all_of(code.begin(), code.end(), [](unsigned char ch) { return std::isalpha(ch); });
}
