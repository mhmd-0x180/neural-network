#include "NeuralNetworkGPU.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <iomanip>
#include "mnist/mnist_reader.hpp"
#include <destortImages.hpp>
#include <thread>
#include <algorithm>
#include <random>

const u32 width = 1280, height = 720;

std::vector<f64> labelToTargetvector(u8 target)
{
    std::vector<f64> vec(10, 0.0);
    vec[target] = 1.0;
    return vec;
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

int main()
{
    // Initialize Window (Hidden for Compute Context)
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(100, 100, "Neural Network GPU (Headless)");

    std::cout << "Loading MNIST dataset..." << std::endl;
    auto dataset = mnist::read_dataset<std::vector, std::vector, uint8_t, uint8_t>("resources/");
    auto &images = dataset.training_images;
    auto &labels = dataset.training_labels;

    u32 imagesCount = images.size();
    u32 imageSize = images[0].size();
    
    // Use a smaller subset for testing if needed, or full set
    f32 dataPercentage = 100; 
    u32 dataTotrainSize = u32(imagesCount * (dataPercentage / 100.0));

    std::vector<std::vector<f64>> normalizedImages(dataTotrainSize);
    std::vector<std::vector<f64>> labelsAsVector(dataTotrainSize);

    std::cout << "Preprocessing " << dataTotrainSize << " images..." << std::endl;
    for (u32 i = 0; i < dataTotrainSize; i++)
    {
        labelsAsVector[i] = labelToTargetvector(labels[i]);
        normalizedImages[i] = normalizeImage(images[i]);
    }
    
    // Apply augmentations (CPU side for now)
    applyRandomAugmentations(normalizedImages, labelsAsVector);
    std::cout << "Data ready. Shuffling..." << std::endl;

    // Shuffle Data
    u32 totalSize = normalizedImages.size();
    std::vector<std::pair<std::vector<f64>, std::vector<f64>>> combined(totalSize);
    for(size_t i=0; i<totalSize; ++i) {
        combined[i] = {normalizedImages[i], labelsAsVector[i]};
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(combined.begin(), combined.end(), g);
    
    for(size_t i=0; i<totalSize; ++i) {
        normalizedImages[i] = combined[i].first;
        labelsAsVector[i] = combined[i].second;
    }
    std::cout << "Shuffling complete." << std::endl;

    // Initialize GPU Network
    // 784 -> 128 -> 64 -> 10
    std::vector<u32> topology = {784, 128,64, 10};
    NeuralNetworkGPU net(topology);
    
    // Initialize with batch size 64
    net.init(64);
    
    // Upload Data
    std::cout << "Uploading data to VRAM..." << std::endl;
    net.uploadTrainingData(normalizedImages, labelsAsVector);
    
    // Train
    std::cout << "Starting GPU Training..." << std::endl;
    net.train(20, 64, .5f); // 50 Epochs, Batch 64, LR 0.5
    
    std::cout << "Training Complete!" << std::endl;
    
    // Save Model
    net.saveModel("gpu_model_v2.txt");

    CloseWindow();
    return 0;
}
