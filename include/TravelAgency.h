#ifndef TRAVELAGENCY_H
#define TRAVELAGENCY_H

#include "Booking.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TravelAgency {
public:
    TravelAgency() = default;
    ~TravelAgency();

    void readFile(const std::string &path);
    void readBinaryFile(const std::string &path);
    void printAllDetails() const;
    void printStatistics() const;
    void clear();
    bool existsId(const std::string &id) const;

private:
    std::vector<std::unique_ptr<Booking>> bookings_;

    void addBooking(std::unique_ptr<Booking> booking);
};

#endif // TRAVELAGENCY_H
