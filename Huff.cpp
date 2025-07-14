#include <bits/stdc++.h>
#include <sstream>
using namespace std;
struct Node
{
	int ch, freq;
	Node *left, *right;

	Node(int ch, int freq)
			: ch(ch), freq(freq), left(NULL), right(NULL) {}
};

bool isLeaf(Node *root)
{
	return !root->left && !root->right;
}

void dfs(Node *node, string pathSoFar, map<char, string> &mp)
{
	if (isLeaf(node))
	{
		mp[node->ch] = pathSoFar;
		return;
	}

	dfs(node->left, pathSoFar + "0", mp);
	dfs(node->right, pathSoFar + "1", mp);
}

string getEncodedFileContent(string &sourceFileContent, map<char, string> &code)
{
	string encodedFileContent = "";
	for (char &ch : sourceFileContent)
	{
		encodedFileContent += code[ch];
	}

	// the string `encodedFileContent` holds the bits that are supposed to be there
	// in the compressed file, but it itself isn't compressed just yet
	// to represent each bit of info, it itself takes 8 bits (1 char)
	return encodedFileContent;
}

string decodeUsingTree(Node *root, string coded)
{
	// 1010011001010001101010101010110
	//  a-> 101, c -> 0001
	if (isLeaf(root))
	{
		// Handling the case when the file only contains one character
		// So the path to that character (ie, character code for it) is empty string
		char c = root->ch;
		int freq = root->freq;
		string dec = "";
		while (freq--)
		{
			dec += c;
		}
		return dec;
	}
	Node *temp = root;
	string decoded = "";
	for (auto &ch : coded)
	{
		if (ch == '1')
			temp = temp->right;
		else
			temp = temp->left;

		if (isLeaf(temp))
		{
			decoded += (char)temp->ch;
			temp = root;
		}
	}
	return decoded;
}

map<char, int> buildFreqMap(string &text)
{
	map<char, int> freq;
	for (auto &ch : text)
	{
		freq[ch]++;
	}
	return freq;
}
Node *buildTree(map<char, int> &freq)
{
	auto cmp = [](Node *a, Node *b)
	{
		return a->freq > b->freq; // Custom comparator for the priority queue
	};

	// Priority queue (min-heap) to hold the nodes
	priority_queue<Node *, vector<Node *>, decltype(cmp)> pq(cmp);

	// decltype
	//  decltype(cmp) ensures the priority_queue correctly uses the type of the comparator function defined by cmp, allowing the queue to function with the desired priority ordering.

	// Create a leaf node for each character and push it into the priority queue
	for (auto &it : freq)
	{
		Node *temp = new Node(it.first, it.second);
		pq.push(temp);
	}

	//  Build the Huffman tree
	while (pq.size() > 1)
	{
		Node *l = pq.top();
		pq.pop(); // Remove the node with the smallest frequency
		Node *r = pq.top();
		pq.pop(); // Remove the next smallest frequency node

		// Create a new internal node with the sum of the two nodes' frequencies
		Node *merged = new Node(-1, l->freq + r->freq);
		merged->left = l;	 // Left child is the first node removed
		merged->right = r; // Right child is the second node removed

		// Add the merged node back into the priority queue
		pq.push(merged);
	}

	// Step 3: The remaining node in the queue is the root of the Huffman tree
	Node *root = pq.top();

	return root; // Return the root of the Huffman tree
}

map<char, string> generateCharCodes(Node *root)
{
	map<char, string> charCodes;
	string helper = "";
	dfs(root, helper, charCodes);
	// now we know what characters to map to what codes
	// eg, c->101, a -> 11100110 etc
	return charCodes;
}

string readFileContent(ifstream &inputFile)
{
	string text = "";
	string line;

	bool firstLine = true;
	while (getline(inputFile, line))
	{
		if (firstLine)
		{
			firstLine = false;
		}
		else
		{
			text += "\n";
		}
		text += line;
	}
	return text;
}

string serializeFreqMap(map<char, int> m)
{
	stringstream ss;
	for (auto &it : m)
	{
		ss << it.first << " " << it.second << " ";
	}
	ss << "\n";
	string ret = ss.str();
	ret[ret.size() - 2] = '\n';

	return ret;
}

void printFreqMap(map<char, int> m)
{
	for (auto k : m)
	{
		cout << (int)k.first << " " << k.second << endl;
	}
}

// expects _i_ to be at the start of the number
// leaves _i_ just after the last digit parsed
int readNumber(string s, int &i)
{
	int n = 0;
	while (s[i] >= '0' && s[i] <= '9')
	{
		n = n * 10 + (s[i] - '0');
		i++;
	}
	return n;
}

map<char, int> deserializeFreqMap(string s)
{
	int i = 0;
	map<char, int> constructed;
	while (i < s.size())
	{
		char c = s[i++];
		i++;
		int freq = readNumber(s, i);
		i++;
		constructed[c] = freq;
	}
	return constructed;
}

void serializeText(string &text, ofstream &binaryFile, ofstream &fmapFile)
{
	// main logic
	auto freqmap = buildFreqMap(text);
	Node *hofftree = buildTree(freqmap);
	auto codemap = generateCharCodes(hofftree);
	string coded = getEncodedFileContent(text, codemap);
	// printFreqMap(freqmap);
	// cout << decodeUsingTree(hofftree, coded) << endl;

	// string coded = "0100000101000001";
	// Each byte in `coded` represents a bit in the compressed file
	// which we want to put in our binary file
	// BYTE BY BYTE
	// so we need to ensure that its length is a multiple of 8
	// so we add some padding 0s at the end. But when decompressing, we need to
	// know how many 0s in the end are of padding, so we add another 8 digits (0 or 1)
	// which are the binary representation of the number of padding 0s that have been put
	// so when deserializing, we read the last eight digits, parse them into a number, and
	// ignore (that many numbers + 8 ) from the compressed text.
	int paddingZerosReqd = 8 - (coded.size() % 8);
	for (int i = 0; i < paddingZerosReqd; i++)
	{
		coded.push_back('0');
	}
	for (int i = 7; i >= 0; i--)
	{
		char c = 48 + ((paddingZerosReqd >> i) > 0);
		coded.push_back(c);
	}
	int count = 0;
	for (int i = 0; i < coded.size(); i += 8)
	{
		// We compress the 8 bit values (which occur in 8 bytes in the coded string)
		// into an actual char of 8 bits
		// and then put that in the compressed binary file
		string byteStr = coded.substr(i, 8);
		char byte = stoi(byteStr, nullptr, 2);
		count++;
		binaryFile.write(&byte, sizeof(char));
	}
	string serializedFreqMap = serializeFreqMap(freqmap);
	fmapFile.write(serializedFreqMap.c_str(), serializedFreqMap.size());
}

const bool SERIALIZE = false, DESERIALIZE = true;
struct Config
{
	string inputFileName;
	string outputFileName;
	string fmapFileName;
	bool modeOfOperation;
};

void serialize(Config &config)
{
	// it is used to open a file for reading
	ifstream inputFileStream;
	inputFileStream.open(config.inputFileName);

	// is used to open a file for writing in binary mode.
	ofstream binaryFileStream(config.outputFileName, ios::binary);
	ofstream fmapFileStream(config.fmapFileName);
	if (!inputFileStream.is_open() || !binaryFileStream.is_open())
	{
		cerr << "Error! Something went wrong.";
	}

	string text = readFileContent(inputFileStream);
	serializeText(text, binaryFileStream, fmapFileStream);
	inputFileStream.close();

	binaryFileStream.flush();
	binaryFileStream.close();
}

string expandToBits(const string &s)
{
	std::string result;

	// Iterate over each character in the string
	for (char c : s)
	{
		// Convert character to its ASCII value
		unsigned char asciiValue = static_cast<unsigned char>(c);

		// Convert ASCII value to a 8-bit binary string
		std::bitset<8> binary(asciiValue);

		// Append the binary representation to the result string
		result += binary.to_string();
	}

	return result;
}

string deserialize(Config &conf)
{
	ifstream inputFileStream, fmapFileStream;
	inputFileStream.open(conf.inputFileName, ios::binary);
	fmapFileStream.open(conf.fmapFileName);

	if (!inputFileStream.is_open() || !fmapFileStream.is_open())
	{
		cerr << "Couldn't open compressed file" << endl;
	}
	string serializedFreqMap = readFileContent(fmapFileStream);
	auto freqmap = deserializeFreqMap(serializedFreqMap);

	string compressedText = expandToBits(readFileContent(inputFileStream));
	string padInfo = compressedText.substr(compressedText.size() - 8);
	compressedText = compressedText.substr(0, compressedText.size() - 8 - stoi(padInfo, 0, 2));
	Node *hofftree = buildTree(freqmap);
	string decoded = decodeUsingTree(hofftree, compressedText);
	cout << decoded << endl;
}
struct Config readConfig(int argc, char *argv[])
{
	Config config;
	for (int i = 0; i < argc; i++)
	{
		string arg(argv[i]);
		if (arg.find("--input=") != string::npos)
		{
			config.inputFileName = arg.substr(8);
			continue;
		}
		if (arg.find("--output=") != string::npos)
		{
			config.outputFileName = arg.substr(9);
			config.fmapFileName = config.outputFileName.substr(0, config.outputFileName.size() - 4) + "_fmap.txt";

			continue;
		}
		if (arg.find("--fmap=") != string::npos)
		{
			config.fmapFileName = arg.substr(7);

			continue;
		}

		if (arg == "--deserialize")
		{
			config.modeOfOperation = DESERIALIZE;
		}
	}
	// cout<<config.inputFileName<<" "<<config.outputFileName<<" "<<config.fmapFileName<<endl;
	return config;
}

int main(int argc, char *argv[])
{
	Config conf = readConfig(argc, argv);
	if (conf.modeOfOperation == SERIALIZE)
	{
		serialize(conf);
	}
	else
	{
		deserialize(conf);
	}
}