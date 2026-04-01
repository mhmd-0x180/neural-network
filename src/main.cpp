#include <raylib.h>
#include <raymath.h>
#include "NeuralNetwork.hpp"
#include <rlgl.h>
#include <iomanip>
#include "mnist/mnist_reader.hpp"
#include <destortImages.hpp>
#include <thread>
const u32 width = 1280, height = 720;

std::vector<f64> labelToTargetvector(u8 target)
{
    std::vector<f64> vec(10, 0.0); // Create vector of size 10, all elements = 0.0
    vec[target] = 1.0;
    return vec;
}

u8 targetVectorToLabel(std::vector<f64> vec)
{
    for (u8 i = 0; i < vec.size(); i++)
    {
        if (vec[i] == 1)
            return i;
    }
    return 255;
}

void clamp(i8 &input, i8 min, i8 max)
{
    if (input < min)
        input = min;
    if (input > max)
        input = max;
}

std::vector<f64> normalizeImage(const std::vector<u8> &image)
{
    std::vector<f64> outputImage(image.size());
    for (u32 i = 0; i < image.size(); i++)
    {
        outputImage[i] = f64(image[i]) / 255.0;
    }
    return outputImage;
}
enum Mode
{
    DRAW,
    PREVIEW
};
struct imagePreview
{
    i32 currentImageIndex = 0;
    Mode mode = DRAW;
    std::vector<f64> image;
    Font customFont;
    bool hasChanged = true;
    imagePreview()
    {
        image.resize(28 * 28);
        customFont = LoadFontEx("resources/fonts/Roboto-Bold.ttf", 32, 0, 0);
    }

    ~imagePreview()
    {
        UnloadFont(customFont);
    }

    void display(Vector2 pos, u32 size, const std::vector<std::vector<f64>> &images, const std::vector<std::vector<f64>> &labels)
    {
        u32 pixelSize = u32(f32(size) / 28.0);
        DrawRectangleLines(pos.x, pos.y, 28 * pixelSize, 28 * pixelSize, WHITE);

        hasChanged = IsKeyPressed(KEY_C) | IsKeyPressed(KEY_D) | IsKeyPressed(KEY_DOWN) | IsKeyPressed(KEY_UP) | IsKeyPressed(KEY_RIGHT) | IsKeyPressed(KEY_LEFT) | IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

        if (IsKeyPressed(KEY_D))
        {
            mode = mode ? DRAW : PREVIEW;
        };

        if (mode == PREVIEW)
        {
            i32 jumbSize = 60;

            if (IsKeyPressed(KEY_LEFT) && currentImageIndex > 0)
                currentImageIndex--;
            if (IsKeyPressed(KEY_RIGHT) && currentImageIndex < images.size() - 1)
                currentImageIndex++;
            if (IsKeyPressed(KEY_UP) && currentImageIndex + (jumbSize - 1) < images.size() - 1)
                currentImageIndex += (jumbSize - 1);
            if (IsKeyPressed(KEY_DOWN) && currentImageIndex - (jumbSize - 1) > 0)
                currentImageIndex -= (jumbSize - 1);

            DrawTextEx(customFont, ("image : " + std::to_string(currentImageIndex)).c_str(), {pos.x + size / 2 - 60, pos.y - 30}, 25, 1, WHITE);

            for (u32 y = 0; y < 28; y++)
            {
                for (u32 x = 0; x < 28; x++)
                {
                    u8 pixelValue = u8(255.0 * images[currentImageIndex][y * 28 + x]);
                    DrawRectangle(pos.x + x * pixelSize, pos.y + y * pixelSize, pixelSize, pixelSize, {pixelValue, pixelValue, pixelValue, 255});
                }
            }

            DrawTextEx(customFont, ("label : " + std::to_string(targetVectorToLabel(labels[currentImageIndex]))).c_str(), {pos.x + size / 2 - 60, pos.y + size + 30}, 50, 1, WHITE);
        }

        if (mode == DRAW)
        {
            DrawTextEx(customFont, "draw an image", {pos.x + size / 2 - 60, pos.y - 30}, 25, 1, WHITE);
 
            if (IsKeyPressed(KEY_C))
            {
                for (u32 i = 0; i < image.size(); i++)
                {
                    image[i] = 0;
                }
            }

            i8 onImageX = (GetMouseX() - pos.x) / pixelSize;
            i8 onImageY = (GetMouseY() - pos.y) / pixelSize;

            clamp(onImageX, 0, 27);
            clamp(onImageY, 0, 27);

            // Brush settings
            const int brushSize = 3;          // Radius of the brush (adjust as needed)
            const float brushStrength = 0.8f; // Brush opacity/strength

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                float targetValue = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1.0f : 0.0f;

                // Apply brush effect in a circular area
                for (int dy = -brushSize; dy <= brushSize; dy++)
                {
                    for (int dx = -brushSize; dx <= brushSize; dx++)
                    {
                        int pixelX = onImageX + dx;
                        int pixelY = onImageY + dy;

                        // Check bounds
                        if (pixelX >= 0 && pixelX < 28 && pixelY >= 0 && pixelY < 28)
                        {
                            // Calculate distance from brush center
                            float distance = sqrt(dx * dx + dy * dy);

                            // Only affect pixels within brush radius
                            if (distance <= brushSize)
                            {
                                // Calculate brush falloff (softer edges)
                                float falloff = 1.0f - (distance / brushSize);
                                float effectiveStrength = brushStrength * falloff;

                                int index = pixelY * 28 + pixelX;

                                // Blend the current pixel value with the target value
                                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                                {
                                    // For drawing (left mouse), blend towards white
                                    image[index] = image[index] + (targetValue - image[index]) * effectiveStrength;
                                }
                                else
                                {
                                    // For erasing (right mouse), blend towards black
                                    image[index] = image[index] * (1.0f - effectiveStrength);
                                }

                                // Clamp values to [0, 1] range
                                if (image[index] > 1.0f)
                                    image[index] = 1.0f;
                                if (image[index] < 0.0f)
                                    image[index] = 0.0f;
                            }
                        }
                    }
                }
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
    }
};

int main()
{
    auto dataset = mnist::read_dataset<std::vector, std::vector, uint8_t, uint8_t>("resources/");
    auto &images = dataset.training_images;
    auto &labels = dataset.training_labels;

    u32 imagesCount = images.size();
    u32 imageSize = images[0].size();
    u32 labelsCount = labels.size();

    f32 dataPercentage = 60; // traning the network with x% of the data


    u32 dataTotrainSize = u32(imagesCount * (dataPercentage / 100.0));

    std::vector<std::vector<f64>> normalizedImages(dataTotrainSize);
    std::vector<std::vector<f64>> labelsAsVector(dataTotrainSize);

    // converting labes to target vector
    for (u32 i = 0; i < dataTotrainSize; i++)
    {
        labelsAsVector[i] = labelToTargetvector(labels[i]);
    }
    // normalizing images
    for (u32 i = 0; i < dataTotrainSize; i++)
    {
        normalizedImages[i] = normalizeImage(images[i]);
        // std::cout<<i<<"\n";
    }

    std::cout << "normalizing complete \n";

    applyRandomAugmentations(normalizedImages, labelsAsVector);

    std::cout << "random augmentations done .\n";
    // Zip the images and lables into pairs
    std::vector<std::pair<std::vector<f64>, std::vector<f64>>> combined(dataTotrainSize);
    for (u32 i = 0; i < dataTotrainSize; i++)
    {
        combined[i] = {normalizedImages[i], labelsAsVector[i]};
    }

    // Shuffle pairs
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(combined.begin(), combined.end(), rng);

    // Unzip
    for (u32 i = 0; i < dataTotrainSize; i++)
    {
        normalizedImages[i] = combined[i].first;
        labelsAsVector[i] = combined[i].second;
    }

    NeuralNetwork net({imageSize, 100, 100, 10}, 0.15);

    // u32 batchSize = 64;
    // u32 numThreads = std::thread::hardware_concurrency();
    // net.train(20, normalizedImages, labelsAsVector, batchSize, numThreads);

    // net.saveToFile("test.txt");
    // std::cout << "saving complete\n";

    // net.loadFromFile("resources/weightsAndbiases.txt");
    net.loadFromFile("gpu_model_v2.txt");

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_NONE);
    InitWindow(width, height, "neural network");

    SetTargetFPS(60);
    imagePreview preview;
    preview.mode = PREVIEW;
    while (not WindowShouldClose())
    {
        // Vector2 mousepos = {GetMouseX(), GetMouseY()};
        BeginDrawing();
        ClearBackground({10, 10, 10}); //

        u32 size = 150 + height / 2;

        preview.display({(f32)40, (f32)height / 2 - (size / 2.0f)}, size, normalizedImages, labelsAsVector);

        if (preview.mode == DRAW)
        {
            if (preview.hasChanged)
            {
                net.predict(preview.image);
                preview.hasChanged = false;
            }
        }
        else
        {
            if (preview.hasChanged)
            {
                net.predict(normalizedImages[preview.currentImageIndex]);
                 preview.hasChanged = false;
            }
        }
        net.draw({(size * 1.2f), 0}, {width / 2 + 20, height - 1});

        DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}