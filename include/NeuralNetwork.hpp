#include "types.hpp"
#include <vector>
#include <iostream>
#include <math.h>
#include "rng.h"
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <future>
#include <functional>
#include <memory>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i<threads; ++i)
            workers.emplace_back(
                [this] {
                    for(;;) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this]{ return this->stop || !this->tasks.empty(); });
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                }
            );
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

class NeuralNetwork
{
private:
    std::vector<u32> layerSizes;

    std::vector<std::vector<std::vector<f64>>> weights;
    std::vector<std::vector<f64>> biases;
    std::vector<std::vector<f64>> neurons;
    std::vector<std::vector<f64>> deltas;

    double learningRate = 0.5;

    // ========================drawing shit========================= //

    RenderTexture2D networkTexture;
    bool textureNeedsUpdate = true;
    Vector2 lastTextureSize = {0, 0};
    u32 lastHiddenLayersToShow = 0;
    Font customFont ;
    
void renderToTexture(Vector2 size, u32 numHiddenLayersToShow = 3)
{
    const u32 MAX_NEURONS_PER_LAYER = 16;
    const Color NEURON_COLOR = WHITE;
    const Color BORDER_COLOR = BLACK;

    // Pre-calculate constants
    const f32 nodeRadius = std::max(2.0f, size.y / (4.0f * MAX_NEURONS_PER_LAYER));
    const f32 horizontalMargin = nodeRadius * 5;
    const f32 verticalMargin = nodeRadius * 4;
    const f32 availableWidth = size.x - 2 * horizontalMargin;
    const f32 availableHeight = size.y - 2 * verticalMargin;
    const f32 layerSpacing = availableWidth / std::max(1.0f, (float)(layerSizes.size() - 1));
    const f32 maxWeight = 2.0f;
    const f32 centerY = size.y * 0.5f;

    // Begin texture mode (draw to texture instead of screen)
    BeginTextureMode(networkTexture);
    ClearBackground(BLANK); // Clear with transparent background

    // FIXED: Draw bounding box properly
    DrawRectangle(0, 0, (int)size.x, 1, WHITE);               // Top
    DrawRectangle(0, (int)size.y - 1, (int)size.x, 1, WHITE); // Bottom
    DrawRectangle(0, 0, 1, (int)size.y, WHITE);               // Left
    DrawRectangle((int)size.x - 1, 0, 1, (int)size.y, WHITE); // Right
    
    // Pre-allocate and calculate all neuron positions
    std::vector<std::vector<Vector2>> neuronPositions;
    neuronPositions.reserve(layerSizes.size());

    for (u32 l = 0; l < layerSizes.size(); l++)
    {
        const u32 count = layerSizes[l];
        // FIXED: Round to pixel boundaries to avoid sub-pixel positioning
        const f32 layerX = std::round(horizontalMargin + l * layerSpacing);
        std::vector<Vector2> positions;

        if (count <= MAX_NEURONS_PER_LAYER)
        {
            positions.reserve(count);
            if (count == 1)
            {
                positions.push_back({layerX, std::round(centerY)});
            }
            else
            {
                const f32 totalSpacing = availableHeight * (count - 1) / (MAX_NEURONS_PER_LAYER - 1);
                const f32 startY = verticalMargin + (availableHeight - totalSpacing) * 0.5f;
                const f32 stepY = totalSpacing / (count - 1);

                for (u32 n = 0; n < count; n++)
                {
                    // FIXED: Round Y positions to pixel boundaries
                    positions.push_back({layerX, std::round(startY + n * stepY)});
                }
            }
        }
        else
        {
            positions.reserve(17);
            const f32 stepY = availableHeight / 15.0f;

            // First 8 neurons
            for (u32 n = 0; n < 8; n++)
            {
                positions.push_back({layerX, std::round(verticalMargin + n * stepY)});
            }

            // Middle position for text
            positions.push_back({layerX, std::round(centerY)});

            // Last 8 neurons
            for (u32 n = 0; n < 8; n++)
            {
                positions.push_back({layerX, std::round(verticalMargin + (n + 8) * stepY)});
            }
        }
        neuronPositions.push_back(std::move(positions));
    }

    // Batch draw connections with proper bounds checking
    std::vector<std::pair<Vector2, Vector2>> blueConnections, redConnections;
    std::vector<unsigned char> blueAlpha, redAlpha;

    for (u32 l = 0; l < layerSizes.size() - 1; l++)
    {
        // Safety check - make sure weights array exists for this layer
        if (l >= weights.size())
            continue;

        const auto &currentLayer = neuronPositions[l];
        const auto &nextLayer = neuronPositions[l + 1];
        const bool currentCompressed = layerSizes[l] > MAX_NEURONS_PER_LAYER;
        const bool nextCompressed = layerSizes[l + 1] > MAX_NEURONS_PER_LAYER;

        for (u32 i = 0; i < currentLayer.size(); i++)
        {
            if (currentCompressed && i == 8)
                continue; // Skip text position

            // Calculate actual neuron index in the layer
            u32 actualI = i;
            if (currentCompressed && i > 8)
            {
                actualI = i - 1 + (layerSizes[l] - 16);
            }

            // Bounds check for weights array
            if (actualI >= weights[l].size())
                continue;

            for (u32 j = 0; j < nextLayer.size(); j++)
            {
                if (nextCompressed && j == 8)
                    continue; // Skip text position

                // Calculate actual neuron index in the next layer
                u32 actualJ = j;
                if (nextCompressed && j > 8)
                {
                    actualJ = j - 1 + (layerSizes[l + 1] - 16);
                }

                // Bounds check for weights array
                if (actualJ >= weights[l][actualI].size())
                    continue;

                const f64 weight = weights[l][actualI][actualJ];
                const f32 absWeight = std::abs(weight);

                // Much more visible alpha calculation
                const f32 normalizedWeight = std::min(1.0f, absWeight / maxWeight);
                // Map to range 120-255 for better visibility, with minimum opacity
                const unsigned char alpha = (unsigned char)(normalizedWeight * 135 + 120);

                if (weight >= 0)
                {
                    blueConnections.push_back({currentLayer[i], nextLayer[j]});
                    blueAlpha.push_back(alpha);
                }
                else
                {
                    redConnections.push_back({currentLayer[i], nextLayer[j]});
                    redAlpha.push_back(alpha);
                }
            }
        }
    }

    // Draw connections with better visibility
    for (size_t i = 0; i < blueConnections.size(); i++)
    {
        DrawLineV(blueConnections[i].first, blueConnections[i].second,
                  {50, 100, 255, blueAlpha[i]}); // Brighter blue
    }

    for (size_t i = 0; i < redConnections.size(); i++)
    {
        DrawLineV(redConnections[i].first, redConnections[i].second,
                  {255, 50, 50, redAlpha[i]}); // Brighter red
    }

    // Draw neurons and text
    for (u32 l = 0; l < layerSizes.size(); l++)
    {
        const u32 count = layerSizes[l];
        const bool isCompressed = (count > MAX_NEURONS_PER_LAYER);
        const auto &positions = neuronPositions[l];

        for (u32 i = 0; i < positions.size(); i++)
        {
            if (isCompressed && i == 8)
            {
                static char textBuffer[32];
                snprintf(textBuffer, sizeof(textBuffer), "%u", count);
                const int fontSize = std::max(25, (int)(nodeRadius * 0.8f));
                const int textWidth = MeasureText(textBuffer, fontSize);

                DrawTextEx(customFont, textBuffer, {(positions[i].x - textWidth * 0.5f), (positions[i].y - fontSize * 0.5f)},
                           fontSize, 1,
                           WHITE);
            }
            else
            {
                Color neuronColor = NEURON_COLOR;
                if (l < neurons.size())
                {
                    const u32 actualI = (isCompressed && i > 8) ? i - 1 + (count - 16) : i;
                    if (actualI < neurons[l].size())
                    {
                        const f32 activation = neurons[l][actualI];

                        // All neurons: black for 0, white for 1
                        const f32 intensity = std::min(1.0f, std::max(0.0f, activation)); // Clamp to [0,1]
                        const unsigned char colorValue = (unsigned char)(intensity * 255);
                        neuronColor = Color{colorValue, colorValue, colorValue, 255};
                    }
                }

                // FIXED: Round positions and use integer coordinates for circles
                int roundedX = (int)std::round(positions[i].x);
                int roundedY = (int)std::round(positions[i].y);
                int roundedRadius = (int)std::round(nodeRadius);

                // Draw filled circle
                DrawCircle(roundedX, roundedY, roundedRadius, neuronColor);

                // Draw white outline for ALL neurons
                DrawCircleLines(roundedX, roundedY, roundedRadius, WHITE);

                // Check if this is the output layer (last layer)
                if (l == layerSizes.size() - 1)
                {
                    const u32 actualI = (isCompressed && i > 8) ? i - 1 + (count - 16) : i;
                    
                    // Draw digit labels (0-9) inside the neuron circle for output layer
                    if (actualI < 10) // Only for digits 0-9
                    {
                        char digitText[2];
                        snprintf(digitText, sizeof(digitText), "%u", actualI);
                        
                        int fontSize = std::max(16, (int)(nodeRadius * 0.6f));
                        int textWidth = MeasureText(digitText, fontSize);
                        int textX = roundedX - textWidth / 2; // Center horizontally
                        int textY = roundedY - fontSize / 2;  // Center vertically
                        
                        // Use contrasting color based on neuron activation
                        Color textColor = WHITE;
                        if (l < neurons.size() && actualI < neurons[l].size())
                        {
                            const f32 activation = neurons[l][actualI];
                            // Use black text for bright neurons, white for dark neurons
                            textColor = (activation > 0.5f) ? BLACK : WHITE;
                        }
                        
                        DrawTextEx(customFont,
                                   digitText,
                                   {(f32)textX, (f32)textY},
                                   fontSize, 1,
                                   textColor);
                    }
                    
                    // Draw output values for the last layer
                    if (l < neurons.size() && actualI < neurons[l].size())
                    {
                        float outputVal = neurons[l][actualI];
                        char valueText[16];
                        snprintf(valueText, sizeof(valueText), "%.2f", outputVal);

                        int fontSize = std::max(20, (int)(nodeRadius * 0.7f));
                        int textX = roundedX + (int)(nodeRadius * 1.5f); // Positioned to the right
                        int textY = roundedY - fontSize / 2;

                        DrawTextEx(customFont,
                                   valueText,
                                   {(f32)textX, (f32)textY},
                                   fontSize, 1,
                                   WHITE);
                    }
                }
            }
        }
    }
    EndTextureMode();
}
    // ============================================================ //

    f64 sigmoid(f64 input)
    {
        return 1.0f / (1.0f + exp(-input));
    }
    f64 sigmoidDerevative(f64 activated)
    {
        return activated * (1.0 - activated);
    }

    void initialaze()
    {
        Rng rng;
        rng.seed(time(0));

        // weights
        for (i32 layer = 0; layer < layerSizes.size() - 1; layer++)
        {
            // resize the vector of the weights on each node in this layer to the layer size
            weights[layer].resize(layerSizes[layer]);

            for (u32 from = 0; from < layerSizes[layer]; from++)
            {
                // Resize the outgoing weights from this neuron to match the number of neurons in the next layer
                weights[layer][from].resize(layerSizes[layer + 1]);

                for (u32 to = 0; to < layerSizes[layer + 1]; to++)
                {
                    // Xavier initialization
                    f64 scale = std::sqrt(2.0 / (layerSizes[layer] + layerSizes[layer + 1]));
                    weights[layer][from][to] = rng.randDouble(-1.0, 1.0) * scale;
                }
            }
        }
        // biases
        for (u32 layer = 1; layer < layerSizes.size(); layer++) // must start at 1 cus the input layer don't need biases
        {
            biases[layer].resize(layerSizes[layer]);
            for (u32 neuron = 0; neuron < layerSizes[layer]; neuron++)
            {
                // good enought initialization small random value
                biases[layer][neuron] = rng.randDouble(0, 0.1);
            }
        }

        // neurons and deltas
        for (u32 layer = 0; layer < layerSizes.size(); layer++)
        {
            neurons[layer].resize(layerSizes[layer]);
            deltas[layer].resize(layerSizes[layer]);
        }
    };

public:
    NeuralNetwork(const std::vector<u32> &layerSizes, f64 learningRate = 0.5)
    {
        customFont = LoadFontEx("resources/fonts/Roboto-Bold.ttf", 32, 0, 0);
        this->learningRate = learningRate;
        this->layerSizes = layerSizes;

        u32 numLayers = layerSizes.size();

        // evey n number of layer there are n-1 number of weight networks between them
        weights.resize(numLayers - 1);
        // the first layer don't need biases but we have to add it to make the calculation simpler making sure to start from 1 when iterating
        biases.resize(numLayers);

        deltas.resize(numLayers);
        neurons.resize(numLayers);

        initialaze();
    };
    ~NeuralNetwork() {
        // Clean up render texture
        if (networkTexture.id != 0) {
            UnloadRenderTexture(networkTexture);
        }
        
        // Clean up font
        UnloadFont(customFont);
    }
    void saveToFile(const std::string &filename)
    {
        std::ofstream out(filename);
        if (!out.is_open())
            throw std::runtime_error("Failed to open file for saving.");

        // Set high precision for double values
        out << std::setprecision(16);

        // Save layer sizes
        out << layerSizes.size() << '\n';
        for (u32 size : layerSizes)
            out << size << ' ';
        out << '\n';

        // Save weights
        for (const auto &layer : weights)
        {
            for (const auto &neuronWeights : layer)
            {
                for (f64 weight : neuronWeights)
                {
                    out << weight << ' ';
                }
                out << '\n';
            }
        }

        // Save biases
        for (const auto &layerBiases : biases)
        {
            for (f64 bias : layerBiases)
            {
                out << bias << ' ';
            }
            out << '\n';
        }

        out.close();
    }

    void loadFromFile(const std::string &filename)
    {
        std::ifstream in(filename);
        if (!in.is_open())
            throw std::runtime_error("Failed to open file for loading.");

        size_t numLayers;
        in >> numLayers;

        if (numLayers < 2)
        {
            throw std::runtime_error("Invalid number of layers: must be at least 2");
        }

        layerSizes.clear();
        layerSizes.reserve(numLayers);

        for (size_t i = 0; i < numLayers; ++i)
        {
            u32 size;
            in >> size;
            if (size == 0)
            {
                throw std::runtime_error("Invalid layer size: cannot be 0");
            }
            layerSizes.push_back(size);
        }

        // Clear and resize all containers
        weights.clear();
        biases.clear();
        neurons.clear();
        deltas.clear();

        weights.resize(numLayers - 1);
        biases.resize(numLayers);
        neurons.resize(numLayers);
        deltas.resize(numLayers);

        // Initialize neurons and deltas
        for (size_t i = 0; i < numLayers; ++i)
        {
            neurons[i].resize(layerSizes[i], 0.0);
            deltas[i].resize(layerSizes[i], 0.0);
        }

        // === Load weights ===
        // The weights matrix should match the structure used in saveToFile
        // weights[layer][from_neuron][to_neuron]
        // where layer goes from 0 to numLayers-2
        for (size_t layer = 0; layer < numLayers - 1; ++layer)
        {
            size_t fromSize = layerSizes[layer];   // neurons in current layer
            size_t toSize = layerSizes[layer + 1]; // neurons in next layer

            weights[layer].resize(fromSize);

            for (size_t from = 0; from < fromSize; ++from)
            {
                weights[layer][from].resize(toSize);
                for (size_t to = 0; to < toSize; ++to)
                {
                    if (!(in >> weights[layer][from][to]))
                    {
                        std::cerr << "Failed to read weight[" << layer << "][" << from << "][" << to << "]\n";
                        throw std::runtime_error("Weight read error at layer " + std::to_string(layer));
                    }
                }
            }
        }

        // === Load biases ===
        // Biases start from layer 1 (no biases for input layer)
        for (size_t layer = 1; layer < numLayers; ++layer)
        {
            size_t layerSize = layerSizes[layer];
            biases[layer].resize(layerSize);

            for (size_t neuron = 0; neuron < layerSize; ++neuron)
            {
                if (!(in >> biases[layer][neuron]))
                {
                    std::cerr << "Failed to read bias[" << layer << "][" << neuron << "]\n";
                    throw std::runtime_error("Bias read error at layer " + std::to_string(layer));
                }
            }
        }

        in.close();

        // Force texture update since network structure changed
        // textureNeedsUpdate = true;
    }

    struct ThreadContext
    {
        std::vector<std::vector<f64>> neurons;
        std::vector<std::vector<f64>> deltas;
        std::vector<std::vector<std::vector<f64>>> weightGradients;
        std::vector<std::vector<f64>> biasGradients;

        void resize(const std::vector<u32> &layerSizes)
        {
            u32 numLayers = layerSizes.size();
            neurons.resize(numLayers);
            deltas.resize(numLayers);
            weightGradients.resize(numLayers - 1);
            biasGradients.resize(numLayers);

            for (u32 i = 0; i < numLayers; i++)
            {
                neurons[i].resize(layerSizes[i]);
                deltas[i].resize(layerSizes[i]);
                if (i < numLayers - 1)
                {
                    weightGradients[i].resize(layerSizes[i]);
                    for (u32 j = 0; j < layerSizes[i]; j++)
                    {
                        weightGradients[i][j].resize(layerSizes[i + 1], 0.0);
                    }
                }
                if (i > 0)
                {
                    biasGradients[i].resize(layerSizes[i], 0.0);
                }
            }
        }

        void resetGradients()
        {
            for (auto &layer : weightGradients)
                for (auto &neuron : layer)
                    std::fill(neuron.begin(), neuron.end(), 0.0);
            
            for (auto &layer : biasGradients)
                std::fill(layer.begin(), layer.end(), 0.0);
        }
    };

    std::vector<f64> forward(const std::vector<f64> &input, std::vector<std::vector<f64>> &neuronStorage)
    {
        if (input.size() != layerSizes[0])
            throw std::runtime_error("Input size doesn't match input layer size.");

        neuronStorage[0] = input;

        for (u32 layer = 1; layer < layerSizes.size(); layer++)
        {
            for (u32 neuron = 0; neuron < layerSizes[layer]; neuron++)
            {
                f64 weightedSum = biases[layer][neuron];

                for (u32 previousNeuron = 0; previousNeuron < layerSizes[layer - 1]; previousNeuron++)
                {
                    weightedSum += neuronStorage[layer - 1][previousNeuron] * weights[layer - 1][previousNeuron][neuron];
                }
                neuronStorage[layer][neuron] = sigmoid(weightedSum);
            }
        }
        return neuronStorage.back();
    }

    // Overload for backward compatibility / single-threaded inference using member storage
    std::vector<f64> forward(const std::vector<f64> &input)
    {
        return forward(input, this->neurons);
    }

    void backwardAccumulate(const std::vector<f64> &target, ThreadContext &ctx)
    {
        u32 outputLayerIndex = layerSizes.size() - 1;

        // calculating the deltas for the output layer
        for (u32 neuron = 0; neuron < layerSizes[outputLayerIndex]; neuron++)
        {
            f64 neuronOutput = ctx.neurons[outputLayerIndex][neuron];
            f64 error = neuronOutput - target[neuron];
            ctx.deltas[outputLayerIndex][neuron] = error * sigmoidDerevative(neuronOutput);
        }

        // calculating the deltas for the hidden layers
        for (i32 layer = outputLayerIndex - 1; layer >= 1; layer--)
        {
            for (u32 neuron = 0; neuron < layerSizes[layer]; neuron++)
            {
                f64 errorSum = 0;

                for (u32 nextNeuron = 0; nextNeuron < layerSizes[layer + 1]; nextNeuron++)
                {
                    errorSum += ctx.deltas[layer + 1][nextNeuron] * weights[layer][neuron][nextNeuron];
                }
                ctx.deltas[layer][neuron] = errorSum * sigmoidDerevative(ctx.neurons[layer][neuron]);
            }
        }

        // Accumulate gradients
        for (u32 layer = 0; layer < weights.size(); layer++)
        {
            for (u32 from = 0; from < layerSizes[layer]; from++)
            {
                for (u32 to = 0; to < layerSizes[layer + 1]; to++)
                {
                    f64 weightGradient = ctx.deltas[layer + 1][to] * ctx.neurons[layer][from];
                    ctx.weightGradients[layer][from][to] += weightGradient;
                }
            }
        }

        for (u32 layer = 1; layer < layerSizes.size(); layer++)
        {
            for (u32 neuron = 0; neuron < layerSizes[layer]; neuron++)
            {
                f64 biasGradient = ctx.deltas[layer][neuron];
                ctx.biasGradients[layer][neuron] += biasGradient;
            }
        }
    }

    void applyGradients(const std::vector<ThreadContext> &contexts, u32 batchSize)
    {
        // Average gradients and update weights/biases
        f64 learningRateAdjusted = learningRate / batchSize;

        for (const auto &ctx : contexts)
        {
            for (u32 layer = 0; layer < weights.size(); layer++)
            {
                for (u32 from = 0; from < layerSizes[layer]; from++)
                {
                    for (u32 to = 0; to < layerSizes[layer + 1]; to++)
                    {
                        weights[layer][from][to] -= learningRateAdjusted * ctx.weightGradients[layer][from][to];
                    }
                }
            }

            for (u32 layer = 1; layer < layerSizes.size(); layer++)
            {
                for (u32 neuron = 0; neuron < layerSizes[layer]; neuron++)
                {
                    biases[layer][neuron] -= learningRateAdjusted * ctx.biasGradients[layer][neuron];
                }
            }
        }
    }

    void trainBatchParallel(const std::vector<std::vector<f64>> &inputs, const std::vector<std::vector<f64>> &targets, u32 startIdx, u32 endIdx, u32 numThreads, std::vector<ThreadContext> &contexts, ThreadPool& pool)
    {
        u32 batchSize = endIdx - startIdx;
        if (batchSize == 0) return;

        // Reset gradients
        for (auto &ctx : contexts)
            ctx.resetGradients();

        std::vector<std::future<void>> futures;
        u32 itemsPerThread = batchSize / numThreads;
        u32 remainder = batchSize % numThreads;

        u32 currentStart = startIdx;

        for (u32 t = 0; t < numThreads; t++)
        {
            u32 count = itemsPerThread + (t < remainder ? 1 : 0);
            if (count == 0) continue; 
            
            u32 currentEnd = currentStart + count;

            futures.emplace_back(pool.enqueue([this, &inputs, &targets, currentStart, currentEnd, t, &contexts]() {
                for (u32 i = currentStart; i < currentEnd; i++)
                {
                    forward(inputs[i], contexts[t].neurons);
                    backwardAccumulate(targets[i], contexts[t]);
                }
            }));

            currentStart = currentEnd;
        }

        for (auto &f : futures)
            f.get();

        applyGradients(contexts, batchSize);
    }

    f64 getErrorParallel(const std::vector<std::vector<f64>> &inputs, const std::vector<std::vector<f64>> &targets, u32 numThreads, std::vector<ThreadContext> &contexts, ThreadPool& pool)
    {
        std::vector<std::future<f64>> futures;
        
        u32 totalItems = inputs.size();
        u32 itemsPerThread = totalItems / numThreads;
        u32 remainder = totalItems % numThreads;
        
        u32 currentStart = 0;

        for (u32 t = 0; t < numThreads; t++)
        {
            u32 count = itemsPerThread + (t < remainder ? 1 : 0);
            if (count == 0) continue;

            u32 currentEnd = currentStart + count;

            futures.emplace_back(pool.enqueue([this, &inputs, &targets, currentStart, currentEnd, t, &contexts]() -> f64 {
                f64 localError = 0.0;
                for (u32 i = currentStart; i < currentEnd; i++)
                {
                    // Use thread-local neurons for forward pass to avoid race conditions on this->neurons
                    forward(inputs[i], contexts[t].neurons);
                    
                    const auto& output = contexts[t].neurons.back();
                    for (int j = 0; j < output.size(); j++)
                    {
                        f64 error = targets[i][j] - output[j];
                        localError += error * error;
                    }
                }
                return localError;
            }));

            currentStart = currentEnd;
        }

        f64 total_error = 0.0;
        for (auto &f : futures)
            total_error += f.get();

        return total_error / (inputs.size() * targets[0].size());
    }

public:
    void train(u32 epochs, const std::vector<std::vector<f64>> &inputs, const std::vector<std::vector<f64>> &targets, u32 batchSize = 32, u32 numThreads = std::thread::hardware_concurrency())
    {
        if (numThreads == 0) numThreads = 4; // Fallback safety
        
        std::cout << "Training with " << numThreads << " threads (ThreadPool), batch size " << batchSize << "...\n";
        
        ThreadPool pool(numThreads);

        // Initialize contexts once
        std::vector<ThreadContext> contexts(numThreads);
        for (auto &ctx : contexts)
            ctx.resize(layerSizes);

        for (u32 epoch = 0; epoch < epochs; epoch++)
        {
            for (u32 i = 0; i < inputs.size(); i += batchSize)
            {
                u32 endIdx = std::min((u32)inputs.size(), i + batchSize);
                trainBatchParallel(inputs, targets, i, endIdx, numThreads, contexts, pool);
            }

            // Use parallel error calculation
            double mse = getErrorParallel(inputs, targets, numThreads, contexts, pool);
            std::cout << "complete " << u32(100 * (f32(epoch) / f32(epochs)))
                      << "% | MSE: " << std::fixed << std::setprecision(6) << mse << std::endl;
        }
    }

    f64 getError(const std::vector<std::vector<f64>> &inputs, const std::vector<std::vector<f64>> &targets)
    {
        // Fallback for single threaded or external calls
        f64 total_error = 0.0;
        for (int i = 0; i < inputs.size(); i++)
        {
            std::vector<f64> output = forward(inputs[i]);
            for (int j = 0; j < output.size(); j++)
            {
                f64 error = targets[i][j] - output[j];
                total_error += error * error;
            }
        }
        return total_error / (inputs.size() * targets[0].size());
    }

    std::vector<f64> predict(const std::vector<f64> &input)
    {
        textureNeedsUpdate=true;
        return forward(input);
    }

    void draw(Vector2 pos, Vector2 size, u32 numHiddenLayersToShow = 3)
    {
        // Check if we need to create or update the texture
        if (networkTexture.id == 0 ||
            textureNeedsUpdate ||
            lastTextureSize.x != size.x ||
            lastTextureSize.y != size.y ||
            lastHiddenLayersToShow != numHiddenLayersToShow)
        {
            // Unload old texture if it exists
            if (networkTexture.id != 0)
            {
                UnloadRenderTexture(networkTexture);
            }

            // Create new render texture
            networkTexture = LoadRenderTexture((int)size.x, (int)size.y);

            // Render the network to the texture
            renderToTexture(size, numHiddenLayersToShow);

            // Update cached values
            textureNeedsUpdate = false;
            lastTextureSize = size;
            lastHiddenLayersToShow = numHiddenLayersToShow;
        }

        // Simply draw the cached texture to the screen
        Rectangle source = {0, 0, (float)networkTexture.texture.width, -(float)networkTexture.texture.height};
        Rectangle dest = {pos.x, pos.y, size.x, size.y};
        DrawTexturePro(networkTexture.texture, source, dest, {0, 0}, 0.0f, WHITE);
    }

    // Force texture update (useful if you want to manually refresh)
    void forceTextureUpdate()
    {
        textureNeedsUpdate = true;
    }
    void print()
    {
        std::cout << "\n=== Network Architecture ===" << std::endl;
        std::cout << "Layers: " << layerSizes.size() << std::endl;
        for (int i = 0; i < layerSizes.size(); i++)
        {
            std::cout << "Layer " << i << ": " << layerSizes[i] << " neurons";
            if (i == 0)
                std::cout << " (Input)";
            else if (i == layerSizes.size() - 1)
                std::cout << " (Output)";
            else
                std::cout << " (Hidden)";
            std::cout << std::endl;
        }

        int total_weights = 0, total_biases = 0;
        for (int i = 0; i < weights.size(); i++)
        {
            total_weights += layerSizes[i] * layerSizes[i + 1];
        }
        for (int i = 1; i < layerSizes.size(); i++)
        {
            total_biases += layerSizes[i];
        }

        std::cout << "Total Parameters: " << total_weights + total_biases
                  << " (" << total_weights << " weights + " << total_biases << " biases)" << std::endl;

        std::cout << "\n=== Weights ===" << std::endl;
        for (int l = 0; l < weights.size(); l++)
        {
            std::cout << "Weights between Layer " << l << " and Layer " << (l + 1) << ":\n";
            for (int i = 0; i < weights[l].size(); i++)
            {
                for (int j = 0; j < weights[l][i].size(); j++)
                {
                    std::cout << weights[l][i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
    }
};
