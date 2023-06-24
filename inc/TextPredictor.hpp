#pragma once

#include "../lib/IntNet/intnet.hpp"

#define CHARBUFFERSIZE 1

class TextPredictor
{
	public:
		in::NeuralNetwork	*network;

		TextPredictor();
		TextPredictor(in::NeuralNetwork *network);

		void train(std::string input, char target);

		char predict(std::string input);

		~TextPredictor();
};
