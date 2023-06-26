#pragma once

#include "../lib/IntNet/intnet.hpp"

#define CHARBUFFERSIZE 3

struct CharRank
{
		char  c;
		float prob;
};

class TextPredictor
{
	public:
		in::NeuralNetwork	 *network;
		std::vector<CharRank> rank;

		TextPredictor();
		TextPredictor(in::NeuralNetwork *network);

		void train(std::string input, char target);

		char predict(std::string input);

		~TextPredictor();
};
