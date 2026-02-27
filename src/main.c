#include <raylib.h>

int main(void) {
  InitWindow(800, 600, "Nob Raylib Template");

  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("Hello, World!", 400, 300, 12, BLACK);

    DrawFPS(0, 0);
    EndDrawing();
  }

  CloseWindow();
}
