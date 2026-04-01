#include <raylib.h>
#include <raymath.h>
#include "NeuralNetwork.hpp"
#include <rlgl.h>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>

const u32 width = 1280, height = 720;

void clamp(i8 &input, i8 min, i8 max)
{
    if (input < min)
        input = min;
    if (input > max)
        input = max;
}

struct imagePreview
{
    std::vector<f64> image;
    Font customFont;
    bool hasChanged = true;
    Vector2 lastMousePos = {-1, -1};
    bool wasDrawing = false;

    imagePreview()
    {
        image.resize(28 * 28, 0.0);
        customFont = LoadFontEx("resources/fonts/Roboto-Bold.ttf", 32, 0, 0);
    }

    ~imagePreview()
    {
        UnloadFont(customFont);
    }

    void applyBrush(int centerX, int centerY, bool isLeftClick) {
        // Brush settings
        const int brushSize = 2;          
        const float brushStrength = 1.0f; 

        // Apply brush effect in a circular area
        for (int dy = -brushSize; dy <= brushSize; dy++)
        {
            for (int dx = -brushSize; dx <= brushSize; dx++)
            {
                int pixelX = centerX + dx;
                int pixelY = centerY + dy;

                // Check bounds
                if (pixelX >= 0 && pixelX < 28 && pixelY >= 0 && pixelY < 28)
                {
                    // Calculate distance from brush center
                    float distance = sqrt(dx * dx + dy * dy);

                    // Only affect pixels within brush radius
                    if (distance <= brushSize + 0.5f) // Slightly larger to smooth edges
                    {
                        // Calculate brush falloff (softer edges)
                        float falloff = 1.0f - (distance / (brushSize + 1.0f)); 
                        if(falloff < 0) falloff = 0;
                        
                        float effectiveStrength = brushStrength * falloff;

                        int index = pixelY * 28 + pixelX;

                        if (isLeftClick)
                        {
                            // For drawing: Use MAX to prevent accumulation/thickening
                            // This ensures the pixel only gets as bright as the brush's max intensity at that radius
                            image[index] = std::max(image[index], (f64)effectiveStrength);
                        }
                        else
                        {
                            // For erasing: Blend towards black
                            image[index] = image[index] * (1.0f - effectiveStrength);
                        }

                        // Clamp values to [0, 1] range
                        if (image[index] > 1.0f) image[index] = 1.0f;
                        if (image[index] < 0.0f) image[index] = 0.0f;
                    }
                }
            }
        }
    }

    void display(Vector2 pos, u32 size)
    {
        u32 pixelSize = u32(f32(size) / 28.0);
        DrawRectangleLines(pos.x, pos.y, 28 * pixelSize, 28 * pixelSize, WHITE);

        // Check for input
        bool isLeftClick = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool isRightClick = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
        bool isDrawing = isLeftClick || isRightClick;
        bool isClearing = IsKeyPressed(KEY_C);
        
        hasChanged = isDrawing || isClearing;

        DrawTextEx(customFont, "Draw a digit", {pos.x + size / 2 - 60, pos.y - 30}, 25, 1, WHITE);
        DrawTextEx(customFont, "Press 'C' to clear", {pos.x + size / 2 - 70, pos.y + size + 10}, 20, 1, GRAY);

        if (isClearing)
        {
            std::fill(image.begin(), image.end(), 0.0);
        }

        if (isDrawing)
        {
            // Calculate current mouse position in grid coordinates
            Vector2 currentMousePos = { 
                (float)(GetMouseX() - pos.x) / pixelSize, 
                (float)(GetMouseY() - pos.y) / pixelSize 
            };

            // Clamp to bounds
            if(currentMousePos.x < 0) currentMousePos.x = 0;
            if(currentMousePos.x > 27) currentMousePos.x = 27;
            if(currentMousePos.y < 0) currentMousePos.y = 0;
            if(currentMousePos.y > 27) currentMousePos.y = 27;

            if (!wasDrawing) {
                lastMousePos = currentMousePos;
                wasDrawing = true;
            }

            // Interpolate between last position and current position to draw lines
            float dist = Vector2Distance(lastMousePos, currentMousePos);
            // Step size of 0.5 ensures we don't miss pixels
            int steps = (int)(dist * 2.0f) + 1; 

            for(int i = 0; i <= steps; i++) {
                float t = (float)i / steps;
                Vector2 p = Vector2Lerp(lastMousePos, currentMousePos, t);
                
                applyBrush((int)p.x, (int)p.y, isLeftClick);
            }

            lastMousePos = currentMousePos;
        }
        else
        {
            wasDrawing = false;
        }

        // Render the image
        for (u32 y = 0; y < 28; y++)
        {
            for (u32 x = 0; x < 28; x++)
            {
                u8 pixelValue = u8(255.0 * image[y * 28 + x]);
                DrawRectangle(pos.x + x * pixelSize, pos.y + y * pixelSize, pixelSize, pixelSize, {pixelValue, pixelValue, pixelValue, 255});
            }
        }
    }
};

int main()
{
    // Initialize Network
    // Topology doesn't matter much here as loadFromFile will resize it, 
    // but good to have a default.
    NeuralNetwork net({784, 128, 64, 10});

    // Try to load the latest model
    std::string modelFile = "gpu_model_data.txt";
    std::cout << "Attempting to load model: " << modelFile << std::endl;
    
    try {
        net.loadFromFile(modelFile);
        std::cout << "Model loaded successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading " << modelFile << ": " << e.what() << std::endl;
        std::cout << "Trying fallback: gpu_model.txt" << std::endl;
        try {
            net.loadFromFile("gpu_model.txt");
            std::cout << "Fallback model loaded successfully." << std::endl;
        } catch (const std::exception& e2) {
             std::cerr << "Error loading fallback model: " << e2.what() << std::endl;
             std::cerr << "Starting with untrained network!" << std::endl;
        }
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_NONE);
    InitWindow(width, height, "Neural Network - Draw Mode");
    SetTargetFPS(60);

    imagePreview preview;
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground({10, 10, 10});

        u32 size = 150 + height / 2;
        
        // Display the drawing grid
        preview.display({(f32)40, (f32)height / 2 - (size / 2.0f)}, size);

        // Run prediction if image changed
        if (preview.hasChanged)
        {
            net.predict(preview.image);
            preview.hasChanged = false;
        }

        // Draw the network visualization
        net.draw({(size * 1.2f), 0}, {width / 2 + 20, height - 1});

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
