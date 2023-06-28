#include "../inc/TextPredictor.hpp"
#include <fstream>

#define CHARDIMENSION 26

TextPredictor::TextPredictor()
{
	in::NetworkStructure netstruct =
		in::NetworkStructure(1 + (CHARBUFFERSIZE * CHARDIMENSION), {CHARBUFFERSIZE * CHARDIMENSION},
							 CHARDIMENSION); // null, a-z, , \n
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
	else
	{
		ni = 'z'; // lazy fix
	}
	return ni;
}

char TextPredictor::predict(std::string input)
{
	for (int i = 97; i <= 122; i++)
	{
		if (nodeToChar(charToNode(i)) != i)
		{
			std::cout << "false " << i << '\n';
		}
	}

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

	int totalSubOut = network->structure.totalNodes - network->structure.totalOutputNodes;

	rank.clear();

	float val;
	float max;

	for (int i = 0; i < network->structure.totalOutputNodes; i++)
	{
		for (int x = 0; x < rank.size(); x++)
		{
			if (network->node[i + totalSubOut].value > rank[x].prob)
			{
				val = network->node[i + totalSubOut].value;
				max = i;
				rank.insert(rank.begin() + x, {nodeToChar(max), val});

				goto endFor;
			}
		}

		val = network->node[i + totalSubOut].value;
		max = i;
		rank.push_back({nodeToChar(max), val});

	endFor:;
	}

	return rank[0].c;
}

float TextPredictor::train(std::string input, char target)
{
	std::vector<float> targetValue;
	targetValue.resize(CHARDIMENSION + 1);

	targetValue[charToNode(target)] = 1;

	this->predict(input);

	return network->backpropagation(targetValue);
}

TextPredictor::~TextPredictor()
{
	network->destroy();
	delete network;
}
