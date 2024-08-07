#include <bits/stdc++.h>
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

void dfs(Node *node, string pathSoFar, unordered_map<char, string> &mp)
{
	if (isLeaf(node))
	{
		mp[node->ch] = pathSoFar;
		return;
	}

	dfs(node->left, pathSoFar + "0", mp);
	dfs(node->right, pathSoFar + "1", mp);
}

string getEncodedFileContent(string &sourceFileContent, unordered_map<char, string> &code)
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

unordered_map<char, int> buildFreqMap(string &text)
{
	unordered_map<char, int> freq;
	for (auto &ch : text)
	{
		freq[ch]++;
	}
	return freq;
}

Node *buildTree(unordered_map<char, int> &freq)
{
	auto cmp = [](Node *a, Node *b)
	{
		// todo: why > ?
		return a->freq > b->freq;
	};
	priority_queue<Node *, vector<Node *>, decltype(cmp)> pq(cmp);
	for (auto &it : freq)
	{
		Node *temp = new Node(it.first, it.second);
		pq.push(temp);
	}

	while (pq.size() > 1)
	{
		Node *l = pq.top();
		pq.pop();
		Node *r = pq.top();
		pq.pop();

		Node *merged = new Node(-1, l->freq + r->freq);
		merged->left = l;
		merged->right = r;

		pq.push(merged);
	}

	Node *root = pq.top();

	return root;
}

unordered_map<char, string> generateCharCodes(Node *root)
{
	unordered_map<char, string> charCodes;
	string helper = "";
	dfs(root, helper, charCodes);
	// now we know what characters to map to what codes
	// eg, c->101, a -> 11100110 etc
	for (auto k : charCodes)
	{
		cout << (int)k.first << ":" << k.second << endl;
	}
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

int main(int argc, char *argv[])
{
	ifstream inputFile;
	string inputFileName = argv[1];
	string outputFileName;
	if (argc > 2)
		outputFileName = argv[2];
	else
		outputFileName = "compressed_" + inputFileName.substr(0, inputFileName.size() - 4) + ".bin";

	inputFile.open(inputFileName);
	ofstream binaryFile(outputFileName, ios::binary);
	if (inputFile.is_open() && binaryFile.is_open())
	{
		string text = readFileContent(inputFile);
		inputFile.close();

		// main logic
		auto freqmap = buildFreqMap(text);
		Node *hofftree = buildTree(freqmap);
		auto codemap = generateCharCodes(hofftree);
		string coded = getEncodedFileContent(text, codemap);
		// cout << decodeUsingTree(hofftree, coded) << endl;

		// string coded = "0100000101000001";
		// Each bute in `coded` represents a bit in the compressed file
		// which we want to put in our binary file
		// BYTE BY BYTE
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
		binaryFile.flush();
		binaryFile.close();
	}
	else
	{
		cerr << "Error! Something went wrong.";
	}

	return 0;
}