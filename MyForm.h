#pragma once
#include "NativeCalendar.h"
#include "AddPlantForm.h"
#include <msclr/marshal_cppstd.h>

namespace CalendarApp {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class MainForm : public Form {
    private:
        Panel^ sfmlPanel;
        Label^ monthLabel;
        Button^ prevButton;
        Button^ nextButton;
        ListBox^ plantsList;
        Button^ addButton;
        Button^ deleteButton;
        Button^ settingsButton;
        NativeCalendar* nativeCalendar;

        void InitializeComponent() {
            this->Text = L"Трекер полива растений";
            this->Size = Drawing::Size(1200, 600);
            this->StartPosition = FormStartPosition::CenterScreen;

            Panel^ leftPanel = gcnew Panel();
            leftPanel->Dock = DockStyle::Left;
            leftPanel->Width = 200;
            leftPanel->BackColor = Color::FromArgb(220, 220, 220);
            this->Controls->Add(leftPanel);

            monthLabel = gcnew Label();
            monthLabel->Text = L"Май 2025";
            monthLabel->Location = Point(20, 20);
            monthLabel->Font = gcnew Drawing::Font(L"Arial", 12, FontStyle::Bold);
            leftPanel->Controls->Add(monthLabel);

            prevButton = gcnew Button();
            prevButton->Text = L"< Предыдущий";
            prevButton->Location = Point(20, 60);
            prevButton->Size = Drawing::Size(160, 30);
            prevButton->Click += gcnew EventHandler(this, &MainForm::OnPrevClick);
            leftPanel->Controls->Add(prevButton);

            nextButton = gcnew Button();
            nextButton->Text = L"Следующий >";
            nextButton->Location = Point(20, 100);
            nextButton->Size = Drawing::Size(160, 30);
            nextButton->Click += gcnew EventHandler(this, &MainForm::OnNextClick);
            leftPanel->Controls->Add(nextButton);

            Label^ listLabel = gcnew Label();
            listLabel->Text = L"Список растений";
            listLabel->Location = Point(20, 150);
            listLabel->Font = gcnew Drawing::Font(L"Arial", 10, FontStyle::Bold);
            leftPanel->Controls->Add(listLabel);

            plantsList = gcnew ListBox();
            plantsList->Location = Point(20, 180);
            plantsList->Size = Drawing::Size(160, 300);
            plantsList->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::OnPlantSelected);
            leftPanel->Controls->Add(plantsList);

            Panel^ rightPanel = gcnew Panel();
            rightPanel->Dock = DockStyle::Right;
            rightPanel->Width = 200;
            rightPanel->BackColor = Color::FromArgb(220, 220, 220);
            this->Controls->Add(rightPanel);

            addButton = gcnew Button();
            addButton->Text = L"Добавить растение";
            addButton->Location = Point(20, 180);
            addButton->Size = Drawing::Size(160, 40);
            addButton->BackColor = Color::FromArgb(144, 238, 144);
            addButton->Click += gcnew EventHandler(this, &MainForm::OnAddClick);
            rightPanel->Controls->Add(addButton);

            deleteButton = gcnew Button();
            deleteButton->Text = L"Удалить растение";
            deleteButton->Location = Point(20, 230);
            deleteButton->Size = Drawing::Size(160, 40);
            deleteButton->BackColor = Color::FromArgb(255, 160, 122);
            deleteButton->Click += gcnew EventHandler(this, &MainForm::OnDeleteClick);
            rightPanel->Controls->Add(deleteButton);

            settingsButton = gcnew Button();
            settingsButton->Text = L"Настройки";
            settingsButton->Location = Point(20, 280);
            settingsButton->Size = Drawing::Size(160, 40);
            settingsButton->BackColor = Color::FromArgb(173, 216, 230);
            rightPanel->Controls->Add(settingsButton);

            sfmlPanel = gcnew Panel();
            sfmlPanel->Dock = DockStyle::Fill;
            this->Controls->Add(sfmlPanel);
        }

        void InitializeSFML() {
            try {
                nativeCalendar = new NativeCalendar();
                nativeCalendar->Initialize(sfmlPanel->Handle.ToPointer());

                Timer^ timer = gcnew Timer();
                timer->Interval = 16;
                timer->Tick += gcnew EventHandler(this, &MainForm::OnTimerTick);
                timer->Start();
            }
            catch (const std::exception& e) {
                MessageBox::Show(gcnew String(e.what()), L"Ошибка SFML");
            }
        }

        void OnTimerTick(Object^ sender, EventArgs^ e) {
            nativeCalendar->Render();
        }

        void OnPrevClick(Object^ sender, EventArgs^ e) {
            nativeCalendar->PrevMonth();
        }

        void OnNextClick(Object^ sender, EventArgs^ e) {
            nativeCalendar->NextMonth();
        }

        void OnAddClick(Object^ sender, EventArgs^ e) {
            auto addForm = gcnew AddPlantForm(nativeCalendar);
            if (addForm->ShowDialog() == Windows::Forms::DialogResult::OK) {
                UpdatePlantList();
            }
        }

        void OnDeleteClick(Object^ sender, EventArgs^ e) {
            if (plantsList->SelectedIndex != -1) {
                nativeCalendar->plantDB.removePlant(plantsList->SelectedIndex);
                UpdatePlantList();
            }
        }

        void OnPlantSelected(Object^ sender, EventArgs^ e) {
            nativeCalendar->plantDB.selectedIndex = plantsList->SelectedIndex;
            nativeCalendar->Render();
        }

        void UpdatePlantList() {
            plantsList->Items->Clear();
            for (const auto& plant : nativeCalendar->plantDB.getUserPlants()) {
                plantsList->Items->Add(gcnew String(plant.name.c_str()));
            }
        }

    public:
        MainForm() {
            InitializeComponent();
            InitializeSFML();
        }

        ~MainForm() {
            this->!MainForm();
        }

        !MainForm() {
            if (nativeCalendar) {
                delete nativeCalendar;
                nativeCalendar = nullptr;
            }
        }
    };
}