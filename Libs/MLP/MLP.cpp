/**
 * @file MLP.cpp
 * @brief Implementation of a Multi-Layer Perceptron (MLP) neural network.
 * 
 * This file contains the implementation of a Multi-Layer Perceptron (MLP) neural network.
 * The MLP class provides methods for initializing the network, training it using backpropagation,
 * and making predictions. The network supports different activation functions and can be trained
 * on both Windows and Linux platforms.
 * 
 * @note This implementation uses OpenMP for parallel processing and nlohmann::json for JSON parsing.
 * 
 * @include "MLP.hpp"
 * @include <nlohmann/json.hpp>
 * @include <fstream>
 * @include <sstream>
 * @include <stdexcept>
 * @include <thread>
 * @include <conio.h> (Windows)
 * @include <atomic> (Linux)
 * @include <unistd.h> (Linux)
 * @include <fcntl.h> (Linux)
 * @include <termios.h> (Linux)
 * 
 * @namespace std
 * @namespace chrono
 * 
 * @class MultiLayerPerceptron
 * @brief A class representing a Multi-Layer Perceptron (MLP) neural network.
 * 
 * @tparam T The data type used for the network's weights, biases, and computations (e.g., float, double).
 * 
 * @fn MultiLayerPerceptron<T>::MultiLayerPerceptron()
 * @brief Default constructor. Initializes an empty MLP.
 * 
 * @fn MultiLayerPerceptron<T>::MultiLayerPerceptron(const vector<int> &layersSize)
 * @brief Constructor that initializes the MLP with the specified layer sizes.
 * @param layersSize A vector containing the number of nodes in each layer.
 * 
 * @fn MultiLayerPerceptron<T>::MultiLayerPerceptron(const string &filename)
 * @brief Constructor that initializes the MLP from a JSON file.
 * @param filename The path to the JSON file.
 * 
 * @fn MultiLayerPerceptron<T>::~MultiLayerPerceptron()
 * @brief Destructor. Frees the memory used by the MLP.
 * 
 * @fn void MultiLayerPerceptron<T>::initLayer(const vector<int>& Size)
 * @brief Initializes the layers of the MLP with the specified sizes.
 * @param Size A vector containing the number of nodes in each layer.
 * 
 * @fn vector<T> MultiLayerPerceptron<T>::feedForward(const vector<T> &inputs)
 * @brief Performs a forward pass through the network.
 * @param inputs A vector containing the input values.
 * @return A vector containing the output values.
 * 
 * @fn void MultiLayerPerceptron<T>::backPropagation(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate)
 * @brief Performs backpropagation to train the network.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @param learningRate The learning rate for training.
 * 
 * @fn T MultiLayerPerceptron<T>::updateWeights(const T weight, const T learningRate, const T error, const T input)
 * @brief Updates a weight using the specified learning rate, error, and input.
 * @param weight The current weight.
 * @param learningRate The learning rate.
 * @param error The error value.
 * @param input The input value.
 * @return The updated weight.
 * 
 * @fn T MultiLayerPerceptron<T>::updateBias(const T bias, const T learningRate, const T error)
 * @brief Updates a bias using the specified learning rate and error.
 * @param bias The current bias.
 * @param learningRate The learning rate.
 * @param error The error value.
 * @return The updated bias.
 * 
 * @fn T MultiLayerPerceptron<T>::hiddenLayerError(const T output, const T error, const T weight)
 * @brief Computes the error for a hidden layer node.
 * @param output The output value of the node.
 * @param error The error value.
 * @param weight The weight value.
 * @return The computed error.
 * 
 * @fn T MultiLayerPerceptron<T>::outputLayerError(const T output, const T target)
 * @brief Computes the error for an output layer node.
 * @param output The output value of the node.
 * @param target The target value.
 * @return The computed error.
 * 
 * @fn void MultiLayerPerceptron<T>::resetWeightsBias()
 * @brief Resets the weights and biases of the network.
 * 
 * @fn void MultiLayerPerceptron<T>::train(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate, const bool verbose)
 * @brief Trains the network using the specified inputs, targets, and learning rate.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @param learningRate The learning rate for training.
 * @param verbose If true, prints training progress.
 * 
 * @fn void MultiLayerPerceptron<T>::train(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate, const int iterations, const bool verbose)
 * @brief Trains the network for a specified number of iterations.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @param learningRate The learning rate for training.
 * @param iterations The number of iterations to train for.
 * @param verbose If true, prints training progress.
 * 
 * @fn void MultiLayerPerceptron<T>::typeDeActivation(string type)
 * @brief Sets the activation function type for the network.
 * @param type The activation function type (e.g., "linear", "sigmoid").
 * 
 * @fn T MultiLayerPerceptron<T>::activationDerivative(T x, string type)
 * @brief Computes the derivative of the activation function.
 * @param x The input value.
 * @param type The activation function type.
 * @return The computed derivative.
 * 
 * @fn T MultiLayerPerceptron<T>::calculateAccuracy(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
 * @brief Calculates the accuracy of the network.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @return The accuracy value.
 * 
 * @fn T MultiLayerPerceptron<T>::calculateLoss(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
 * @brief Calculates the loss of the network.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @return The loss value.
 * 
 * @fn bool MultiLayerPerceptron<T>::allOutputsCorrect(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
 * @brief Checks if all outputs are correct.
 * @param inputs A vector of input vectors.
 * @param targets A vector of target output vectors.
 * @return True if all outputs are correct, false otherwise.
 * 
 * @fn void MultiLayerPerceptron<T>::setActivation(const vector<string> &activationTypes)
 * @brief Sets the activation functions for each layer.
 * @param activationTypes A vector of activation function types.
 * 
 * @fn void MultiLayerPerceptron<T>::setLayerWeights(int layerIndex, const vector<vector<T>> &weights)
 * @brief Sets the weights for a specified layer.
 * @param layerIndex The index of the layer.
 * @param weights A vector of weight vectors.
 * 
 * @fn void MultiLayerPerceptron<T>::setLayerBias(int layerIndex, const vector<T> &biases)
 * @brief Sets the biases for a specified layer.
 * @param layerIndex The index of the layer.
 * @param biases A vector of bias values.
 * 
 * @fn void MultiLayerPerceptron<T>::setAccuracy(T accuracy)
 * @brief Sets the accuracy threshold for the network.
 * @param accuracy The accuracy value.
 * 
 * @fn vector<T> MultiLayerPerceptron<T>::predict(const vector<T> &inputs, const rod r)
 * @brief Makes a prediction using the network.
 * @param inputs A vector of input values.
 * @param r The rounding/display option.
 * @return A vector of output values.
 * 
 * @fn vector<vector<T>> MultiLayerPerceptron<T>::predict(const vector<vector<T>> &inputs, const rod r)
 * @brief Makes predictions for multiple inputs using the network.
 * @param inputs A vector of input vectors.
 * @param r The rounding/display option.
 * @return A vector of output vectors.
 * 
 * @fn void MultiLayerPerceptron<T>::export_to_json(const string &filename)
 * @brief Exports the network configuration to a JSON file.
 * @param filename The path to the JSON file.
 * 
 * @fn void MultiLayerPerceptron<T>::import_from_json(const string &filename)
 * @brief Imports the network configuration from a JSON file.
 * @param filename The path to the JSON file.
 * 
 * @fn void MultiLayerPerceptron<T>::display()
 * @brief Displays the network configuration.
 * 
 * @fn MultiLayerPerceptron<T> MultiLayerPerceptron<T>::clone() const
 * @brief Creates a clone of the network.
 * @return A cloned MLP object.
 * 
 * @fn vector<MultiLayerPerceptron<T>> MultiLayerPerceptron<T>::getHistory()
 * @brief Gets the training history of the network.
 * @return A vector of MLP objects representing the training history.
 */

#include "MLP.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>



#ifdef _WIN32
    #include <thread>
    #include <conio.h> // For _kbhit and _getch
    
    bool running = true;
    bool trainning_display = false;

    void checkInput() {
        while (running) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 'q' || ch == 'Q') {
                    running = false;
                }

                if (ch == 'd' || ch == 'D') {
                    trainning_display = !trainning_display;
                }
            }
        }
    }

#elif __linux__
    #include <thread>
    #include <atomic>
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>

    std::atomic<bool> running(true);
    std::atomic<bool> trainning_display(false);

    bool kbhit() {
        struct termios oldt, newt;
        int ch;
        int oldf;

        // Get the current terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        // Disable canonical mode and echo
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        // Set stdin to non-blocking mode
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        // Check for input
        ch = getchar();

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if(ch != EOF) {
            ungetc(ch, stdin);
            return true;
        }

        return false;
    }

    void checkInput() {
        while (running) {
            if (kbhit()) {
                char ch = getchar();
                if (ch == 'q' || ch == 'Q') {
                    running = false;
                }

                if (ch == 'd' || ch == 'D') {
                    trainning_display = !trainning_display;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay to avoid high CPU usage
        }
    }
#endif

template <typename T>
MultiLayerPerceptron<T>::MultiLayerPerceptron()
{
    // * Set seed for random
    srand((unsigned int)time(NULL));
    // * Constructor
    this->layers = vector<vector<Perceptron<T>>>();
}

template <typename T>
MultiLayerPerceptron<T>::MultiLayerPerceptron(const vector<int> &layersSize)
{
    // * Constructor
    this->layers = vector<vector<Perceptron<T>>>();
    initLayer(layersSize);
}

template <typename T>
MultiLayerPerceptron<T>::MultiLayerPerceptron(const string &filename)
{
    // * check file type and select import by type
    if (filename.substr(filename.find_last_of(".") + 1) == "json") {
        import_from_json(filename);
    } else {
        throw std::invalid_argument("File type not supported.");
    }
}

template <typename T>
MultiLayerPerceptron<T>::~MultiLayerPerceptron()
{
    // * Destructor
    // * Free memory
    for (int i = 0; i < layers.size(); ++i) {
        layers[i].clear();
        layers[i].shrink_to_fit();
    }
    layers.clear();
    layers.shrink_to_fit();

    // * clear cache
    activationTypes.clear();
    activationTypes.shrink_to_fit();
}

template <typename T>
void MultiLayerPerceptron<T>::initLayer(const vector<int>& Size)
{
    // * Set seed for random
    srand((unsigned int)time(NULL));
    // * Initialize layers
    for (int i = 0; i < Size.size() - 1; ++i) {
        layers.push_back(vector<Perceptron<T>>());
        for (int j = 0; j < Size[i + 1]; ++j) {
            layers[i].push_back(Perceptron<T>(Size[i]));
        }
    }
}

template <typename T>
vector<T> MultiLayerPerceptron<T>::feedForward(const vector<T> &inputs)
{
    vector<T> outputs = inputs;
    for (int i = 0; i < layers.size(); ++i) {
        vector<T> newOutputs;
        for (int j = 0; j < layers[i].size(); ++j) {
            layers[i][j].typeActivation(activationTypes[i]);
            newOutputs.push_back(layers[i][j].feedForward(outputs));
        }
        outputs = newOutputs;
    }
    return outputs;
}

/**
 * @brief Performs backpropagation on the MultiLayerPerceptron to update weights and biases.
 * 
 * This function takes a set of input vectors and corresponding target vectors, and adjusts the 
 * weights and biases of the neural network using the backpropagation algorithm. The learning rate 
 * determines the step size for weight and bias updates.
 * 
 * @tparam T The data type used for the neural network (e.g., float, double).
 * @param inputs A vector of input vectors, where each input vector corresponds to a single training example.
 * @param targets A vector of target vectors, where each target vector corresponds to the desired output for a single training example.
 * @param learningRate The learning rate used to update the weights and biases.
 * 
 * @throws std::invalid_argument If the size of the inputs vector does not match the size of the targets vector.
 * 
 * The function performs the following steps:
 * 1. Checks that the size of the inputs vector matches the size of the targets vector.
 * 2. For each training example:
 *    a. Performs a forward pass to compute the output of the network.
 *    b. Computes the error at the output layer.
 *    c. Computes the errors for each layer from the output layer to the input layer.
 *    d. Updates the weights and biases for each layer based on the computed errors.
 */
template <typename T>
void MultiLayerPerceptron<T>::backPropagation(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate)
{
    if (inputs.size() != targets.size()) {
        throw std::invalid_argument("Inputs and targets must have the same size.");
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        vector<T> output = feedForward(inputs[i]);

        vector<T> outputError(targets[i].size());
        for (size_t j = 0; j < targets[i].size(); ++j) {
            outputError[j] = targets[i][j] - output[j];
        }

        vector<vector<T>> errors(layers.size());
        vector<vector<T>> layerOutputs(layers.size() + 1);
        layerOutputs[0] = inputs[i];

        // * Forward pass
        for (size_t j = 0; j < layers.size(); ++j) {
            vector<T> newOutputs;
            for (size_t k = 0; k < layers[j].size(); ++k) {
                layers[j][k].typeActivation(activationTypes[j]);
                newOutputs.push_back(layers[j][k].feedForward(layerOutputs[j]));
            }
            layerOutputs[j + 1] = newOutputs;
        }

        // * Compute errors from output layer to input layer
        for (int j = layers.size() - 1; j >= 0; --j) {
            vector<T> layerErrors(layers[j].size());
            if (j == layers.size() - 1) {
                #pragma omp parallel for
                for (size_t k = 0; k < layers[j].size(); ++k) {
                    layerErrors[k] = outputLayerError(layerOutputs[j + 1][k], targets[i][k]);
                }
            } else {
                #pragma omp parallel for
                for (size_t k = 0; k < layers[j].size(); ++k) {
                    T error = 0;
                    for (size_t l = 0; l < layers[j + 1].size(); ++l) {
                        error += hiddenLayerError(layerOutputs[j + 1][l], errors[j + 1][l], layers[j + 1][l].getWeights()[k]);
                    }
                    layerErrors[k] = error;
                }
            }
            errors[j] = layerErrors;
        }

        // cout << "Error: ";
        // for (int j = 0; j < errors.size(); ++j) {
        //     cout << "Layer " << j << ": ";
        //     for (int k = 0; k < errors[j].size(); ++k) {
        //         cout << "Node " << k << ": ";
        //         cout << errors[j][k] << " ";
        //         cout << endl;
        //     }
        // }

        // * Update weights and biases
        for (int j = layers.size() - 1; j >= 0; --j) {
            #pragma omp parallel for
            for (size_t k = 0; k < layers[j].size(); ++k) {
                for (size_t l = 0; l < layers[j][k].getWeights().size(); ++l) {
                    layers[j][k].setWeights(l, updateWeights(layers[j][k].getWeights()[l], learningRate, errors[j][k], layerOutputs[j][l]));
                }
                layers[j][k].setBias(updateBias(layers[j][k].getBias(), learningRate, errors[j][k]));
            }
        }
    }
}

template <typename T>
T MultiLayerPerceptron<T>::updateWeights(const T weight, const T learningRate, const T error, const T input) {
    return weight - (learningRate * error * input);
}

template <typename T>
T MultiLayerPerceptron<T>::updateBias(const T bias, const T learningRate, const T error) {
    return bias - (learningRate * error);
}

template <typename T>
T MultiLayerPerceptron<T>::hiddenLayerError(const T output, const T error, const T weight) {
    return output * error * weight;
}

template <typename T>
T MultiLayerPerceptron<T>::outputLayerError(const T output, const T target) {
    return (output - target);
}

template <typename T>
void MultiLayerPerceptron<T>::resetWeightsBias()
{
    for (size_t i = 0; i < layers.size(); ++i) {
        for (size_t j = 0; j < layers[i].size(); ++j) {
            layers[i][j].resetWeightsBias();
        }
    }
}

template <typename T>
void MultiLayerPerceptron<T>::train(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate, const bool verbose)
{
    // * Start time
    auto start = chrono::high_resolution_clock::now();

    std::thread inputThread(checkInput);

    int iterations = 0;
    T oldloss = 0;
    T loss = 0;
    int lossCount = 0;

    // * Start train
    while (running) {
        backPropagation(inputs, targets, learningRate);
        iterations++;
        if(verbose == true || trainning_display) {
            cout << "\033[1;32mIterations: \033[0m" << iterations << " \033[1;32mAccuracy: \033[0m" << calculateAccuracy(inputs, targets) * 100 << "%" << " \033[1;32mLoss: \033[0m" << calculateLoss(inputs, targets) << endl;
        }

        // * if loss is less than accuracy then break
        if (calculateLoss(inputs, targets) < accuracy * accuracy) {
            // * all outputs correct
            if (allOutputsCorrect(inputs, targets)) {
                running = false;
                break;
            }
        }

        // * if loss is nan, inf then break
        if (isnan(calculateLoss(inputs, targets)) || isinf(calculateLoss(inputs, targets))) {
            running = false;
            break;
        }
    }

    inputThread.join();

    // * End time
    auto end = chrono::high_resolution_clock::now();

    // * report trainning
    cout << endl;
    cout << "\033[1;32mTraining finished!\033[0m" << endl;
    cout << "\033[1;32mIterations: \033[0m" << iterations << endl;
    cout << "\033[1;32mAccuracy: \033[0m" << calculateAccuracy(inputs, targets) * 100 << "%" << endl;
    cout << "\033[1;32mLoss: \033[0m" << calculateLoss(inputs, targets) << endl;
    cout << "\033[1;32mAll outputs correct: \033[0m" << allOutputsCorrect(inputs, targets) << endl;
    cout << "\033[1;32mTime using: \033[0m" << chrono::duration_cast<chrono::seconds>(end - start).count() << "s" << endl;
    cout << endl;
}

template <typename T>
void MultiLayerPerceptron<T>::train(const vector<vector<T>> &inputs, const vector<vector<T>> &targets, const T learningRate, const int iterations, const bool verbose)
{   
    // * Start time 
    auto start = chrono::high_resolution_clock::now();

    // * Start train
    for (int i = 0; i < iterations; ++i) {
        backPropagation(inputs, targets, learningRate);
        if(verbose == true || trainning_display) {
            cout << "\033[1;32mIterations: \033[0m" << iterations << " \033[1;32mAccuracy: \033[0m" << calculateAccuracy(inputs, targets) * 100 << "%" << " \033[1;32mLoss: \033[0m" << calculateLoss(inputs, targets) << endl;
        }

        // * if loss is less than accuracy then break
        if (calculateLoss(inputs, targets) < accuracy) {
            // * all outputs correct
            if (allOutputsCorrect(inputs, targets)) {
                running = false;
                break;
            }
        }

        // * if loss is nan, inf then break
        if (isnan(calculateLoss(inputs, targets)) || isinf(calculateLoss(inputs, targets))) {
            running = false;
            break;
        }
    }

    // * End time
    auto end = chrono::high_resolution_clock::now();

    // * report trainning
    cout << endl;
    cout << "\033[1;32mTraining finished!\033[0m" << endl;
    cout << "\033[1;32mIterations: \033[0m" << iterations << endl;
    cout << "\033[1;32mAccuracy: \033[0m" << calculateAccuracy(inputs, targets) * 100 << "%" << endl;
    cout << "\033[1;32mLoss: \033[0m" << calculateLoss(inputs, targets) << endl;
    cout << "\033[1;32mAll outputs correct: \033[0m" << allOutputsCorrect(inputs, targets) << endl;
    cout << "\033[1;32mTime using: \033[0m" << chrono::duration_cast<chrono::seconds>(end - start).count() << "s" << endl;
    cout << endl;
}

template <typename T>
void MultiLayerPerceptron<T>::typeDeActivation(string type)
{
    // * change type to lower case
    for (int i = 0; i < type.length(); i++)
    {
        type[i] = tolower(type[i]);
    }

    if (!(type == "linear" || 
          type == "sigmoid" ||
          type == "tanh" ||
          type == "relu" ||
          type == "leakyrelu" ||
          type == "softmax" ||
          type == "step"))
    {
        cerr << "\033[1;31mActivation Type Not Found\033[0m" << endl;
        return;
    }

    this->activationType = type;
}

template <typename T>
T MultiLayerPerceptron<T>::activationDerivative(T x, string type)
{

    // * change type to lower case
    for (int i = 0; i < type.length(); i++)
    {
        type[i] = tolower(type[i]);
    }
    
    if (type == "linear") {
        // Derivative of a linear function f(x) = x is 1
        return 1;
    }
    else if (type == "sigmoid") {
        // Compute the sigmoid function value
        T sigmoid = 1 / (1 + exp(-x));
        // Derivative of sigmoid is sigmoid * (1 - sigmoid)
        return sigmoid * (1 - sigmoid);
    }
    else if (type == "tanh") {
        // Compute tanh(x) value
        T tanh_x = tanh(x);
        // Derivative of tanh(x) is 1 - tanh(x)^2
        return 1 - tanh_x * tanh_x;
    }
    else if (type == "relu") {
        // Derivative of ReLU is 1 if x > 0 else 0
        return x > 0 ? 1 : 0;
    }
    else if (type == "leakyrelu") {
        // Derivative of Leaky ReLU is 1 if x > 0 else 0.01
        return x > 0 ? 1 : 0.01;
    }
    else if (type == "softmax") {
        // Derivative of softmax is complex and involves the full vector
        throw std::invalid_argument("Softmax derivative is not applicable for a single value.");
    }
    else if (type == "step") {
        // Derivative of step function is not well-defined, usually 0 everywhere
        return 0; // Optionally, throw an exception or handle it as per your requirements
    }
    else {
        // Handle unknown activation type
        std::cerr << "\033[1;31mActivation Type Not Found\033[0m" << std::endl;
        throw std::invalid_argument("Activation Type Not Found");
    }
}

template <typename T>
T MultiLayerPerceptron<T>::calculateAccuracy(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
{
    int correct = 0;

    for (int i = 0; i < inputs.size(); ++i) {
        vector<T> output = feedForward(inputs[i]);
        for (int j = 0; j < output.size(); ++j) {
            if (abs(targets[i][j] - output[j]) < accuracy) {
                correct++;
            }
        }
    }

    return (T)correct / (inputs.size() * targets[0].size());
}

template <typename T>
T MultiLayerPerceptron<T>::calculateLoss(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
{
    double totalLoss = 0.0;

    for (size_t i = 0; i < inputs.size(); ++i) {
        vector<T> output = feedForward(inputs[i]);
        for (size_t j = 0; j < output.size(); ++j) {
            T error = targets[i][j] - output[j];
            totalLoss += error;
        }
    }

    // * formula : totalLoss / (input(size) * target(size))
    return T(totalLoss / (inputs.size() * targets[0].size())); 
}

template <typename T>
bool MultiLayerPerceptron<T>::allOutputsCorrect(const vector<vector<T>> &inputs, const vector<vector<T>> &targets)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
        vector<T> output = feedForward(inputs[i]);
        for (size_t j = 0; j < output.size(); ++j) {
            if (abs(targets[i][j] - output[j]) > accuracy) {
                return false;
            }
        }
    }
    return true;
}

template <typename T>
void MultiLayerPerceptron<T>::setActivation(const vector<string> &activationTypes)
{ 
    // * check oversize or undersize
    if (activationTypes.size() > layers.size() || activationTypes.size() < layers.size()) {
        throw std::invalid_argument("Number of activation types does not match number of layers");
    }

    this->activationTypes = activationTypes;
    for (int i = 0; i < layers.size(); ++i) {
        for (int j = 0; j < layers[i].size(); ++j) {
            layers[i][j].typeActivation(activationTypes[i]);
        }
    }
}

template <typename T>
void MultiLayerPerceptron<T>::setLayerWeights(int layerIndex, const vector<vector<T>> &weights) {
    if (layerIndex < 0 || layerIndex >= layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }

    if (weights.size() != layers[layerIndex].size()) {
        throw std::invalid_argument("Number of nodes in weights does not match layer size");
    }

    for (int i = 0; i < weights.size(); ++i) {
        if (weights[i].size() != layers[layerIndex][i].getWeights().size()) {
            throw std::invalid_argument("Weights size does not match number of inputs in the layer");
        }

        layers[layerIndex][i].setWeights(weights[i]);
    }
}

template <typename T>
void MultiLayerPerceptron<T>::setLayerBias(int layerIndex, const vector<T> &biases) {
    if (layerIndex < 0 || layerIndex >= layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }

    if (biases.size() != layers[layerIndex].size()) {
        throw std::invalid_argument("Number of biases does not match layer size");
    }

    for (int i = 0; i < biases.size(); ++i) {
        layers[layerIndex][i].setBias(biases[i]);
    }
}

template <typename T>
void MultiLayerPerceptron<T>::setAccuracy(T accuracy)
{
    this->accuracy = accuracy;
}

template <typename T>
vector<T> MultiLayerPerceptron<T>::predict(const vector<T> &inputs, const rod r)
{
    // * display input and outputadd color text
    if (r == DISPLAY || r == R_D) {
        cout << "\033[1;34mInput: \033[0m";
        for (int i = 0; i < inputs.size(); ++i) {
            cout << inputs[i] << " ";
        }
        cout << "\033[1;34mOutput: \033[0m";
        for (int i = 0; i < feedForward(inputs).size(); ++i) {
            if (r == ROUND || r == R_D) {
                cout << round(feedForward(inputs)[i]) << " ";
            } else {
                cout << feedForward(inputs)[i] << " ";
            }
        }
        cout << endl;
    }

    if (r == ROUND || r == R_D) {
        vector<T> outputs;
        for (int i = 0; i < inputs.size(); ++i) {
            outputs.push_back(round(feedForward(inputs)[i]));
        }
        return outputs;
    } else {
        return feedForward(inputs);
    }
}

template <typename T>
vector<vector<T>> MultiLayerPerceptron<T>::predict(const vector<vector<T>> &inputs, const rod r)
{
    vector<vector<T>> outputs;
    for (int i = 0; i < inputs.size(); ++i) {
        outputs.push_back(feedForward(inputs[i]));
    }

    // * display input and output
    if (r == DISPLAY || r == R_D) {
        for (int i = 0; i < inputs.size(); ++i) {
            cout << "\033[1;34mInput: \033[0m";
            for (int j = 0; j < inputs[i].size(); ++j) {
                cout << inputs[i][j] << " ";
            }
            cout << "\033[1;34mOutput: \033[0m";
            for (int j = 0; j < outputs[i].size(); ++j) {
                if (r == ROUND || r == R_D) {
                    cout << round(outputs[i][j]) << " ";
                } else {
                    cout << outputs[i][j] << " ";
                }
            }
            cout << endl;
        }
    }

    if (r == ROUND || r == R_D) {
        vector<vector<T>> roundedOutputs;
        for (int i = 0; i < outputs.size(); ++i) {
            vector<T> roundedOutput;
            for (int j = 0; j < outputs[i].size(); ++j) {
                roundedOutput.push_back(round(outputs[i][j]));
            }
            roundedOutputs.push_back(roundedOutput);
        }
        return roundedOutputs;
    } else {
        return outputs;
    }
}

template <typename T>
void MultiLayerPerceptron<T>::export_to_json(const string &filename){
    // * Write the JSON file
    nlohmann::json json;
    json["layers"] = nlohmann::json::array();
    for (int i = 0; i < layers.size(); ++i) {
        json["layers"].push_back(nlohmann::json::object());
        json["layers"][i]["activation"] = activationTypes[i];
        json["layers"][i]["nodes"] = nlohmann::json::array();
        for (int j = 0; j < layers[i].size(); ++j) {
            json["layers"][i]["nodes"].push_back(nlohmann::json::object());
            json["layers"][i]["nodes"][j]["weights"] = layers[i][j].getWeights();
            json["layers"][i]["nodes"][j]["bias"] = layers[i][j].getBias();
        }
    }

    ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing");
    }

    file << json.dump(4);
    file.close();
}

template <typename T>
void MultiLayerPerceptron<T>::import_from_json(const string &filename)
{
    // * Read the JSON file
    /*
        we need to config like this
        vector<int> layersSize = {8*8, 64, 32, 1};
        MultiLayerPerceptron<double> mlp(layersSize);

        mlp.setActivation({"sigmoid", "sigmoid", "linear"});
        mlp.setAccuracy(0.01);
        // mlp.display();

        double learningRate = 0.01;
    */

    ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading");
    }

    nlohmann::json json;
    file >> json;
    file.close();

    // * Read the layers
    vector<int> layersSize;
    vector<string> activationTypes;
    vector<vector<vector<T>>> weights;
    vector<vector<T>> biases;

    layersSize.push_back(json["layers"][0]["nodes"][0]["weights"].size());
    for (int i = 0; i < json["layers"].size(); ++i) {
        layersSize.push_back(json["layers"][i]["nodes"].size());
        activationTypes.push_back(json["layers"][i]["activation"]);
        weights.push_back(vector<vector<T>>());
        biases.push_back(vector<T>());

        for (int j = 0; j < json["layers"][i]["nodes"].size(); ++j) {
            weights[i].push_back(json["layers"][i]["nodes"][j]["weights"].get<vector<T>>());
            biases[i].push_back(json["layers"][i]["nodes"][j]["bias"].get<T>());
        }
    }

    // Initialize the MLP with the read configuration
    initLayer(layersSize);
    setActivation(activationTypes);

    // Set the weights and biases for each layer
    for (int i = 0; i < layersSize.size() - 1; ++i) {
        setLayerWeights(i, weights[i]);
        setLayerBias(i, biases[i]);
    }
}

template <typename T>
void MultiLayerPerceptron<T>::display()
{
    // * Display layers
    for (int i = 0; i < layers.size(); ++i) {
        cout << "\033[1;34mLayer: " << i << " -> " << "activation: " << activationTypes[i] << "\033[0m" << endl;
        cout << "\033[1;32mBase accuracy: " << accuracy << "\033[0m" << endl;
        for (int j = 0; j < layers[i].size(); ++j) {
            cout << "\033[1;33mNode: " << j << " \033[0m";
            // * display weights and bias
            cout << "\033[1;36mW: \033[0m";
            for (int k = 0; k < layers[i][j].getWeights().size(); ++k) {
                cout << layers[i][j].getWeights()[k] << " ";
            }
            cout << "\033[1;36mB: \033[0m" << layers[i][j].getBias() << endl;
        }
    }
}

template <typename T>
inline void MultiLayerPerceptron<T>::clearModel()
{
    // * Free memory
    for (int i = 0; i < layers.size(); ++i) {
        layers[i].clear();
        layers[i].shrink_to_fit();
    }
    layers.clear();
    layers.shrink_to_fit();

    // * clear cache
    activationTypes.clear();
    activationTypes.shrink_to_fit();
}

template <typename T>
MultiLayerPerceptron<T> MultiLayerPerceptron<T>::clone() const
{
    return MultiLayerPerceptron<T>(*this);
}

template <typename T>
vector<MultiLayerPerceptron<T>> MultiLayerPerceptron<T>::getHistory()
{
    return vector<MultiLayerPerceptron<T>>(this->history);
}

template class MultiLayerPerceptron<float>;
template class MultiLayerPerceptron<double>;