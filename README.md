# Neural Network Enhanced Version

A high-performance C++ neural network implementation featuring both CPU and GPU training, real-time visualization, and interactive digit recognition. This project is specifically optimized for the MNIST handwritten digit dataset.

## Features

- **Dual Backend Support**:
    - **CPU**: Multi-threaded training using a custom-built ThreadPool for efficient parallelization across all available cores.
    - **GPU**: High-speed training powered by **OpenGL Compute Shaders**, allowing for massive batch processing and significant performance gains.
- **Real-time Visualization**: Dynamic visualization of the network architecture, including neuron activations and connection weights, using the **Raylib** graphics library.
- **Interactive Drawing Mode**: Draw digits directly onto a canvas and watch the neural network predict them in real-time.
- **Data Augmentation**: Built-in support for image distortion and augmentation to improve model robustness and accuracy.
- **MNIST Integration**: Seamlessly loads and processes the MNIST dataset (60,000 training images, 10,000 test images).
- **Flexible Architecture**: Custom topology support (e.g., 784 -> 128 -> 64 -> 10) with Xavier initialization and Sigmoid activation.
- **Model Persistence**: Save and load trained models (weights and biases) to `.txt` files for later use.

## Project Structure

- `src/`: Main source files for CPU, GPU, and Drawing modes.
- `include/`: Core headers including `NeuralNetwork.hpp` and `NeuralNetworkGPU.hpp`.
- `resources/`: Contains the MNIST dataset, fonts, and saved model weights.
- `lib/`: Pre-compiled Raylib static libraries for Windows.

## Getting Started

### Prerequisites

- A C++ compiler (G++ recommended).
- OpenGL 4.3+ capable GPU (for the GPU version).
- Bash environment (for running the build scripts).

### Building and Running

The project includes several build scripts for different modes:

1. **Standard CPU Version (Training + Visualization)**:
   ```bash
   ./build_run.sh
   ```

2. **GPU Accelerated Version (Headless Training)**:
   ```bash
   ./build_run_gpu.sh
   ```

3. **Interactive Drawing Mode**:
   ```bash
   ./build_run_draw.sh
   ```

## Technical Details

### CPU Implementation
The CPU engine uses a **ThreadPool** pattern to distribute the workload of forward and backward passes during batch training. It implements **Backpropagation** with **Xavier initialization** to ensure stable gradients from the start.

### GPU Implementation
The GPU engine utilizes **OpenGL Compute Shaders** (GLSL) to offload heavy matrix operations. 
- **SSBOs (Shader Storage Buffer Objects)** are used to store neurons, weights, biases, and deltas.
- **Compute Shaders** handle the forward pass, backward pass, and weight updates in parallel across thousands of GPU threads.

### Visualization
Powered by **Raylib**, the visualization provides:
- Brighter blue lines for positive weights and red for negative weights.
- Real-time neuron intensity based on activation levels.
- Batch progress bars and Loss/MSE tracking.

## Dataset
The project uses the **MNIST dataset**, which must be placed in the `resources/` directory:
- `train-images-idx3-ubyte`
- `train-labels-idx1-ubyte`
- `t10k-images-idx3-ubyte`
- `t10k-labels-idx1-ubyte`

## License
This project is open-source. Feel free to use and modify it for your own learning or projects!
