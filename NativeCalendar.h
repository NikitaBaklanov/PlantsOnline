#pragma once
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <string>
#include <ctime>
#include <vector>
#include <map>

struct Date {
    int day;
    int month;
    int year;

    Date() : day(0), month(0), year(0) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }

    bool operator<=(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day <= other.day;
    }

    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }
};

struct Plant {
    std::string name;
    int wateringFrequency;
    Date plantingDate;
};

class PlantDatabase {
public:
    int selectedIndex = -1;
    std::map<std::string, int> plantsData;
    std::vector<Plant> userPlants;

    PlantDatabase();
    const std::map<std::string, int>& getAvailablePlants() const;
    const std::vector<Plant>& getUserPlants() const;
    void addPlant(const Plant& plant);
    void removePlant(int index);
    std::vector<Date> getWateringDates(const Plant& plant, const Date& currentMonth) const;
};

int GetDaysInMonth(int month, int year);

class NativeCalendar {
public:
    NativeCalendar();
    ~NativeCalendar();

    void Initialize(void* windowHandle);
    void Render();
    void Resize(unsigned width, unsigned height);
    void NextMonth();
    void PrevMonth();

    PlantDatabase plantDB;
    Date currentDate;
    Date displayedMonth;
    //int GetDaysInMonth(int month, int year) const;
private:
    void DrawCalendar();
    std::string GetMonthName(int month) const;

    sf::RenderWindow* sfmlWindow;
    sf::Font font;
};