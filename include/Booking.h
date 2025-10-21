#ifndef BOOKING_H
#define BOOKING_H

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

std::string formatDate(const std::string &isoDate);
std::string formatPrice(double price);
std::string joinStrings(const std::vector<std::string> &values, const std::string &separator);
std::string trimSpaces(const std::string &value);
bool isAirportCode(const std::string &code);

class Booking {
public:
    Booking(std::string id, double price, std::string fromDate, std::string toDate);
    virtual ~Booking();

    const std::string &getId() const { return id_; }
    double getPrice() const { return price_; }

    virtual void showDetails() const = 0;

protected:
    std::string id_;
    double price_;
    std::string fromDate_;
    std::string toDate_;
};

class FlightBooking : public Booking {
public:
    FlightBooking(std::string id, double price, std::string fromDate, std::string toDate,
                  std::string fromAirport, std::string toAirport, std::string airline);

    void showDetails() const override;

private:
    std::string fromAirport_;
    std::string toAirport_;
    std::string airline_;
};

class HotelReservation : public Booking {
public:
    HotelReservation(std::string id, double price, std::string fromDate, std::string toDate,
                     std::string hotel, std::string city);

    void showDetails() const override;

private:
    std::string hotel_;
    std::string city_;
};

class RentalCarReservation : public Booking {
public:
    RentalCarReservation(std::string id, double price, std::string fromDate, std::string toDate,
                         std::string pickupLocation, std::string returnLocation, std::string company);

    void showDetails() const override;

private:
    std::string pickupLocation_;
    std::string returnLocation_;
    std::string company_;
};

class TrainTicket : public Booking {
public:
    TrainTicket(std::string id, double price, std::string fromDate, std::string toDate,
                std::string fromStation, std::string toStation,
                std::string departureTime, std::string arrivalTime,
                std::vector<std::string> viaStations);

    void showDetails() const override;

private:
    std::string fromStation_;
    std::string toStation_;
    std::string departureTime_;
    std::string arrivalTime_;
    std::vector<std::string> viaStations_;
};

#endif // BOOKING_H
