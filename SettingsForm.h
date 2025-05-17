#pragma once
#include "NativeCalendar.h"
#include "WeatherEditorForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace CalendarApp {
    public ref class SettingsForm : public Form {
    private:
        NativeCalendar* nativeCalendar;
        CheckBox^ weatherCheckBox;
        Button^ editWeatherButton;

    public:
        SettingsForm(NativeCalendar* calendar) : nativeCalendar(calendar) {
            InitializeComponent();
            this->CenterToScreen();
        }

        void InitializeComponent() {
            this->Text = L"Настройки";
            this->Size = System::Drawing::Size(300, 200);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;

            // Чекбокс для учета погоды
            weatherCheckBox = gcnew CheckBox();
            weatherCheckBox->Text = L"Учитывать погоду при поливе";
            weatherCheckBox->Checked = nativeCalendar->IsWeatherAffectingWatering();
            weatherCheckBox->Location = Point(20, 20);
            weatherCheckBox->Size = System::Drawing::Size(250, 30);
            this->Controls->Add(weatherCheckBox);

            // Кнопка редактирования погоды
            editWeatherButton = gcnew Button();
            editWeatherButton->Text = L"Редактировать погоду";
            editWeatherButton->Size = System::Drawing::Size(200, 30);
            editWeatherButton->Location = Point(20, 60);
            editWeatherButton->Click += gcnew EventHandler(this, &SettingsForm::OnEditWeatherClick);
            this->Controls->Add(editWeatherButton);

            // Кнопка сохранения
            Button^ saveButton = gcnew Button();
            saveButton->Text = L"Сохранить";
            saveButton->Size = System::Drawing::Size(200, 30);
            saveButton->Location = Point(20, 100);
            saveButton->Click += gcnew EventHandler(this, &SettingsForm::OnSaveClick);
            this->Controls->Add(saveButton);
        }

        void OnSaveClick(Object^ sender, EventArgs^ e) {
            nativeCalendar->SetUseWeatherForWatering(weatherCheckBox->Checked);
            this->DialogResult = Windows::Forms::DialogResult::OK;
            this->Close();
        }

        void OnEditWeatherClick(Object^ sender, EventArgs^ e) {
            auto weatherEditor = gcnew CalendarApp::WeatherEditorForm(nativeCalendar); // Явное указание пространства имён
            weatherEditor->ShowDialog();
        }
    };
}