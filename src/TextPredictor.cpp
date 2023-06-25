#include "../inc/TextPredictor.hpp"
#include <fstream>

#define CHARDIMENSION 26

TextPredictor::TextPredictor()
{
	in::NetworkStructure netstruct = in::NetworkStructure(1 + (CHARBUFFERSIZE * CHARDIMENSION), {}, CHARDIMENSION); // null, a-z, , \n
	in::NetworkStructure::randomWeights(netstruct);

	network = new in::NeuralNetwork(netstruct);

	network->setActivation(in::sigmoid);

	network->setInputNode(0, 1);
}

TextPredictor::TextPredictor(in::NeuralNetwork *network)
{
	this->network = network;
	network->setActivation(in::sigmoid);
	network->setInputNode(0, 1);
}

int charToNode(char c)
{
	int ci = 0;

	if (c >= 97 && c <= 122)
	{
		ci = c - 96;
	}

	return ci;
}

char nodeToChar(int node)
{
	int ni = 0;

	if ((node + 96) >= 97 && (node + 96) <= 122)
	{
		ni = node + 96;
	}

	return ni;
}

char TextPredictor::predict(std::string input)
{
	for (int i = 1; i < network->structure.totalInputNodes; i++)
	{
		network->setInputNode(i, 0);
	}

	for (int i = 0; i < CHARBUFFERSIZE; i++)
	{
		char node = charToNode(input[i]);

		network->setInputNode((i * CHARDIMENSION) + node, 1);
	}

	network->update();

	int	  max		  = 0;
	float val		  = 0;
	int	  totalSubOut = network->structure.totalNodes - network->structure.totalOutputNodes;

	for (int i = 0; i < network->structure.totalOutputNodes; i++)
	{
		if (network->node[i + totalSubOut].value > val)
		{
			val = network->node[i + totalSubOut].value;
			max = i;
		}
	}

	return nodeToChar(max);
}

void TextPredictor::train(std::string input, char target)
{
	std::vector<float> targetValue;
	targetValue.resize(CHARDIMENSION + 1);

	targetValue[charToNode(target)] = 1;

	this->predict(input);

	std::cout << network->backpropagation(targetValue) << '\n';
}

TextPredictor::~TextPredictor()
{
	network->destroy();
	delete network;
}
