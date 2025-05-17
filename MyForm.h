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
        Panel^ leftPanel;
        Panel^ rightPanel;
        Label^ monthLabel;
        Button^ prevButton;
        Button^ nextButton;
        ListBox^ plantsList;
        Button^ addButton;
        Button^ deleteButton;
        Button^ settingsButton;
        NativeCalendar* nativeCalendar;

        void InitializeComponent() {

            this->Resize += gcnew EventHandler(this, &MainForm::OnResize);
            this->MinimumSize = System::Drawing::Size(800, 600);

            this->Text = L"Трекер полива растений";
            this->Size = Drawing::Size(1200, 750);
            this->StartPosition = FormStartPosition::CenterScreen;

            // Левая панель
            leftPanel = gcnew Panel();
            leftPanel->Dock = DockStyle::Left;
            leftPanel->Width = 200;
            leftPanel->BackColor = Color::FromArgb(240, 240, 240);
            this->Controls->Add(leftPanel);

            // Элементы левой панели
            monthLabel = gcnew Label();
            monthLabel->Text = L"Май 2025";
            monthLabel->Location = Point(20, 20);
            monthLabel->Size = Drawing::Size(180, 30);
            monthLabel->Font = gcnew Drawing::Font(L"Arial", 12, FontStyle::Bold);
            leftPanel->Controls->Add(monthLabel);

            prevButton = gcnew Button();
            prevButton->Text = L"< Предыдущий месяц";
            prevButton->Location = Point(20, 60);
            prevButton->Size = Drawing::Size(160, 30);
            prevButton->Click += gcnew EventHandler(this, &MainForm::OnPrevClick);
            leftPanel->Controls->Add(prevButton);

            nextButton = gcnew Button();
            nextButton->Text = L"Следующий месяц >";
            nextButton->Location = Point(20, 100);
            nextButton->Size = Drawing::Size(160, 30);
            nextButton->Click += gcnew EventHandler(this, &MainForm::OnNextClick);
            leftPanel->Controls->Add(nextButton);

            Label^ listLabel = gcnew Label();
            listLabel->Text = L"Список растений";
            listLabel->Location = Point(20, 140);
            listLabel->Font = gcnew Drawing::Font(L"Arial", 12, FontStyle::Bold);
            leftPanel->Controls->Add(listLabel);

            plantsList = gcnew ListBox();
            plantsList->Location = Point(20, 180);
            plantsList->Size = Drawing::Size(160, 500);
            plantsList->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::OnPlantSelected);
            leftPanel->Controls->Add(plantsList);

            // Правая панель
            rightPanel = gcnew Panel();
            rightPanel->Dock = DockStyle::Right;
            rightPanel->Width = 200;
            rightPanel->BackColor = Color::FromArgb(240, 240, 240);
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

            // Основная панель для SFML
            sfmlPanel = gcnew Panel();
            sfmlPanel->Dock = DockStyle::Fill;
            this->Controls->Add(sfmlPanel);
        }

        void InitializeSFML() {
            try {
                // Создаем экземпляр календаря
                nativeCalendar = new NativeCalendar();

                // Инициализируем SFML окно
                nativeCalendar->Initialize(sfmlPanel->Handle.ToPointer());

                // Устанавливаем текущие размеры панелей
                nativeCalendar->SetPanelWidths(
                    leftPanel->Width,
                    rightPanel->Width
                );

                // Рассчитываем масштаб DPI
                float dpiScaleX = this->DeviceDpi / 96.0f;
                float dpiScaleY = this->DeviceDpi / 96.0f;

                // Устанавливаем начальный размер с учетом DPI
                nativeCalendar->Resize(
                    static_cast<unsigned>(sfmlPanel->Width * dpiScaleX),
                    static_cast<unsigned>(sfmlPanel->Height * dpiScaleY)
                );

                // Инициализируем таймер для плавной отрисовки
                Timer^ timer = gcnew Timer();
                timer->Interval = 16; // ~60 FPS
                timer->Tick += gcnew EventHandler(this, &MainForm::OnTimerTick);
                timer->Start();

                // Обновляем метку с месяцем
                UpdateMonthLabel();

                // Обновляем список растений
                UpdatePlantList();
            }
            catch (const std::exception& e) {
                // Обработка ошибок инициализации
                String^ errorMsg = gcnew String(("Ошибка инициализации SFML: " + std::string(e.what())).c_str());
                MessageBox::Show(errorMsg, L"Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);

                // Пытаемся безопасно завершить работу
                if (nativeCalendar != nullptr) {
                    delete nativeCalendar;
                    nativeCalendar = nullptr;
                }
            }
        }

        void OnResize(Object^ sender, EventArgs^ e) {
            if (nativeCalendar != nullptr && sfmlPanel != nullptr) {
                nativeCalendar->Resize(
                    static_cast<unsigned>(sfmlPanel->Width),
                    static_cast<unsigned>(sfmlPanel->Height)
                );
            }
        }

        void OnTimerTick(Object^ sender, EventArgs^ e) {
            nativeCalendar->Render();
        }

        void OnPrevClick(Object^ sender, EventArgs^ e) {
            nativeCalendar->PrevMonth();
            UpdateMonthLabel();
        }

        void OnNextClick(Object^ sender, EventArgs^ e) {
            nativeCalendar->NextMonth();
            UpdateMonthLabel();
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
            nativeCalendar->SetSelectedPlantIndex(plantsList->SelectedIndex);
        }

        void UpdatePlantList() {
            plantsList->Items->Clear();
            for (const auto& plant : nativeCalendar->plantDB.getUserPlants()) {
                // Форматируем дату в виде "дд.мм.гг"
                std::string dateStr =
                    (plant.plantingDate.day < 10 ? "0" : "") + std::to_string(plant.plantingDate.day) + "." +
                    (plant.plantingDate.month < 10 ? "0" : "") + std::to_string(plant.plantingDate.month) + "." +
                    std::to_string(plant.plantingDate.year % 100); // Берем последние две цифры года

                // Объединяем название растения и дату
                std::string plantInfo = plant.name + " (" + dateStr + ")";
                plantsList->Items->Add(gcnew String(plantInfo.c_str()));
            }
        }

        void UpdateMonthLabel() {
            std::string monthName = nativeCalendar->GetMonthName(nativeCalendar->displayedMonth.month);
            monthLabel->Text = gcnew String((monthName + " " + std::to_string(nativeCalendar->displayedMonth.year)).c_str());
        }

    public:
        MainForm() {
            InitializeComponent();
            InitializeSFML();
        }

        ~MainForm() {
            delete nativeCalendar;
        }
    };
}