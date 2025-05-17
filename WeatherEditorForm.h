#pragma once
#include "NativeCalendar.h"
#include <Windows.h>

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace CalendarApp {
    public ref class WeatherEditorForm : public System::Windows::Forms::Form {
    private:
        NativeCalendar* nativeCalendar;
        MonthCalendar^ monthCalendar;
        ComboBox^ weatherCombo;
        Button^ saveButton;

    public:
        WeatherEditorForm(NativeCalendar* calendar) : nativeCalendar(calendar) {
            InitializeComponent();
            LoadWeatherData();
        }

        void InitializeComponent() {
            this->Text = L"Редактор погоды";
            this->Size = System::Drawing::Size(450, 320);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->StartPosition = FormStartPosition::CenterParent;

            // Инициализация календаря
            monthCalendar = gcnew MonthCalendar();
            monthCalendar->Scale(1.3f);
            monthCalendar->Location = Point(40, 40);
            monthCalendar->MaxSelectionCount = 1; // Выбор только одной даты
            monthCalendar->DateChanged += gcnew DateRangeEventHandler(this, &WeatherEditorForm::OnDateChanged);
            this->Controls->Add(monthCalendar);

            // Инициализация выпадающего списка
            weatherCombo = gcnew ComboBox();
            weatherCombo->Location = Point(250, 20);
            weatherCombo->Size = System::Drawing::Size(150, 60);
            weatherCombo->DropDownStyle = ComboBoxStyle::DropDownList;
            weatherCombo->Items->AddRange(gcnew array<String^>{
                L"Неизвестно", L"Ясно", L"Облачно", L"Немного облачно",
                    L"Рассеянные облака", L"Дождь со снегом", L"Дождь", L"Гроза", L"Снег", L"Туман"
            });
            this->Controls->Add(weatherCombo);

            // Инициализация кнопки сохранения
            saveButton = gcnew Button();
            saveButton->Text = L"Сохранить";
            saveButton->Location = Point(275, 200);
            saveButton->Size = System::Drawing::Size(100, 30);
            saveButton->Click += gcnew EventHandler(this, &WeatherEditorForm::OnSaveClick);
            this->Controls->Add(saveButton);
        }

        // Загрузка данных о погоде для выбранной даты
        void LoadWeatherData() {
            DateTime selectedDate = monthCalendar->SelectionStart;
            Date currentDate{
                selectedDate.Day,
                selectedDate.Month,
                selectedDate.Year
            };

            // Используем геттер для доступа к данным
            const auto& weatherData = nativeCalendar->GetWeatherData();
            auto it = weatherData.find(currentDate);

            if (it != weatherData.end()) {
                weatherCombo->SelectedIndex = static_cast<int>(it->second);
            }
            else {
                weatherCombo->SelectedIndex = -1;
            }
        }

        // Обработчик изменения даты в календаре
        void OnDateChanged(Object^ sender, DateRangeEventArgs^ e) {
            LoadWeatherData();
        }

        // Обработчик сохранения изменений
        void OnSaveClick(Object^ sender, EventArgs^ e) {
            DateTime selectedDate = monthCalendar->SelectionStart;
            Date date{
                selectedDate.Day,
                selectedDate.Month,
                selectedDate.Year
            };

            // Используем метод SetWeatherData вместо прямого доступа
            nativeCalendar->SetWeatherData(
                date,
                static_cast<WeatherCondition>(weatherCombo->SelectedIndex)
            );

            // Устанавливаем флаг ручного изменения
            nativeCalendar->SetManualWeatherChanged(true);

            // Обновляем график полива
            nativeCalendar->UpdateWateringSchedule();
            this->Close();
        }
    };
}