#pragma once
#include "types.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

// Define OpenGL function pointers
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#undef near
#undef far
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextA
#endif

#include <raylib.h>
#include <rlgl.h>

// Constants
#define GL_COMPUTE_SHADER 0x91B9
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_FLOAT 0x1406
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#define GL_BUFFER_SIZE 0x8764

// Types
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef unsigned int GLbitfield;

#ifndef APIENTRY
#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

// Function Pointers
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY *PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY *PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY *PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRY *PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY *PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (APIENTRY *PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
typedef void (APIENTRY *PFNGLDISPATCHCOMPUTEPROC) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (APIENTRY *PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRY *PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
typedef void (APIENTRY *PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);

// Global Function Pointers
static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLBINDBUFFERBASEPROC glBindBufferBase = NULL;
static PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = NULL;
static PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData = NULL;
static PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = NULL;
static PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = NULL;
static PFNGLMEMORYBARRIERPROC glMemoryBarrier = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM1UIPROC glUniform1ui = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;

static void loadGL() {
    #ifdef _WIN32
    HMODULE hOpenGL = LoadLibraryA("opengl32.dll");
    if (!hOpenGL) {
        std::cerr << "ERROR: Failed to load opengl32.dll" << std::endl;
        return;
    }

    typedef PROC (APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR);
    PFNWGLGETPROCADDRESSPROC wglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(hOpenGL, "wglGetProcAddress");

    if (!wglGetProcAddress) {
        std::cerr << "ERROR: Failed to get wglGetProcAddress" << std::endl;
        return;
    }

    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase");
    glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetBufferParameteriv");
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)wglGetProcAddress("glCopyBufferSubData");
    glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)wglGetProcAddress("glGetBufferSubData");
    glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute");
    glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)wglGetProcAddress("glMemoryBarrier");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1ui = (PFNGLUNIFORM1UIPROC)wglGetProcAddress("glUniform1ui");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    
    if(!glDispatchCompute) std::cerr << "ERROR: Failed to load Compute Shader functions! Update Drivers?" << std::endl;
    #endif
}

class NeuralNetworkGPU
{
private:
    std::vector<u32> layerSizes;
    u32 numLayers;
    u32 maxBatchSize;
    u32 totalNeurons;

    // OpenGL Object IDs
    u32 computeProgramForward;
    u32 computeProgramBackwardOutput;
    u32 computeProgramBackwardHidden;
    u32 computeProgramUpdate;
    u32 computeProgramLoadInput;

    // SSBOs
    u32 ssboNeurons;
    u32 ssboWeights;
    u32 ssboBiases;
    u32 ssboDeltas;
    u32 ssboInputs;
    u32 ssboTargets;
    u32 ssboErrors; // New buffer for error storage

    struct LayerInfo {
        u32 size;
        u32 offsetNeurons;
        u32 offsetWeights;
        u32 offsetBiases;
        u32 offsetDeltas;
    };
    std::vector<LayerInfo> layerInfos;
    u32 maxLayerSize = 0;

    // Helper to compile shader
    u32 createComputeShader(const char* source) {
        u32 shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "Compute Shader Compilation Failed:\n" << infoLog << std::endl;
        }

        u32 program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Compute Shader Linking Failed:\n" << infoLog << std::endl;
        }

        glDeleteShader(shader);
        return program;
    }

    void compileShaders() {
        // --- LOAD INPUT SHADER ---
        // Copies data from Inputs buffer to Neurons buffer (Layer 0) for the batch
        const char* srcLoadInput = R"(
            #version 430
            layout(local_size_x = 64, local_size_y = 1) in;
            
            layout(std430, binding = 0) buffer Neurons { float neurons[]; };
            layout(std430, binding = 4) buffer Inputs { float inputs[]; };
            
            uniform uint inputSize;
            uniform uint inputOffset; // Start index in inputs buffer (global index)
            uniform uint totalNeurons; // Stride for neurons buffer (batch stride)
            
            void main() {
                uint n = gl_GlobalInvocationID.x;
                uint b = gl_GlobalInvocationID.y;
                
                if(n >= inputSize) return;
                
                // Input is flat: [Sample0] [Sample1] ...
                // Index in inputs = inputOffset + b * inputSize + n
                // Note: inputOffset passed from CPU is i * inputSize
                float val = inputs[inputOffset + b * inputSize + n];
                
                // Neurons is [Batch0_AllLayers] [Batch1_AllLayers] ...
                // Layer 0 is at offset 0 of each batch block.
                // Index in neurons = b * totalNeurons + n
                neurons[b * totalNeurons + n] = val;
            }
        )";
        computeProgramLoadInput = createComputeShader(srcLoadInput);

        // --- FORWARD SHADER ---
        // Dispatch(LayerSize, BatchSize, 1)
        const char* srcForward = R"(
            #version 430
            layout(local_size_x = 64, local_size_y = 1) in;
            
            layout(std430, binding = 0) buffer Neurons { float neurons[]; };
            layout(std430, binding = 1) buffer Weights { float weights[]; };
            layout(std430, binding = 2) buffer Biases { float biases[]; };
            
            uniform uint prevLayerSize;
            uniform uint currLayerSize;
            uniform uint prevLayerOffset;
            uniform uint currLayerOffset;
            uniform uint weightOffset;
            uniform uint biasOffset;
            uniform uint totalNeurons; // Stride for batching
            
            float sigmoid(float x) { return 1.0 / (1.0 + exp(-x)); }
            
            void main() {
                uint n = gl_GlobalInvocationID.x; // Neuron Index
                uint b = gl_GlobalInvocationID.y; // Batch Index
                
                if(n >= currLayerSize) return;
                
                // Calculate offsets for this batch
                uint batchOffset = b * totalNeurons;
                
                float sum = biases[biasOffset + n];
                
                for(uint i = 0; i < prevLayerSize; ++i) {
                    // Weight: [prev][curr] -> flat index
                    float w = weights[weightOffset + i * currLayerSize + n]; 
                    float a = neurons[batchOffset + prevLayerOffset + i];
                    sum += w * a;
                }
                
                neurons[batchOffset + currLayerOffset + n] = sigmoid(sum);
            }
        )";
        computeProgramForward = createComputeShader(srcForward);

        // --- BACKWARD OUTPUT SHADER ---
        // Dispatch(LayerSize, BatchSize, 1)
        const char* srcBackwardOutput = R"(
            #version 430
            layout(local_size_x = 64, local_size_y = 1) in;
            
            layout(std430, binding = 0) buffer Neurons { float neurons[]; };
            layout(std430, binding = 3) buffer Deltas { float deltas[]; };
            layout(std430, binding = 5) buffer Targets { float targets[]; };
            layout(std430, binding = 6) buffer Errors { float errors[]; };
            
            uniform uint layerSize;
            uniform uint layerOffset;
            uniform uint targetOffset; // Start of targets for this batch
            uniform uint totalNeurons;
            uniform uint targetStride; // Size of one target vector
            
            void main() {
                uint n = gl_GlobalInvocationID.x;
                uint b = gl_GlobalInvocationID.y;
                
                if(n >= layerSize) return;
                
                uint batchOffset = b * totalNeurons;
                uint tOffset = targetOffset + b * targetStride;
                
                float a = neurons[batchOffset + layerOffset + n];
                float t = targets[tOffset + n];
                
                // Error: 0.5 * (a - t)^2
                float err = 0.5 * (a - t) * (a - t);
                // Store error. We need atomic add if we want total error, but for now let's just store per-neuron error
                // and sum it up on CPU or use a reduction shader.
                // For simplicity, we'll store it in a buffer of size [BatchSize * OutputSize] and sum on CPU for display.
                // Index: b * layerSize + n
                errors[b * layerSize + n] = err;
                
                deltas[batchOffset + layerOffset + n] = (a - t) * a * (1.0 - a);
            }
        )";
        computeProgramBackwardOutput = createComputeShader(srcBackwardOutput);

        // --- BACKWARD HIDDEN SHADER ---
        // Dispatch(LayerSize, BatchSize, 1)
        const char* srcBackwardHidden = R"(
            #version 430
            layout(local_size_x = 64, local_size_y = 1) in;
            
            layout(std430, binding = 0) buffer Neurons { float neurons[]; };
            layout(std430, binding = 1) buffer Weights { float weights[]; };
            layout(std430, binding = 3) buffer Deltas { float deltas[]; };
            
            uniform uint currLayerSize;
            uniform uint nextLayerSize;
            uniform uint currLayerOffset;
            uniform uint nextLayerOffset;
            uniform uint weightOffset;
            uniform uint totalNeurons;
            
            void main() {
                uint n = gl_GlobalInvocationID.x;
                uint b = gl_GlobalInvocationID.y;
                
                if(n >= currLayerSize) return;
                
                uint batchOffset = b * totalNeurons;
                
                float sum = 0.0;
                for(uint i = 0; i < nextLayerSize; ++i) {
                    // Weight from curr[n] to next[i]
                    float w = weights[weightOffset + n * nextLayerSize + i];
                    float d = deltas[batchOffset + nextLayerOffset + i];
                    sum += w * d;
                }
                
                float a = neurons[batchOffset + currLayerOffset + n];
                deltas[batchOffset + currLayerOffset + n] = sum * a * (1.0 - a);
            }
        )";
        computeProgramBackwardHidden = createComputeShader(srcBackwardHidden);

        // --- UPDATE SHADER ---
        // Dispatch(CurrLayerSize, 1, 1) - We parallelize over neurons in current layer
        // Each thread loops over the batch to accumulate gradients
        const char* srcUpdate = R"(
            #version 430
            layout(local_size_x = 64, local_size_y = 1) in;
            
            layout(std430, binding = 0) buffer Neurons { float neurons[]; };
            layout(std430, binding = 1) buffer Weights { float weights[]; };
            layout(std430, binding = 2) buffer Biases { float biases[]; };
            layout(std430, binding = 3) buffer Deltas { float deltas[]; };
            
            uniform uint prevLayerSize;
            uniform uint currLayerSize;
            uniform uint prevLayerOffset;
            uniform uint currLayerOffset;
            uniform uint weightOffset;
            uniform uint biasOffset;
            uniform float learningRate;
            uniform uint batchSize;
            uniform uint totalNeurons;
            
            void main() {
                uint n = gl_GlobalInvocationID.x; // Neuron in current layer (to)
                if(n >= currLayerSize) return;
                
                // 1. Accumulate Bias Gradient
                float biasGrad = 0.0;
                for(uint b = 0; b < batchSize; ++b) {
                    biasGrad += deltas[b * totalNeurons + currLayerOffset + n];
                }
                
                // Update Bias
                biases[biasOffset + n] -= (learningRate / float(batchSize)) * biasGrad;
                
                // 2. Update Weights connecting to this neuron
                for(uint i = 0; i < prevLayerSize; ++i) {
                    float weightGrad = 0.0;
                    for(uint b = 0; b < batchSize; ++b) {
                        float d = deltas[b * totalNeurons + currLayerOffset + n];
                        float a = neurons[b * totalNeurons + prevLayerOffset + i];
                        weightGrad += d * a;
                    }
                    
                    uint wIndex = weightOffset + i * currLayerSize + n;
                    weights[wIndex] -= (learningRate / float(batchSize)) * weightGrad;
                }
            }
        )";
        computeProgramUpdate = createComputeShader(srcUpdate);
    }

public:
    NeuralNetworkGPU(const std::vector<u32>& sizes) : layerSizes(sizes) {
        numLayers = sizes.size();
        for(u32 s : sizes) if(s > maxLayerSize) maxLayerSize = s;
        
        // Calculate offsets
        u32 currentNeuronOffset = 0;
        u32 currentWeightOffset = 0;
        u32 currentBiasOffset = 0; 
        
        layerInfos.resize(numLayers);
        for(u32 i=0; i<numLayers; ++i) {
            layerInfos[i].size = sizes[i];
            layerInfos[i].offsetNeurons = currentNeuronOffset;
            layerInfos[i].offsetDeltas = currentNeuronOffset;
            
            currentNeuronOffset += sizes[i];
            
            if (i < numLayers - 1) {
                layerInfos[i].offsetWeights = currentWeightOffset;
                currentWeightOffset += sizes[i] * sizes[i+1];
            }
            
            if (i > 0) {
                layerInfos[i].offsetBiases = currentBiasOffset;
                currentBiasOffset += sizes[i];
            }
        }
        totalNeurons = currentNeuronOffset;
    }

    void init(u32 batchSize = 64) {
        loadGL();
        maxBatchSize = batchSize;
        
        // 1. Create SSBOs
        
        // Neurons & Deltas - Need one copy per sample in batch
        u32 bufferSizeNeurons = totalNeurons * maxBatchSize;
        
        // Weights & Biases - Shared
        u32 totalWeights = 0;
        for(size_t i=0; i<layerSizes.size()-1; ++i) totalWeights += layerSizes[i] * layerSizes[i+1];
        
        u32 totalBiases = 0;
        for(size_t i=1; i<layerSizes.size(); ++i) totalBiases += layerSizes[i];

        // Create Buffers
        u32 buffers[7];
        glGenBuffers(7, buffers);
        ssboNeurons = buffers[0];
        ssboWeights = buffers[1];
        ssboBiases = buffers[2];
        ssboDeltas = buffers[3];
        ssboInputs = buffers[4];
        ssboTargets = buffers[5];
        ssboErrors = buffers[6];

        // Allocate memory
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNeurons);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeNeurons * sizeof(f32), NULL, GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDeltas);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeNeurons * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

        // Error buffer: BatchSize * OutputLayerSize
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboErrors);
        glBufferData(GL_SHADER_STORAGE_BUFFER, maxBatchSize * layerSizes.back() * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

        // Initialize Weights and Biases
        std::vector<f32> initialWeights(totalWeights);
        std::vector<f32> initialBiases(totalBiases);
        
        for(auto& w : initialWeights) w = (float)((rand() % 2000) / 1000.0f - 1.0f) * 0.1f;
        for(auto& b : initialBiases) b = 0.0f;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboWeights);
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalWeights * sizeof(f32), initialWeights.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiases);
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalBiases * sizeof(f32), initialBiases.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        compileShaders();
    }

    void uploadTrainingData(const std::vector<std::vector<f64>>& inputs, const std::vector<std::vector<f64>>& targets) {
        size_t numSamples = inputs.size();
        size_t inputSize = inputs[0].size();
        size_t targetSize = targets[0].size();

        std::vector<f32> flatInputs;
        std::vector<f32> flatTargets;
        flatInputs.reserve(numSamples * inputSize);
        flatTargets.reserve(numSamples * targetSize);

        for(const auto& v : inputs) for(auto d : v) flatInputs.push_back((f32)d);
        for(const auto& v : targets) for(auto d : v) flatTargets.push_back((f32)d);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboInputs);
        glBufferData(GL_SHADER_STORAGE_BUFFER, flatInputs.size() * sizeof(f32), flatInputs.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTargets);
        glBufferData(GL_SHADER_STORAGE_BUFFER, flatTargets.size() * sizeof(f32), flatTargets.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void train(u32 epochs, u32 batchSize, f32 learningRate) {
        if (batchSize > maxBatchSize) {
            std::cerr << "Error: Batch size " << batchSize << " exceeds max initialized batch size " << maxBatchSize << std::endl;
            return;
        }

        // Bind buffers
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNeurons);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboWeights);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboBiases);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboDeltas);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboInputs);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboTargets);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboErrors);

        GLint inputBufferSize;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboInputs);
        glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &inputBufferSize);
        u32 numSamples = inputBufferSize / (sizeof(f32) * layerSizes[0]);
        
        std::cout << "GPU Training Started: " << epochs << " epochs, " << numSamples << " samples, batch size " << batchSize << std::endl;

        std::vector<f32> errorBuffer(batchSize * layerSizes.back());

        for(u32 epoch = 0; epoch < epochs; ++epoch) {
            f64 totalError = 0.0;
            u32 batchesProcessed = 0;

            for(u32 i = 0; i < numSamples; i += batchSize) {
                u32 currentBatchSize = std::min(batchSize, numSamples - i);
                
                // 1. Load Input Batch (Compute Shader)
                glUseProgram(computeProgramLoadInput);
                glUniform1ui(glGetUniformLocation(computeProgramLoadInput, "inputSize"), layerSizes[0]);
                glUniform1ui(glGetUniformLocation(computeProgramLoadInput, "inputOffset"), i * layerSizes[0]); // Offset in floats
                glUniform1ui(glGetUniformLocation(computeProgramLoadInput, "totalNeurons"), totalNeurons);
                
                // Dispatch (InputSize, BatchSize)
                glDispatchCompute((layerSizes[0] + 63) / 64, currentBatchSize, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                
                // 2. Forward Pass
                for(u32 l = 1; l < numLayers; ++l) {
                    glUseProgram(computeProgramForward);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "prevLayerSize"), layerSizes[l-1]);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "currLayerSize"), layerSizes[l]);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "prevLayerOffset"), layerInfos[l-1].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "currLayerOffset"), layerInfos[l].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "weightOffset"), layerInfos[l-1].offsetWeights);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "biasOffset"), layerInfos[l].offsetBiases);
                    glUniform1ui(glGetUniformLocation(computeProgramForward, "totalNeurons"), totalNeurons);
                    
                    // Dispatch (LayerSize, BatchSize)
                    glDispatchCompute((layerSizes[l] + 63) / 64, currentBatchSize, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                // 3. Backward Output
                glUseProgram(computeProgramBackwardOutput);
                glUniform1ui(glGetUniformLocation(computeProgramBackwardOutput, "layerSize"), layerSizes.back());
                glUniform1ui(glGetUniformLocation(computeProgramBackwardOutput, "layerOffset"), layerInfos.back().offsetNeurons);
                glUniform1ui(glGetUniformLocation(computeProgramBackwardOutput, "targetOffset"), i * layerSizes.back()); // Start of targets for this batch
                glUniform1ui(glGetUniformLocation(computeProgramBackwardOutput, "totalNeurons"), totalNeurons);
                glUniform1ui(glGetUniformLocation(computeProgramBackwardOutput, "targetStride"), layerSizes.back());
                
                glDispatchCompute((layerSizes.back() + 63) / 64, currentBatchSize, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                // 4. Backward Hidden
                for(i32 l = numLayers - 2; l >= 1; --l) {
                    glUseProgram(computeProgramBackwardHidden);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "currLayerSize"), layerSizes[l]);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "nextLayerSize"), layerSizes[l+1]);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "currLayerOffset"), layerInfos[l].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "nextLayerOffset"), layerInfos[l+1].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "weightOffset"), layerInfos[l].offsetWeights);
                    glUniform1ui(glGetUniformLocation(computeProgramBackwardHidden, "totalNeurons"), totalNeurons);
                    
                    glDispatchCompute((layerSizes[l] + 63) / 64, currentBatchSize, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                // 5. Update Weights & Biases (Accumulate gradients over batch)
                for(u32 l = 1; l < numLayers; ++l) {
                    glUseProgram(computeProgramUpdate);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "prevLayerSize"), layerSizes[l-1]);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "currLayerSize"), layerSizes[l]);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "prevLayerOffset"), layerInfos[l-1].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "currLayerOffset"), layerInfos[l].offsetNeurons);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "weightOffset"), layerInfos[l-1].offsetWeights);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "biasOffset"), layerInfos[l].offsetBiases);
                    glUniform1f(glGetUniformLocation(computeProgramUpdate, "learningRate"), learningRate);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "batchSize"), currentBatchSize);
                    glUniform1ui(glGetUniformLocation(computeProgramUpdate, "totalNeurons"), totalNeurons);
                    
                    // Dispatch (CurrLayerSize, 1) - Each thread loops over batch
                    glDispatchCompute((layerSizes[l] + 63) / 64, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                // Calculate Error periodically (every 100 batches) to avoid stalling pipeline too often
                if (batchesProcessed % 100 == 0) {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboErrors);
                    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, currentBatchSize * layerSizes.back() * sizeof(f32), errorBuffer.data());
                    
                    for(u32 b=0; b<currentBatchSize; ++b) {
                        for(u32 n=0; n<layerSizes.back(); ++n) {
                            totalError += errorBuffer[b * layerSizes.back() + n];
                        }
                    }
                    
                    // Progress Bar
                    float progress = (float)i / (float)numSamples;
                    int barWidth = 50;
                    std::cout << "[";
                    int pos = barWidth * progress;
                    for (int p = 0; p < barWidth; ++p) {
                        if (p < pos) std::cout << "=";
                        else if (p == pos) std::cout << ">";
                        else std::cout << " ";
                    }
                    std::cout << "] " << int(progress * 100.0) << " % | Loss: " << (totalError / (batchesProcessed * batchSize + 1)) << "\r";
                    std::cout.flush();
                }
                batchesProcessed++;
            }
            std::cout << std::endl << "Epoch " << epoch + 1 << " complete. Avg Loss: " << (totalError / numSamples) << std::endl;
        }
    }

    void saveModel(const std::string& filename) {
        std::cout << "Saving model to " << filename << "..." << std::endl;
        
        // 1. Download Weights and Biases from GPU
        u32 totalWeights = 0;
        for(size_t i=0; i<layerSizes.size()-1; ++i) totalWeights += layerSizes[i] * layerSizes[i+1];
        
        u32 totalBiases = 0;
        for(size_t i=1; i<layerSizes.size(); ++i) totalBiases += layerSizes[i];

        std::vector<f32> weights(totalWeights);
        std::vector<f32> biases(totalBiases);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboWeights);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalWeights * sizeof(f32), weights.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiases);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalBiases * sizeof(f32), biases.data());

        // 2. Write to file (Format must match NeuralNetwork::loadFromFile)
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Failed to open file for saving." << std::endl;
            return;
        }

        out << std::setprecision(16);

        // Save layer sizes
        out << layerSizes.size() << '\n';
        for (u32 size : layerSizes)
            out << size << ' ';
        out << '\n';

        // Save weights
        // GPU Layout: Flat array. Layer 0 weights, then Layer 1 weights...
        // Layer i weights: [prevLayerSize * currLayerSize]
        // Order: w[0][0], w[0][1]... (row-major? No, shader uses: weightOffset + i * currLayerSize + n)
        // i is prev neuron, n is curr neuron.
        // So it is: w[prev=0][curr=0], w[prev=0][curr=1], ... w[prev=0][curr=N], w[prev=1][curr=0]...
        // This matches standard row-major if rows are "prev neurons".
        
        u32 wOffset = 0;
        for (size_t l = 0; l < layerSizes.size() - 1; ++l) {
            u32 prevSize = layerSizes[l];
            u32 currSize = layerSizes[l+1];
            
            // CPU expects:
            // for (from) { for (to) { out << weight << ' '; } out << '\n'; }
            
            for (u32 from = 0; from < prevSize; ++from) {
                for (u32 to = 0; to < currSize; ++to) {
                    // Index in GPU buffer:
                    // wIndex = weightOffset + i * currLayerSize + n;
                    // i = from, n = to
                    out << weights[wOffset + from * currSize + to] << ' ';
                }
                out << '\n';
            }
            wOffset += prevSize * currSize;
        }

        // Save biases
        // GPU Layout: Flat array. Layer 1 biases, then Layer 2...
        u32 bOffset = 0;
        for (size_t l = 1; l < layerSizes.size(); ++l) {
            u32 currSize = layerSizes[l];
            for (u32 n = 0; n < currSize; ++n) {
                out << biases[bOffset + n] << ' ';
            }
            out << '\n';
            bOffset += currSize;
        }

        out.close();
        std::cout << "Model saved successfully." << std::endl;
    }
};
