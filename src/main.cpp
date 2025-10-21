#include "TravelAgency.h"

#include <iostream>
#include <limits>
#include <string>

int main() {
    TravelAgency agency;

    while (true) {
        std::cout << "Bitte wählen Sie das Eingabeformat:\n";
        std::cout << "1) JSON\n2) Binär\n0) Beenden\n> ";
        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cerr << "Ungültige Eingabe. Programm beendet.\n";
            return 1;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            std::cout << "Auf Wiedersehen!\n";
            return 0;
        }

        std::string path;
        if (choice == 1) {
            while (true) {
                std::cout << "Pfad zur JSON-Datei (Standard: bookings.json): ";
                std::getline(std::cin, path);
                if (path.empty()) {
                    path = "bookings.json";
                }
                try {
                    agency.readFile(path);
                    break;
                } catch (const std::exception &ex) {
                    std::cerr << ex.what() << "\n";
                    std::cout << "Bitte korrigieren Sie die Datei und versuchen Sie es erneut.\n";
                }
            }
            break;
        } else if (choice == 2) {
            std::cout << "Pfad zur Binärdatei (Standard: bookingsBinary.bin): ";
            std::getline(std::cin, path);
            if (path.empty()) {
                path = "bookingsBinary.bin";
            }
            try {
                agency.readBinaryFile(path);
                break;
            } catch (const std::exception &ex) {
                std::cerr << ex.what() << "\n";
                return 1;
            }
        } else {
            std::cout << "Bitte wählen Sie 0, 1 oder 2.\n";
        }
    }

    agency.printAllDetails();
    agency.printStatistics();

    return 0;
}
