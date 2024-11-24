

/**
 * @file Perceptron.cpp
 * @brief Implementation of the Perceptron class for a simple neural network.
 * 
 * This file contains the implementation of a Perceptron class template, which is a fundamental building block for neural networks.
 * The Perceptron class supports various activation functions and provides methods for initializing, training, and using the perceptron.
 * 
 * @tparam T The data type for the perceptron weights, inputs, and outputs (e.g., float, double).
 */

#include "Perceptron.hpp"
#include <cmath>
#include <string>
#include <chrono>

using namespace std;
using std::vector;

/**
 * @brief Default constructor for the Perceptron class.
 * 
 * Initializes the perceptron with default values.
 */
template <typename T>
Perceptron<T>::Perceptron()
{
    // * Constructor
    this->bias = 0;
    this->output = 0;
    this->activationType = "sigmoid";
}

/**
 * @brief Constructor for the Perceptron class with a specified input size.
 * 
 * @param inputSize The number of inputs to the perceptron.
 */
template <typename T>
Perceptron<T>::Perceptron(int inputSize)
{
    init(inputSize);
}

/**
 * @brief Destructor for the Perceptron class.
 * 
 * Frees any allocated memory and resets the perceptron state.
 */
template <typename T>
Perceptron<T>::~Perceptron()
{
    // * Destructor
    // * Free memory
    weights.clear();
    weights.shrink_to_fit();

    // * Reset bias
    bias = 0;
    output = 0;
    activationType = "sigmoid";
}

/**
 * @brief Initializes the perceptron with random weights and bias.
 * 
 * @param inputSize The number of inputs to the perceptron.
 */
template <typename T>
void Perceptron<T>::init(int inputSize)
{
    weights.resize(inputSize);
    for (int i = 0; i < inputSize; ++i) {
        weights[i] = ((T)rand() / RAND_MAX) * 2 - 1; // Random values between -1 and 1
    }
    bias = ((T)rand() / RAND_MAX) * 2 - 1;
}

/**
 * @brief Sets the weights of the perceptron.
 * 
 * @param weights A vector containing the new weights.
 */
template <typename T>
void Perceptron<T>::setWeights(const vector<T>& weights) {
    this->weights = weights;
}

/**
 * @brief Sets a specific weight of the perceptron.
 * 
 * @param index The index of the weight to set.
 * @param weights The new weight value.
 */
template <typename T>
void Perceptron<T>::setWeights(const int index, const T weights)
{
    this->weights[index] = weights;
}

/**
 * @brief Sets the bias of the perceptron.
 * 
 * @param bias The new bias value.
 */
template <typename T>
void Perceptron<T>::setBias(T bias)
{
    this->bias = T(bias);
}

/**
 * @brief Resets the weights and bias of the perceptron to random values.
 */
template <typename T>
void Perceptron<T>::resetWeightsBias()
{
    for (int i = 0; i < weights.size(); ++i) {
        weights[i] = ((T)rand() / RAND_MAX) * 2 - 1; // Random values between -1 and 1
    }
    bias = ((T)rand() / RAND_MAX) * 2 - 1;
}

/**
 * @brief Returns the current weights of the perceptron.
 * 
 * @return A vector containing the current weights.
 */
template <typename T>
vector<T> Perceptron<T>::_weights()
{
    return vector<T>(this->weights);
}

/**
 * @brief Returns the current bias of the perceptron.
 * 
 * @return The current bias value.
 */
template <typename T>
T Perceptron<T>::_bias()
{
    return T(this->bias);
}

/**
 * @brief Sets the activation function type for the perceptron.
 * 
 * @param type The activation function type (e.g., "linear", "sigmoid", "tanh", "relu", "leakyrelu", "softmax", "step").
 */
template <typename T>
void Perceptron<T>::typeActivation(string type)
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

/**
 * @brief Applies the activation function to the input value.
 * 
 * @param x The input value.
 * @return The output value after applying the activation function.
 */
template <typename T>
T Perceptron<T>::activation(T x) {
    if (activationType == "linear") {
        return x;
    } else if (activationType == "sigmoid") {
        return 1 / (1 + exp(-x));
    } else if (activationType == "tanh") {
        return tanh(x);
    } else if (activationType == "relu") {
        return (x > 0) ? x : 0;
    } else if (activationType == "leakyrelu") {
        return (x > 0) ? x : 0.01 * x;
    } else if (activationType == "softmax") {
        // Softmax is not applicable here for scalar values.
        std::cerr << "\033[1;31mSoftmax must be computed over a vector, not a scalar.\033[0m" << std::endl;
        throw std::invalid_argument("Softmax must be computed over a vector.");
    } else if (activationType == "step") {
        return (x > 0) ? 1 : 0;
    } else {
        std::cerr << "\033[1;31mActivation Type Not Found\033[0m" << std::endl;
        throw std::invalid_argument("Activation Type Not Found");
    }
}

/**
 * @brief Feeds the input values through the perceptron and computes the output.
 * 
 * @param inputs A vector containing the input values.
 * @return The output value after applying the perceptron.
 */
template <typename T>
T Perceptron<T>::feedForward(const vector<T>& inputs)
{
    T total = bias;
    for (int i = 0; i < weights.size(); ++i) {
        total += weights[i] * inputs[i];
    }

    output = activation(total);
    return T(output);
}

/**
 * @brief Feeds the input values through the perceptron with a specified bias and computes the output.
 * 
 * @param inputs A vector containing the input values.
 * @param bias The bias value to use.
 * @return The output value after applying the perceptron.
 */
template <typename T>
T Perceptron<T>::feedForward(const vector<T>& inputs, T bias)
{
    T total = bias;
    for (int i = 0; i < weights.size(); ++i) {
        total += weights[i] * inputs[i];
    }

    output = activation(total);
    return T(output);
}

/**
 * @brief Trains the perceptron using the given inputs and target output.
 * 
 * @param inputs A vector containing the input values.
 * @param target The target output value.
 * @param learningRate The learning rate for the training.
 */
template <typename T>
void Perceptron<T>::train(const vector<T>& inputs, T target, const T learningRate) {
    T error = target - feedForward(inputs);
    for (int i = 0; i < weights.size(); ++i) {
        weights[i] += learningRate * error * inputs[i];
    }
    bias += learningRate * error;
}

/**
 * @brief Returns the current weights of the perceptron.
 * 
 * @return A vector containing the current weights.
 */
template <typename T>
vector<T> Perceptron<T>::getWeights()
{
    return vector<T>(this->weights);
}

/**
 * @brief Returns the current bias of the perceptron.
 * 
 * @return The current bias value.
 */
template <typename T>
T Perceptron<T>::getBias()
{
    return T(this->bias);
}

/**
 * @brief Creates a copy of the current perceptron environment.
 * 
 * @return A copy of the current perceptron.
 */
template <typename T>
Perceptron<T> Perceptron<T>::cpyEnv() const
{
    return Perceptron<T>(*this);
}

/**
 * @brief Returns the current output of the perceptron.
 * 
 * @return The current output value.
 */
template <typename T>
T Perceptron<T>::getOutput()
{
    return T(this->output);
}

/**
 * @brief Displays the current state of the perceptron.
 */
template <typename T>
void Perceptron<T>::display()
{
    cout << "\033[1;32m-->> Perceptron <<--\033[0m" << endl << endl;
    cout << "\033[1;33mSize:\033[0m " << weights.size() << endl;
    cout << "\033[1;33mWeights:\033[0m ";
    for (int i = 0; i < weights.size(); ++i) {
        cout << weights[i] << " ";
    }
    cout << endl;
    cout << "\033[1;33mBias:\033[0m " << bias << endl;
    cout << "\033[1;33mActivation Type:\033[0m " << activationType << endl;
    cout << "\033[1;33mOutput:\033[0m " << output << endl;
}

// Explicitly instantiate the template for the types you need
template class Perceptron<float>;
template class Perceptron<double>;