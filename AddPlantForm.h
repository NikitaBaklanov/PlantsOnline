#pragma once
#include "NativeCalendar.h"
#include <msclr/marshal_cppstd.h>
#include <Windows.h>

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
        }

        void InitializeComponent() {
            this->Text = L"Добавить растение";
            this->Size = Drawing::Size(300, 200);

            plantCombo = gcnew ComboBox();
            plantCombo->Location = Point(20, 20);
            plantCombo->Width = 200;
            for (const auto& plant : nativeCalendar->plantDB.getAvailablePlants()) {
                plantCombo->Items->Add(gcnew String(plant.first.c_str()));
            }
            this->Controls->Add(plantCombo);

            datePicker = gcnew DateTimePicker();
            datePicker->Location = Point(20, 60);
            datePicker->Value = DateTime::Now;
            this->Controls->Add(datePicker);

            okButton = gcnew Button();
            okButton->Text = L"Добавить";
            okButton->Location = Point(20, 100);
            okButton->Click += gcnew EventHandler(this, &AddPlantForm::OnOKClick);
            this->Controls->Add(okButton);
        }

        void OnOKClick(Object^ sender, EventArgs^ e) {
            if (plantCombo->SelectedIndex == -1) {
                MessageBox::Show(L"Выберите растение!", L"Ошибка");
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