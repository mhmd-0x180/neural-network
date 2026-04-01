#include <random>
#include <algorithm>
#include <cmath>
#include <types.hpp>
#include <iostream>


class ImageAugmenter {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<f64> uniform_dist;
    std::normal_distribution<f64> normal_dist;
    
public:
    ImageAugmenter(u32 seed = std::random_device{}()) 
        : rng(seed), uniform_dist(0.0, 1.0), normal_dist(0.0, 1.0) {}
    
    // Helper function to get pixel value safely (returns 0 for out-of-bounds)
    f64 getPixel(const std::vector<f64>& image, i32 x, i32 y) {
        if (x < 0 || x >= 28 || y < 0 || y >= 28) return 0.0;
        return image[y * 28 + x];
    }
    
    // Rotate image by angle in degrees
    std::vector<f64> rotate(const std::vector<f64>& image, f64 angleDegrees) {
        std::vector<f64> result(784, 0.0);
        f64 angleRad = angleDegrees * M_PI / 180.0;
        f64 cosAngle = std::cos(angleRad);
        f64 sinAngle = std::sin(angleRad);
        f64 centerX = 13.5, centerY = 13.5;
        
        for (i32 y = 0; y < 28; y++) {
            for (i32 x = 0; x < 28; x++) {
                // Translate to center, rotate, translate back
                f64 dx = x - centerX;
                f64 dy = y - centerY;
                f64 sourceX = dx * cosAngle + dy * sinAngle + centerX;
                f64 sourceY = -dx * sinAngle + dy * cosAngle + centerY;
                
                // Bilinear interpolation
                i32 x1 = (i32)std::floor(sourceX);
                i32 y1 = (i32)std::floor(sourceY);
                i32 x2 = x1 + 1;
                i32 y2 = y1 + 1;
                
                f64 wx = sourceX - x1;
                f64 wy = sourceY - y1;
                
                f64 val = (1 - wx) * (1 - wy) * getPixel(image, x1, y1) +
                         wx * (1 - wy) * getPixel(image, x2, y1) +
                         (1 - wx) * wy * getPixel(image, x1, y2) +
                         wx * wy * getPixel(image, x2, y2);
                
                result[y * 28 + x] = std::clamp(val, 0.0, 1.0);
            }
        }
        return result;
    }
    
    // Shift image horizontally (positive = right, negative = left)
    std::vector<f64> shiftHorizontal(const std::vector<f64>& image, i32 pixels) {
        std::vector<f64> result(784, 0.0);
        
        for (i32 y = 0; y < 28; y++) {
            for (i32 x = 0; x < 28; x++) {
                i32 sourceX = x - pixels;
                if (sourceX >= 0 && sourceX < 28) {
                    result[y * 28 + x] = image[y * 28 + sourceX];
                }
            }
        }
        return result;
    }
    
    // Shift image vertically (positive = down, negative = up)
    std::vector<f64> shiftVertical(const std::vector<f64>& image, i32 pixels) {
        std::vector<f64> result(784, 0.0);
        
        for (i32 y = 0; y < 28; y++) {
            for (i32 x = 0; x < 28; x++) {
                i32 sourceY = y - pixels;
                if (sourceY >= 0 && sourceY < 28) {
                    result[y * 28 + x] = image[sourceY * 28 + x];
                }
            }
        }
        return result;
    }
    
    // Scale image (factor > 1 = zoom in, factor < 1 = zoom out)
    std::vector<f64> scale(const std::vector<f64>& image, f64 factor) {
        std::vector<f64> result(784, 0.0);
        f64 centerX = 13.5, centerY = 13.5;
        
        for (i32 y = 0; y < 28; y++) {
            for (i32 x = 0; x < 28; x++) {
                f64 dx = (x - centerX) / factor;
                f64 dy = (y - centerY) / factor;
                f64 sourceX = dx + centerX;
                f64 sourceY = dy + centerY;
                
                // Bilinear interpolation
                i32 x1 = (i32)std::floor(sourceX);
                i32 y1 = (i32)std::floor(sourceY);
                i32 x2 = x1 + 1;
                i32 y2 = y1 + 1;
                
                if (x1 >= 0 && x2 < 28 && y1 >= 0 && y2 < 28) {
                    f64 wx = sourceX - x1;
                    f64 wy = sourceY - y1;
                    
                    f64 val = (1 - wx) * (1 - wy) * getPixel(image, x1, y1) +
                             wx * (1 - wy) * getPixel(image, x2, y1) +
                             (1 - wx) * wy * getPixel(image, x1, y2) +
                             wx * wy * getPixel(image, x2, y2);
                    
                    result[y * 28 + x] = std::clamp(val, 0.0, 1.0);
                }
            }
        }
        return result;
    }
    
    // Add Gaussian noise
    std::vector<f64> addNoise(const std::vector<f64>& image, f64 stddev = 0.1) {
        std::vector<f64> result(image.size());
        std::normal_distribution<f64> noise(0.0, stddev);
        
        for (size_t i = 0; i < image.size(); i++) {
            f64 noisyValue = image[i] + noise(rng);
            result[i] = std::clamp(noisyValue, 0.0, 1.0);
        }
        return result;
    }
    
    // Random augmentation (combines multiple transformations)
    std::vector<f64> randomAugment(const std::vector<f64>& image, 
                                  f64 maxRotation = 15.0,
                                  i32 maxShift = 2,
                                  f64 maxScale = 0.2,
                                  f64 noiseStd = 0.05) {
        std::vector<f64> result = image;
        
        // Random rotation
        if (maxRotation > 0) {
            f64 angle = (uniform_dist(rng) - 0.5) * 2 * maxRotation;
            result = rotate(result, angle);
        }
        
        // Random horizontal shift
        if (maxShift > 0) {
            i32 shiftX = (i32)((uniform_dist(rng) - 0.5) * 2 * maxShift);
            result = shiftHorizontal(result, shiftX);
        }
        
        // Random vertical shift
        if (maxShift > 0) {
            i32 shiftY = (i32)((uniform_dist(rng) - 0.5) * 2 * maxShift);
            result = shiftVertical(result, shiftY);
        }
        
        // Random scaling
        if (maxScale > 0) {
            f64 scaleFactor = 1.0 + (uniform_dist(rng) - 0.5) * 2 * maxScale;
            result = scale(result, scaleFactor);
        }
        
        // Add noise
        if (noiseStd > 0) {
            result = addNoise(result, noiseStd);
        }
        
        return result;
    }
    
    // Generate augmented dataset
    void augmentDataset(std::vector<std::vector<f64>>& images,
                       std::vector<std::vector<f64>>& labels,
                       u32 augmentationsPerImage = 3,
                       f64 maxRotation = 15.0,
                       i32 maxShift = 2,
                       f64 maxScale = 0.2,
                       f64 noiseStd = 0.05) {
        
        u32 originalSize = images.size();
        u32 newSize = originalSize * (1 + augmentationsPerImage);
        
        images.reserve(newSize);
        labels.reserve(newSize);
        
        for (u32 i = 0; i < originalSize; i++) {
            for (u32 aug = 0; aug < augmentationsPerImage; aug++) {
                std::vector<f64> augmentedImage = randomAugment(images[i], 
                                                               maxRotation, 
                                                               maxShift, 
                                                               maxScale, 
                                                               noiseStd);
                images.push_back(augmentedImage);
                labels.push_back(labels[i]); // Same label as original
            }
            
            // Progress indicator
            if (i % 1000 == 0) {
                std::cout << "Augmented " << i << "/" << originalSize << " images\n";
            }
        }
        
        std::cout << "Dataset augmentation complete. New size: " << images.size() << "\n";
    }
};

// Usage example functions to add to your main.cpp:

// Example 1: Apply specific transformations
void applySpecificTransformations(std::vector<std::vector<f64>>& normalizedImages,
                                 std::vector<std::vector<f64>>& labelsAsVector) {
    ImageAugmenter augmenter(42); // Fixed seed for reproducibility
    
    u32 originalSize = normalizedImages.size();
    
    for (u32 i = 0; i < originalSize; i++) {
        // Rotate by 10 degrees
        auto rotated = augmenter.rotate(normalizedImages[i], 10.0);
        normalizedImages.push_back(rotated);
        labelsAsVector.push_back(labelsAsVector[i]);
        
        // Shift right by 2 pixels
        auto shifted = augmenter.shiftHorizontal(normalizedImages[i], 2);
        normalizedImages.push_back(shifted);
        labelsAsVector.push_back(labelsAsVector[i]);
        
        // Scale by 1.1x
        auto scaled = augmenter.scale(normalizedImages[i], 1.1);
        normalizedImages.push_back(scaled);
        labelsAsVector.push_back(labelsAsVector[i]);
        
        // Add noise
        auto noisy = augmenter.addNoise(normalizedImages[i], 0.1);
        normalizedImages.push_back(noisy);
        labelsAsVector.push_back(labelsAsVector[i]);
    }
}

// Example 2: Random augmentation
void applyRandomAugmentations(std::vector<std::vector<f64>>& normalizedImages,
                             std::vector<std::vector<f64>>& labelsAsVector) {
    ImageAugmenter augmenter;
    
    // This will triple your dataset size (2 augmentations per original image)
    augmenter.augmentDataset(normalizedImages, labelsAsVector, 
                            2,      // 1 augmentations per image
                            20.0,   // max rotation ±30 degrees
                            7,      // max shift ±7 pixels
                            0.25,    // max scale ±40%
                            0.012);  // noise std dev
}