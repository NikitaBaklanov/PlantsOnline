#pragma once
#include "NativeCalendar.h"
#include <msclr/marshal_cppstd.h>
#include <Windows.h>
#include <curl/curl.h>

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace CalendarApp {
    public ref class AddPlantForm : public Form {
    private:
        NativeCalendar* nativeCalendar;
        ComboBox^ plantCombo;
        DateTimePicker^ datePicker;
        Button^ okButton;

    public:
        AddPlantForm(NativeCalendar* calendar) {
            nativeCalendar = calendar;
            InitializeComponent();
            this->CenterToScreen();
        }

        void InitializeComponent() {

            this->Text = L"Добавить растение";
            this->Size = System::Drawing::Size(350, 220);
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->MinimizeBox = false;

            
            plantCombo = gcnew System::Windows::Forms::ComboBox();
            plantCombo->Location = System::Drawing::Point(20, 20);
            plantCombo->Size = System::Drawing::Size(300, 25);
            plantCombo->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->Controls->Add(plantCombo);

            datePicker = gcnew System::Windows::Forms::DateTimePicker();
            datePicker->Location = System::Drawing::Point(20, 70);
            datePicker->Size = System::Drawing::Size(300, 25);
            datePicker->Format = System::Windows::Forms::DateTimePickerFormat::Short;
            this->Controls->Add(datePicker);

            okButton = gcnew System::Windows::Forms::Button();
            okButton->Text = L"Добавить";
            okButton->Size = System::Drawing::Size(120, 40);
            okButton->Location = System::Drawing::Point(
                (this->Width - okButton->Width) / 2,
                120
            );
            okButton->Font = gcnew System::Drawing::Font(L"Arial", 10, System::Drawing::FontStyle::Bold);
            okButton->Click += gcnew System::EventHandler(this, &AddPlantForm::OnOKClick);
            this->Controls->Add(okButton);

            for (const auto& plant : nativeCalendar->plantDB.getAvailablePlants()) {
                plantCombo->Items->Add(gcnew System::String(plant.first.c_str()));
            }

            if (plantCombo->Items->Count > 0) {
                plantCombo->SelectedIndex = 0;
            }
        }

        void OnOKClick(Object^ sender, EventArgs^ e) {
            if (plantCombo->SelectedIndex == -1) {
                MessageBox::Show(L"ТЕКСТ!", L"Я ХЗ ЧТО ЗДЕСЬ");
                return;
            }

            Plant newPlant;
            newPlant.name = msclr::interop::marshal_as<std::string>(plantCombo->Text);
            newPlant.plantingDate = Date(
                datePicker->Value.Day,
                datePicker->Value.Month,
                datePicker->Value.Year
            );
            newPlant.wateringFrequency = nativeCalendar->plantDB.getAvailablePlants().at(newPlant.name);
            nativeCalendar->plantDB.addPlant(newPlant);
            this->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->Close();
        }
    };
}