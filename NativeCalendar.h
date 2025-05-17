#pragma once
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <string>
#include <ctime>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

struct Date {
    int day;
    int month;
    int year;

    Date() : day(0), month(0), year(0) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
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
    Date lastWateredDate;
};

enum class WeatherCondition {
    UNKNOWN,
    CLEAR,
    FEW_CLOUDS,
    SCATTERED_CLOUDS,
    BROKEN_CLOUDS,
    SHOWER_RAIN,
    RAIN,
    THUNDERSTORM,
    SNOW,
    MIST,
};

class PlantDatabase {
public:
    std::map<std::string, int> plantsData;
    std::vector<Plant> userPlants;

    PlantDatabase();
    const std::map<std::string, int>& getAvailablePlants() const;
    const std::vector<Plant>& getUserPlants() const;
    void addPlant(const Plant& plant);
    void removePlant(int index);
    void updateLastWateredDate(int plantIndex, const Date& date);
    std::vector<Date> getWateringDates(const Plant& plant, const Date& currentMonth) const;
    std::vector<Date> getAdaptiveWateringDates(
        const Plant& plant,
        const Date& currentMonth,
        const std::map<Date, WeatherCondition>& weatherData,
        bool useWeather // Добавленный параметр
    ) const;
    void NormalizeDate(Date& date) const;
};

class NativeCalendar {
public:
    NativeCalendar();
    ~NativeCalendar();

    void Initialize(void* windowHandle);
    void Render();
    void Resize(unsigned width, unsigned height);
    void SetPanelWidths(int left, int right);

    void NextMonth();
    void PrevMonth();

    int GetSelectedPlantIndex() const;
    std::string GetMonthName(int month) const;
    void SetSelectedPlantIndex(int index);
    void MarkAsWateredToday();

    std::string GetLastWeatherError() const;
    bool IsWeatherDataAvailable() const;

    PlantDatabase plantDB;
    Date currentDate;
    Date displayedMonth;

    void SetUseWeatherForWatering(bool value) { useWeatherForWatering = value; }
    bool IsWeatherAffectingWatering() const { return useWeatherForWatering; }
    void UpdateWateringSchedule();
    bool useWeatherForWatering = true;

    void SetManualWeatherChanged(bool changed) { hasManualWeatherChanges = changed; }
    void SetWeatherData(const Date& date, WeatherCondition condition) {
        weatherData[date] = condition;
    }
    const std::map<Date, WeatherCondition>& GetWeatherData() const { return weatherData; }

private:
    std::unordered_map<int, std::vector<Date>> wateringScheduleCache;
    bool hasManualWeatherChanges = false;
    Date cachedMonth;
    bool isCacheValid = false;
    void UpdateWateringCache();
    void DrawCalendar();
    void FetchWeatherData();
    void LoadWeatherTextures();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
    WeatherCondition ParseWeatherIcon(const std::string& icon) const;

    int selectedPlantIndex = -1;
    float leftPanelWidth = 250.0f;
    float rightPanelWidth = 250.0f;

    std::map<Date, WeatherCondition> weatherData;
    sf::Texture weatherTextures[10];
    sf::Texture waterDropTexture;
    sf::RenderWindow* sfmlWindow;
    sf::Font font;
    std::string lastWeatherError;
};

int GetDaysInMonth(int month, int year);