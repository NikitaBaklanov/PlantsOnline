#include "NativeCalendar.h"
#include "SettingsForm.h"
#include "WeatherEditorForm.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <iostream>

void NativeCalendar::UpdateWateringSchedule() {
    isCacheValid = false;
    UpdateWateringCache();
    Render();
}

void NativeCalendar::UpdateWateringCache() {
    wateringScheduleCache.clear();
    const auto& plants = plantDB.getUserPlants();
    for (size_t i = 0; i < plants.size(); ++i) {
        wateringScheduleCache[i] = plantDB.getAdaptiveWateringDates(
            plants[i],
            displayedMonth,
            weatherData,
            useWeatherForWatering
        );
    }
    cachedMonth = displayedMonth;
    isCacheValid = true;
}

PlantDatabase::PlantDatabase() {
    plantsData = {
        {"Ромашка садовая", 5},
        {"Петуния", 2},
        {"Календула", 7},
        {"Костянчик", 4}
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

void PlantDatabase::updateLastWateredDate(int plantIndex, const Date& date) {
    if (plantIndex >= 0 && plantIndex < userPlants.size()) {
        userPlants[plantIndex].lastWateredDate = date;
    }
}

const std::vector<Plant>& PlantDatabase::getUserPlants() const {
    return userPlants;
}

std::vector<Date> PlantDatabase::getWateringDates(const Plant& plant, const Date& currentMonth) const {
    std::vector<Date> dates;
    Date currentDate = plant.plantingDate;

    while (currentDate < Date(1, currentMonth.month, currentMonth.year)) {
        currentDate.day += plant.wateringFrequency;
        NormalizeDate(currentDate);
    }

    while (currentDate.month == currentMonth.month && currentDate.year == currentMonth.year) {
        dates.push_back(currentDate);
        currentDate.day += plant.wateringFrequency;
        NormalizeDate(currentDate);
    }

    return dates;
}

std::vector<Date> PlantDatabase::getAdaptiveWateringDates(
    const Plant& plant,
    const Date& currentMonth,
    const std::map<Date, WeatherCondition>& weatherData,
    bool useWeather
) const {
    std::vector<Date> dates;

    // Если влияние погоды отключено, возвращаем базовый график
    if (!useWeather) {
        return getWateringDates(plant, currentMonth);
    }

    // Начальная дата для расчёта полива
    Date currentDate = (plant.lastWateredDate.day == 0)
        ? plant.plantingDate
        : plant.lastWateredDate;

    // Продвигаемся до начала текущего месяца
    while (currentDate < Date(1, currentMonth.month, currentMonth.year)) {
        currentDate.day += plant.wateringFrequency;
        NormalizeDate(currentDate);
    }

    // Основной цикл расчёта полива для текущего месяца
    while (currentDate.month == currentMonth.month &&
        currentDate.year == currentMonth.year) {
        bool skipDueToRain = false;

        // Проверяем погоду на день полива
        auto weatherIt = weatherData.find(currentDate);
        if (weatherIt != weatherData.end()) {
            // Пропускаем полив, если дождь или ливень
            if (weatherIt->second == WeatherCondition::RAIN ||
                weatherIt->second == WeatherCondition::SHOWER_RAIN) {
                skipDueToRain = true;
            }
        }

        if (!skipDueToRain) {
            dates.push_back(currentDate);
        }

        // Рассчитываем следующую дату полива
        Date nextDate = currentDate;
        nextDate.day += plant.wateringFrequency;
        NormalizeDate(nextDate);

        // Проверяем погоду в промежуточные дни
        Date checkDate = currentDate;
        while (checkDate < nextDate) {
            checkDate.day++;
            NormalizeDate(checkDate);

            // Если найден дождливый день, переносим полив на него
            weatherIt = weatherData.find(checkDate);
            if (weatherIt != weatherData.end() &&
                (weatherIt->second == WeatherCondition::RAIN ||
                    weatherIt->second == WeatherCondition::SHOWER_RAIN)) {
                nextDate = checkDate;
                break;
            }
        }

        currentDate = nextDate;
    }

    return dates;
}

void PlantDatabase::NormalizeDate(Date& date) const {
    // Проверяем валидность месяца
    if (date.month < 1) {
        date.year -= (-date.month / 12) + 1;
        date.month = 12 - (-date.month % 12);
    }
    else if (date.month > 12) {
        date.year += (date.month - 1) / 12;
        date.month = (date.month - 1) % 12 + 1;
    }

    // Проверяем валидность дня
    int daysInMonth = GetDaysInMonth(date.month, date.year);
    if (date.day < 1) {
        date.month--;
        if (date.month < 1) {
            date.month = 12;
            date.year--;
        }
        date.day += GetDaysInMonth(date.month, date.year);
    }
    else if (date.day > daysInMonth) {
        date.day -= daysInMonth;
        date.month++;
        if (date.month > 12) {
            date.month = 1;
            date.year++;
        }
    }
}

int GetDaysInMonth(int month, int year) {
    static const int daysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month == 2) {
        // Проверка високосного года
        bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return isLeap ? 29 : 28;
    }

    if (month >= 1 && month <= 12) {
        return daysPerMonth[month - 1];
    }

    return 31; // Возвращаем значение по умолчанию при невалидном месяце
}


NativeCalendar::NativeCalendar()
    : sfmlWindow(nullptr), currentDate{ 1, 1, 2000 }, displayedMonth{ 1, 1, 2000 } {
    LoadWeatherTextures();
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

    if (!font.loadFromFile("arial.ttf")) {
        throw std::runtime_error("Failed to load font arial.ttf");
    }

    if (!waterDropTexture.loadFromFile("water_drop.png")) {
        throw std::runtime_error("Failed to load water drop texture");
    }

    FetchWeatherData();
}

WeatherCondition NativeCalendar::ParseWeatherIcon(const std::string& icon) const {
    if (icon == "01d") return WeatherCondition::CLEAR;
    if (icon == "02d") return WeatherCondition::FEW_CLOUDS;
    if (icon == "03d") return WeatherCondition::SCATTERED_CLOUDS;
    if (icon == "04d") return WeatherCondition::BROKEN_CLOUDS;
    if (icon == "09d") return WeatherCondition::SHOWER_RAIN;
    if (icon == "10d") return WeatherCondition::RAIN;
    if (icon == "11d") return WeatherCondition::THUNDERSTORM;
    if (icon == "13d") return WeatherCondition::SNOW;
    if (icon == "50d") return WeatherCondition::MIST;
    return WeatherCondition::UNKNOWN;
}

void NativeCalendar::FetchWeatherData() {
    static time_t lastFetchTime = 0;
    time_t currentTime = time(nullptr);
    if (currentTime - lastFetchTime < 3600) return;
    lastFetchTime = currentTime;

    CURL* curl = curl_easy_init();
    if (!curl) {
        lastWeatherError = "Failed to initialize CURL";
        return;
    }

    std::string readBuffer;
    const std::string url = "https://api.openweathermap.org/data/2.5/forecast?lat=58.010455&lon=56.229443&appid=b85de43185edf09d3a029ad98676bffb&units=metric&lang=ru";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &NativeCalendar::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        lastWeatherError = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return;
    }

    long https_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &https_code);

    if (https_code != 200) {
        lastWeatherError = "HTTP error: " + std::to_string(https_code);
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);

    try {
        weatherData.clear();
        auto jsonData = json::parse(readBuffer);

        
        int code = jsonData["cod"].is_number() ? jsonData["cod"].get<int>() : std::stoi(jsonData["cod"].get<std::string>());
        if (code != 200) {
            lastWeatherError = jsonData.contains("message") ? jsonData["message"].get<std::string>() : "API error";
            return;
        }

        std::map<Date, std::string> dailyForecast;
        for (const auto& item : jsonData["list"]) {
            std::tm tm = {};
            std::istringstream ss(item["dt_txt"].get<std::string>());
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

            Date date{ tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900 };
            if (dailyForecast.find(date) == dailyForecast.end()) {
                dailyForecast[date] = item["weather"][0]["icon"].get<std::string>();
            }
        }

        for (const auto& [date, icon] : dailyForecast) {
            weatherData[date] = ParseWeatherIcon(icon);
        }
        lastWeatherError.clear();
    }
    catch (const std::exception& e) {
        lastWeatherError = "JSON error: " + std::string(e.what());
    }

}

size_t NativeCalendar::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void NativeCalendar::LoadWeatherTextures() {
    const std::vector<std::pair<std::string, WeatherCondition>> textureFiles = {
        {"weather_unknown.png", WeatherCondition::UNKNOWN},
        {"weather_clear.png", WeatherCondition::CLEAR},
        {"weather_clouds.png", WeatherCondition::FEW_CLOUDS},
        {"weather_clouds.png", WeatherCondition::SCATTERED_CLOUDS},
        {"weather_clouds.png", WeatherCondition::BROKEN_CLOUDS},
        {"weather_shower_rain.png", WeatherCondition::SHOWER_RAIN},
        {"weather_rain.png", WeatherCondition::RAIN},
        {"weather_thunderstorm.png", WeatherCondition::THUNDERSTORM},
        {"weather_snow.png", WeatherCondition::SNOW}, 
        {"weather_mist.png", WeatherCondition::MIST},
    };

    for (const auto& pair : textureFiles) {
        if (!weatherTextures[static_cast<int>(pair.second)].loadFromFile(pair.first)) {
            throw std::runtime_error("Failed to load texture: " + pair.first);
        }
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
        Render();
    }
}

void NativeCalendar::SetPanelWidths(int left, int right) {
    leftPanelWidth = static_cast<float>(left);
    rightPanelWidth = static_cast<float>(right);
}

void NativeCalendar::NextMonth() {
    displayedMonth.month++;
    if (displayedMonth.month > 12) {
        displayedMonth.month = 1;
        displayedMonth.year++;
    }
    FetchWeatherData();
}

void NativeCalendar::PrevMonth() {
    displayedMonth.month--;
    if (displayedMonth.month < 1) {
        displayedMonth.month = 12;
        displayedMonth.year--;
    }
    FetchWeatherData();
}

int NativeCalendar::GetSelectedPlantIndex() const {
    return selectedPlantIndex;
}

std::string NativeCalendar::GetMonthName(int month) const {
    const std::string months[] = {
        "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
        "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"
    };
    return months[month - 1];
}

void NativeCalendar::SetSelectedPlantIndex(int index) {
    if (index >= -1 && index < plantDB.getUserPlants().size()) {
        selectedPlantIndex = index;
    }
}

std::string NativeCalendar::GetLastWeatherError() const {
    return lastWeatherError;
}

bool NativeCalendar::IsWeatherDataAvailable() const {
    return !weatherData.empty();
}

void NativeCalendar::MarkAsWateredToday() {
    if (selectedPlantIndex >= 0 && selectedPlantIndex < plantDB.getUserPlants().size()) {
        plantDB.updateLastWateredDate(selectedPlantIndex, currentDate);
    }
}

void NativeCalendar::DrawCalendar() {
    if (!sfmlWindow) return;

    const float windowWidth = static_cast<float>(sfmlWindow->getSize().x);
    const float windowHeight = static_cast<float>(sfmlWindow->getSize().y);
    const float availableWidth = windowWidth - leftPanelWidth - rightPanelWidth;

    if (availableWidth <= 0 || windowHeight <= 100.0f) return;

    const float cellWidth = availableWidth / 7.0f;
    const float cellHeight = (windowHeight - 100.0f) / 6.0f;
    const float startX = leftPanelWidth;
    const float startY = 50.0f;

    // Clear background
    sf::RectangleShape background(sf::Vector2f(availableWidth, windowHeight));
    background.setPosition(startX, 0);
    background.setFillColor(sf::Color::White);
    sfmlWindow->draw(background);

    // Draw weekday headers
    const std::string days[] = { "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс" };
    for (int i = 0; i < 7; ++i) {
        sf::Text text(days[i], font, 16);
        text.setFillColor(i >= 5 ? sf::Color::Red : sf::Color::Black);
        text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
        text.setPosition(startX + i * cellWidth + cellWidth / 2, startY);
        sfmlWindow->draw(text);
    }

    // Calendar grid and cells
    int daysInMonth = GetDaysInMonth(displayedMonth.month, displayedMonth.year);
    tm timeinfo = { 0 };
    timeinfo.tm_year = displayedMonth.year - 1900;
    timeinfo.tm_mon = displayedMonth.month - 1;
    timeinfo.tm_mday = 1;
    mktime(&timeinfo);
    int firstWeekDay = timeinfo.tm_wday == 0 ? 6 : timeinfo.tm_wday - 1;

    int day = 1;
    for (int week = 0; week < 6; ++week) {
        for (int weekday = 0; weekday < 7; ++weekday) {
            float posX = startX + weekday * cellWidth;
            float posY = startY + 35 + week * cellHeight;

            // Draw cell background
            sf::RectangleShape cell(sf::Vector2f(cellWidth - 2, cellHeight - 2));
            cell.setPosition(posX + 1, posY + 1);

            if ((week == 0 && weekday < firstWeekDay) || day > daysInMonth) {
                cell.setFillColor(sf::Color(230, 230, 230));
            }
            else {
                cell.setFillColor(sf::Color::White);
            }

            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color(200, 200, 200));
            sfmlWindow->draw(cell);

            // Draw day number and icons
            if (day <= daysInMonth && !(week == 0 && weekday < firstWeekDay)) {
                // Day number - с новым выделением текущего дня
                sf::Text dayText(std::to_string(day), font, 14);

                // Выделение текущего дня
                if (day == currentDate.day &&
                    displayedMonth.month == currentDate.month &&
                    displayedMonth.year == currentDate.year) {
                    dayText.setFillColor(sf::Color::Blue);
                    dayText.setStyle(sf::Text::Bold);
                    dayText.setCharacterSize(16);
                }
                else {
                    dayText.setFillColor(weekday >= 5 ? sf::Color::Red : sf::Color::Black);
                }

                dayText.setPosition(posX + 5, posY + 5);
                sfmlWindow->draw(dayText);

                // Отрисовка иконки погоды
                Date currentCellDate(day, displayedMonth.month, displayedMonth.year);
                auto weatherIt = weatherData.find(currentCellDate);
                if (weatherIt != weatherData.end()) {
                    sf::Sprite weatherSprite(weatherTextures[static_cast<int>(weatherIt->second)]);
                    weatherSprite.setPosition(posX + 5, posY + cellHeight - 40);
                    sfmlWindow->draw(weatherSprite);
                }

                // Water drop - если растение выбрано
                if (selectedPlantIndex >= 0 && selectedPlantIndex < plantDB.getUserPlants().size()) {
                    const Plant& plant = plantDB.getUserPlants()[selectedPlantIndex];
                    auto dates = plantDB.getAdaptiveWateringDates(
                        plant,
                        displayedMonth,
                        weatherData,
                        useWeatherForWatering // Передача значения из NativeCalendar
                    );

                    for (const auto& date : dates) {
                        if (date.day == day && date.month == displayedMonth.month) {
                            sf::Sprite waterDrop(waterDropTexture);
                            waterDrop.setScale(0.9f, 0.9f);
                            waterDrop.setPosition(posX + cellWidth - 30, posY + 5);
                            sfmlWindow->draw(waterDrop);
                            break;
                        }
                    }
                }

                day++;
            }
        }
    }

    if (!lastWeatherError.empty()) {
        sf::Text errorText("Weather: " + lastWeatherError, font, 12);
        errorText.setFillColor(sf::Color::Red);
        errorText.setPosition(startX + 10, startY + 10);
        sfmlWindow->draw(errorText);
    }

    sfmlWindow->display();
}