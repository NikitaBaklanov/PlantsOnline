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

    void NormalizeDate(Date& date) const;

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
    void Resize(unsigned width, unsigned height) {
        if (sfmlWindow) {
            sfmlWindow->setSize(sf::Vector2u(width, height));
            sfmlWindow->setView(sf::View(sf::FloatRect(0, 0, width, height)));
            Render(); // ƒобавл€ем перерисовку после изменени€ размера
        }
    }

    void SetPanelWidths(int left, int right) {
        leftPanelWidth = static_cast<float>(left);
        rightPanelWidth = static_cast<float>(right);
    }

    void NextMonth();
    void PrevMonth();

    PlantDatabase plantDB;
    Date currentDate;
    Date displayedMonth;

    int GetSelectedPlantIndex() const;
    std::string GetMonthName(int month) const;

    void SetSelectedPlantIndex(int index) {
        if (index >= -1 && index < plantDB.getUserPlants().size()) {
            selectedPlantIndex = index;
        }
    }
private:
    void DrawCalendar();

    int selectedPlantIndex = -1;

    float leftPanelWidth = 250.0f;  // ƒобавл€ем пол€ дл€ хранени€ ширины
    float rightPanelWidth = 250.0f;

    sf::Texture waterDropTexture;
    sf::RenderWindow* sfmlWindow;
    sf::Font font;



};