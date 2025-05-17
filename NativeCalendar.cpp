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

    // Находим первый день полива в текущем месяце
    while (currentDate < Date(1, currentMonth.month, currentMonth.year)) {
        currentDate.day += plant.wateringFrequency;
        NormalizeDate(currentDate);
    }

    // Собираем все дни полива в текущем месяце
    while (currentDate.month == currentMonth.month && currentDate.year == currentMonth.year) {
        dates.push_back(currentDate);
        currentDate.day += plant.wateringFrequency;
        NormalizeDate(currentDate);
    }

    return dates;
}

void PlantDatabase::NormalizeDate(Date& date) const {
    while (date.day > GetDaysInMonth(date.month, date.year)) {
        date.day -= GetDaysInMonth(date.month, date.year);
        date.month++;
        if (date.month > 12) {
            date.month = 1;
            date.year++;
        }
    }
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
    std::string fontPath = "C:\\Users\\ZEFS\\Desktop\\PlantsOnline\\arial.ttf";
    if (!font.loadFromFile(fontPath)) {
        throw std::runtime_error("Failed to load font arial.ttf");
    }

    sf::Image waterDropImage;
    waterDropImage.create(32, 32, sf::Color::Transparent);

    // Рисуем каплю с градиентом
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            float dx = (x - 16) / 16.0f;
            float dy = (y - 16) / 16.0f;
            float distance = dx * dx + dy * dy * 0.6f;

            if (distance <= 1.0f) {
                // Градиент от светло-голубого к синему
                int alpha = 200 - static_cast<int>(distance * 150);
                int blue = 255 - static_cast<int>(distance * 100);
                waterDropImage.setPixel(x, y, sf::Color(30, 144, blue, alpha));
            }
        }
    }
    waterDropTexture.loadFromImage(waterDropImage);
}


void NativeCalendar::Render() {
    if (!sfmlWindow) return;
    DrawCalendar();
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

int NativeCalendar::GetSelectedPlantIndex() const {
    return selectedPlantIndex;
}


void NativeCalendar::DrawCalendar() {
    if (!sfmlWindow) return;

    const float windowWidth = static_cast<float>(sfmlWindow->getSize().x);
    const float windowHeight = static_cast<float>(sfmlWindow->getSize().y);

    // Рассчитываем размеры панелей (используем сохраненные значения)
    const float availableWidth = windowWidth - leftPanelWidth - rightPanelWidth;
    if (availableWidth <= 0 || windowHeight <= 100.0f) return;

    // Расчет размеров ячеек с плавающей точкой
    const float cellWidth = availableWidth / 7.0f;
    const float cellHeight = (windowHeight - 100.0f) / 6.0f; // 100px верхний отступ

    // Позиционирование календаря
    const float startX = leftPanelWidth;
    const float startY = 50.0f; // Отступ сверху

    // Очищаем область календаря
    sf::RectangleShape background(sf::Vector2f(availableWidth, windowHeight));
    background.setPosition(startX, 0);
    background.setFillColor(sf::Color::White);
    sfmlWindow->draw(background);

    for (int week = 0; week < 6; ++week) {
        for (int weekday = 0; weekday < 7; ++weekday) {
            float posY = startY + 30.0f + week * cellHeight; // 30 - отступ от заголовков дней
            // ... отрисовка ячейки ...
        }
    }

    // Получаем информацию о текущем месяце
    int daysInMonth = GetDaysInMonth(displayedMonth.month, displayedMonth.year);

    // Определяем первый день месяца
    tm timeinfo = { 0 };
    timeinfo.tm_year = displayedMonth.year - 1900;
    timeinfo.tm_mon = displayedMonth.month - 1;
    timeinfo.tm_mday = 1;
    mktime(&timeinfo);
    int firstWeekDay = timeinfo.tm_wday == 0 ? 6 : timeinfo.tm_wday - 1; // Пн=0, Вс=6

    // Отрисовываем названия дней недели
    const std::string days[] = { "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс" };
    for (int i = 0; i < 7; ++i) {
        sf::Text text(days[i], font, 16);
        text.setFillColor(i >= 5 ? sf::Color::Red : sf::Color::Black);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        text.setPosition(startX + i * cellWidth + cellWidth / 2, startY);
        sfmlWindow->draw(text);
    }

    // Отрисовываем сетку календаря
    int day = 1;
    for (int week = 0; week < 6; ++week) {
        for (int weekday = 0; weekday < 7; ++weekday) {
            float posX = startX + weekday * cellWidth;
            float posY = startY + 35 + week * cellHeight; // 35 - отступ от названий дней

            // Пропускаем пустые ячейки
            if ((week == 0 && weekday < firstWeekDay) || day > daysInMonth) {
                // Отрисовываем пустую ячейку
                sf::RectangleShape cell(sf::Vector2f(cellWidth - 2, cellHeight - 2));
                cell.setPosition(posX + 1, posY + 1);
                cell.setFillColor(sf::Color(230, 230, 230)); // Серый фон для пустых ячеек
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color(200, 200, 200));
                sfmlWindow->draw(cell);
                continue;
            }

            // Рисуем ячейку с днем
            sf::RectangleShape cell(sf::Vector2f(cellWidth - 2, cellHeight - 2));
            cell.setPosition(posX + 1, posY + 1);
            cell.setFillColor(sf::Color::White);
            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color(200, 200, 200));
            sfmlWindow->draw(cell);

            // Отрисовываем число
            sf::Text dayText(std::to_string(day), font, 14);
            dayText.setFillColor(weekday >= 5 ? sf::Color::Red : sf::Color::Black);
            sf::FloatRect textRect = dayText.getLocalBounds();
            dayText.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            dayText.setPosition(posX + cellWidth / 2, posY + 20);
            sfmlWindow->draw(dayText);

            // Отрисовываем капельки для выбранного растения
            if (selectedPlantIndex >= 0 &&
                selectedPlantIndex < plantDB.getUserPlants().size()) {


                const Plant& plant = plantDB.getUserPlants()[selectedPlantIndex];
                auto dates = plantDB.getWateringDates(plant, displayedMonth);

                for (const auto& date : dates) {
                    if (date.day == day && date.month == displayedMonth.month) {
                        sf::Sprite waterDrop(waterDropTexture);
                        waterDrop.setColor(sf::Color(30, 144, 255)); // Синий цвет
                        waterDrop.setScale(
                            std::min(0.4f, cellWidth / 100.0f),
                            std::min(0.4f, cellHeight / 100.0f)
                        );
                        waterDrop.setPosition(posX + cellWidth - 25, posY + 5);
                        sfmlWindow->draw(waterDrop);
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
