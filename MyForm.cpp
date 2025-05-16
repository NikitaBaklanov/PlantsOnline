#include "MyForm.h"
#include <clocale>

[System::STAThread]
int main(array<System::String^>^ args) {
    std::locale::global(std::locale(""));
    setlocale(LC_ALL, "");
    System::Windows::Forms::Application::EnableVisualStyles();
    System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
    System::Windows::Forms::Application::Run(gcnew CalendarApp::MainForm());
    return 0;
}