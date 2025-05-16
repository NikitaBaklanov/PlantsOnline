#include "NativeCalendar.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

PlantDatabase::PlantDatabase() {
    plantsData = {
        {"Ромашка садовая", 5},
        {"Петуния", 2},
        {"Календула", 7}
    };
}

const std::map<std::string, int>& PlantDatabase::getAvailablePlants() const {
    return plantsData;
}

void PlantDatabase::addPlant(const Plant& plant) {
    userPlants.push_back(plant);
}

void PlantDatabase::removePlant(int index) {
    if (index >= 0 && index < userPlants.size()) {
        userPlants.erase(userPlants.begin() + index);
    }
}

const std::vector<Plant>& PlantDatabase::getUserPlants() const {
    return userPlants;
}

int GetDaysInMonth(int month, int year) {
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    if (month == 2) return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    return 31;
}

std::vector<Date> PlantDatabase::getWateringDates(const Plant& plant, const Date& currentMonth) const {
    std::vector<Date> dates;
    Date currentDate = plant.plantingDate;
    Date today = Date(1, 1, 2023); // Упрощено для примера

    while (currentDate <= today) {
        if (currentDate.month == currentMonth.month && currentDate.year == currentMonth.year) {
            dates.push_back(currentDate);
        }
        currentDate.day += plant.wateringFrequency;

        int maxDays = GetDaysInMonth(currentDate.month, currentDate.year);
        while (currentDate.day > maxDays) {
            currentDate.day -= maxDays;
            currentDate.month++;
            if (currentDate.month > 12) {
                currentDate.month = 1;
                currentDate.year++;
            }
            maxDays = GetDaysInMonth(currentDate.month, currentDate.year);
        }
    }
    return dates;
}

NativeCalendar::NativeCalendar()
    : sfmlWindow(nullptr), currentDate{ 1, 1, 2000 }, displayedMonth{ 1, 1, 2000 } {
}

NativeCalendar::~NativeCalendar() {
    if (sfmlWindow) {
        sfmlWindow->close();
        delete sfmlWindow;
    }
}

void NativeCalendar::Initialize(void* windowHandle) {
    time_t t = time(nullptr);
    tm now;
    localtime_s(&now, &t);
    currentDate = { now.tm_mday, now.tm_mon + 1, now.tm_year + 1900 };
    displayedMonth = { 1, currentDate.month, currentDate.year };

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sfmlWindow = new sf::RenderWindow();
    sfmlWindow->create(sf::WindowHandle(static_cast<HWND>(windowHandle)), settings);
    std::string fontPath = "arial.ttf";
    if (!font.loadFromFile(fontPath)) {
        throw std::runtime_error("Failed to load font arial.ttf");
    }
}

void NativeCalendar::Render() {
    if (!sfmlWindow) return;
    DrawCalendar();
}

void NativeCalendar::Resize(unsigned width, unsigned height) {
    if (sfmlWindow) {
        sfmlWindow->setSize(sf::Vector2u(width, height));
        sfmlWindow->setView(sf::View(sf::FloatRect(0, 0, width, height)));
    }
}

void NativeCalendar::NextMonth() {
    displayedMonth.month++;
    if (displayedMonth.month > 12) {
        displayedMonth.month = 1;
        displayedMonth.year++;
    }
}

void NativeCalendar::PrevMonth() {
    displayedMonth.month--;
    if (displayedMonth.month < 1) {
        displayedMonth.month = 12;
        displayedMonth.year--;
    }
}

void NativeCalendar::DrawCalendar() {
    sfmlWindow->clear(sf::Color::White);

    const int leftPanelWidth = 200;
    const int rightPanelWidth = 200;
    const int windowWidth = sfmlWindow->getSize().x;
    const int cellWidth = (windowWidth - leftPanelWidth - rightPanelWidth) / 7;
    const int startX = leftPanelWidth + (windowWidth - leftPanelWidth - rightPanelWidth - 7 * cellWidth) / 2;

    int daysInMonth = GetDaysInMonth(displayedMonth.month, displayedMonth.year);

    tm timeinfo = { 0 };
    timeinfo.tm_year = displayedMonth.year - 1900;
    timeinfo.tm_mon = displayedMonth.month - 1;
    timeinfo.tm_mday = 1;
    mktime(&timeinfo);
    int firstWeekDay = timeinfo.tm_wday == 0 ? 6 : timeinfo.tm_wday - 1;

    sf::Text monthText(GetMonthName(displayedMonth.month) + " " + std::to_string(displayedMonth.year), font, 24);
    monthText.setPosition(startX + 100, 10);
    sfmlWindow->draw(monthText);

    const std::string days[] = { "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс" };
    for (int i = 0; i < 7; ++i) {
        sf::Text text(days[i], font, 18);
        text.setPosition(startX + i * cellWidth + (cellWidth - 20) / 2, 50);
        text.setFillColor(i >= 5 ? sf::Color::Red : sf::Color::Black);
        sfmlWindow->draw(text);
    }

    int day = 1;
    for (int week = 0; week < 6; ++week) {
        for (int weekday = 0; weekday < 7; ++weekday) {
            if (week == 0 && weekday < firstWeekDay) continue;
            if (day > daysInMonth) continue;

            float posX = startX + weekday * cellWidth;
            float posY = 80 + week * 60;

            sf::Text dayText(std::to_string(day), font, 16);
            dayText.setPosition(posX + 5, posY + 5);
            dayText.setFillColor(weekday >= 5 ? sf::Color::Red : sf::Color::Black);
            sfmlWindow->draw(dayText);

            if (plantDB.selectedIndex != -1 && plantDB.selectedIndex < plantDB.getUserPlants().size()) {
                auto plant = plantDB.getUserPlants()[plantDB.selectedIndex];
                auto dates = plantDB.getWateringDates(plant, displayedMonth);
                for (const auto& date : dates) {
                    if (date.day == day && date.month == displayedMonth.month && date.year == displayedMonth.year) {
                        sf::CircleShape marker(4);
                        marker.setFillColor(sf::Color::Blue);
                        marker.setPosition(posX + cellWidth - 15, posY + 40);
                        sfmlWindow->draw(marker);
                    }
                }
            }

            day++;
        }
    }

    sfmlWindow->display();
}

std::string NativeCalendar::GetMonthName(int month) const {
    const std::string months[] = {
        "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
        "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"
    };
    return months[month - 1];
}