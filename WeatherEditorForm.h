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
            this->Text = L"�������� ������";
            this->Size = System::Drawing::Size(450, 320);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->StartPosition = FormStartPosition::CenterParent;

            // ������������� ���������
            monthCalendar = gcnew MonthCalendar();
            monthCalendar->Scale(1.3f);
            monthCalendar->Location = Point(40, 40);
            monthCalendar->MaxSelectionCount = 1; // ����� ������ ����� ����
            monthCalendar->DateChanged += gcnew DateRangeEventHandler(this, &WeatherEditorForm::OnDateChanged);
            this->Controls->Add(monthCalendar);

            // ������������� ����������� ������
            weatherCombo = gcnew ComboBox();
            weatherCombo->Location = Point(250, 20);
            weatherCombo->Size = System::Drawing::Size(150, 60);
            weatherCombo->DropDownStyle = ComboBoxStyle::DropDownList;
            weatherCombo->Items->AddRange(gcnew array<String^>{
                L"����������", L"����", L"�������", L"������� �������",
                    L"���������� ������", L"����� �� ������", L"�����", L"�����", L"����", L"�����"
            });
            this->Controls->Add(weatherCombo);

            // ������������� ������ ����������
            saveButton = gcnew Button();
            saveButton->Text = L"���������";
            saveButton->Location = Point(275, 200);
            saveButton->Size = System::Drawing::Size(100, 30);
            saveButton->Click += gcnew EventHandler(this, &WeatherEditorForm::OnSaveClick);
            this->Controls->Add(saveButton);
        }

        // �������� ������ � ������ ��� ��������� ����
        void LoadWeatherData() {
            DateTime selectedDate = monthCalendar->SelectionStart;
            Date currentDate{
                selectedDate.Day,
                selectedDate.Month,
                selectedDate.Year
            };

            // ���������� ������ ��� ������� � ������
            const auto& weatherData = nativeCalendar->GetWeatherData();
            auto it = weatherData.find(currentDate);

            if (it != weatherData.end()) {
                weatherCombo->SelectedIndex = static_cast<int>(it->second);
            }
            else {
                weatherCombo->SelectedIndex = -1;
            }
        }

        // ���������� ��������� ���� � ���������
        void OnDateChanged(Object^ sender, DateRangeEventArgs^ e) {
            LoadWeatherData();
        }

        // ���������� ���������� ���������
        void OnSaveClick(Object^ sender, EventArgs^ e) {
            DateTime selectedDate = monthCalendar->SelectionStart;
            Date date{
                selectedDate.Day,
                selectedDate.Month,
                selectedDate.Year
            };

            // ���������� ����� SetWeatherData ������ ������� �������
            nativeCalendar->SetWeatherData(
                date,
                static_cast<WeatherCondition>(weatherCombo->SelectedIndex)
            );

            // ������������� ���� ������� ���������
            nativeCalendar->SetManualWeatherChanged(true);

            // ��������� ������ ������
            nativeCalendar->UpdateWateringSchedule();
            this->Close();
        }
    };
}