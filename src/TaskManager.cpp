#include "TaskManager.h"
#include "imgui.h"

// This stores the name, CPU usage, and memory usage of each process.
struct Process {
    const char* name;
    float cpu;
    float memory;
};

void DrawTaskManager(bool* p_open) {
    // Sample data for the processes shown in Task Manager (dummy values only)
    static Process processes[] = {
        {"System", 3.2f, 120.0f},
        {"Desktop Shell", 4.1f, 320.0f},
        {"File Explorer", 1.4f, 256.0f},
        {"Discord", 5.8f, 450.0f},
        {"Google Chrome", 8.5f, 640.0f},
        {"Spotify", 1.2f, 150.0f},
        {"Steam", 2.4f, 280.0f},
        {"Visual Studio Code", 5.6f, 420.0f},
        {"Settings", 0.8f, 180.0f},
        {"Task Manager", 0.5f, 90.0f}
    };

    // Check if the Task Manager window should be shown
    if (p_open == nullptr || !*p_open) {
        return;
    }

    // Set the default size of the Task Manager window when opened for the first time
    ImGui::SetNextWindowSize(
        ImVec2(700, 450),
        ImGuiCond_FirstUseEver
    );

    // Create the Task Manager window with a close button
    // The close button updates p_open when clicked
    if (ImGui::Begin("Task Manager", p_open)) {

        // Add a short description at the top of the window
        ImGui::Text("Processes running in the system");
        ImGui::Separator();
        ImGui::Spacing();

        // Start the total CPU and memory usage at zero
        float totalCPU = 0;
        float totalMemory = 0;

        // Add the CPU and memory usage of every process
        for (const Process& process : processes) {
            totalCPU += process.cpu;
            totalMemory += process.memory;
        }

        // Count how many processes are listed
        int totalProcesses =
            sizeof(processes) / sizeof(processes[0]);

        // Show the total CPU usage, memory usage, and process count
        ImGui::Text("CPU Usage: %.1f%%", totalCPU);
        ImGui::SameLine(250);
        ImGui::Text("Memory: %.0f MB", totalMemory);
        ImGui::SameLine(470);
        ImGui::Text("Processes: %d", totalProcesses);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Processes");
        ImGui::Spacing();

        // Create a table with four columns for the process information
        if (ImGui::BeginTable(
                "ProcessTable",
                4,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable
            )) {

            // Set the names of each column
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("CPU (%)");
            ImGui::TableSetupColumn("Memory (MB)");

            // Display the column names at the top of the table
            ImGui::TableHeadersRow();

            // Go through each process and add its information to the table
            for (const Process& process : processes) {
                ImGui::TableNextRow();

                // Show the process name in the first column
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", process.name);

                // Show its current status in the second column
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Running");

                // Show its CPU usage in the third column
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f%%", process.cpu);

                // Show its memory usage in the fourth column
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.0f MB", process.memory);
            }

            // Finish creating the table
            ImGui::EndTable();
        }

        ImGui::Spacing();

        // Show a note that the displayed values are only examples
        ImGui::TextDisabled("CPU and memory values are dummy data.");
    }

    // End the Task Manager window
    // This should be called even if the window is not currently visible
    ImGui::End();
}