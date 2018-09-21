//============================================================================
// Name        : Data Compression Tool.cpp
// Author      : Himanshu Kumar
// Description : A Data compression command line tool to compress and decompress ASCII encoded
//				 files according to the 3 famous compression algorithms
// 				 1. Huffman Coding
//				 2. LWZ Coding
//				 3. Shannon-Fano Coding
//============================================================================

#include <iostream>
#include <queue>
#include <map>
#include <bitset>
#include <fstream>
using namespace std;

class CompressionTool {
public :
	virtual void compression() = 0;
	virtual void decompression() = 0;
};

class Huffman : public CompressionTool{
private :
	map<char, int> freqMapping;
	map<char, vector<bool> > codeMapping;
	const size_t bufferSize = 1024*1024;
	const char *inputfileName;
	const char *outputfileName;

public :
	Huffman(const char *inputfile, const char *outputfile) : inputfileName(inputfile), outputfileName(outputfile){}

	class huffmanNode{
	public :
		int frq;
		virtual ~huffmanNode(){}
	protected :
		huffmanNode(int f) : frq(f) {}
	};

	class InternalNode : public huffmanNode{
	public:
		huffmanNode *left;
		huffmanNode *right;
		InternalNode(huffmanNode* lChild, huffmanNode* rChild) : huffmanNode(lChild->frq + rChild->frq), left(lChild), right(rChild) {}
		~InternalNode(){
			delete left;
			delete right;
	 }
	};

	class LeafNode : public huffmanNode{
	public:
		char c;
		LeafNode(char rep, int frq) : huffmanNode(frq), c(rep) {}
	};

	struct myFreqComp{
		bool operator()(const huffmanNode *lNode,const huffmanNode *rNode){
			return lNode->frq > rNode->frq;
		}
	};

	void buildFreqTable(){
		ifstream inputfile;
		inputfile.open(this->inputfileName, ios::in);
		char *buffer = new char[bufferSize];

		while(inputfile){
			inputfile.read(buffer, bufferSize);

			int idx=0;
			while(buffer[idx]!='\0'){
				this->freqMapping[buffer[idx++]]++;
			}
		}

		delete[] buffer;
		inputfile.close();
	}

	void encodeFile(){
		ofstream outfile;
		ifstream inputfile;

		outfile.open(this->outputfileName, ios::out | ios::binary);
		inputfile.open(this->inputfileName, ios::in);

		char *buffer = new char[bufferSize];

		bitset<8> byteCombination(0);
		int cnt=0;

		while(inputfile){
			inputfile.read(buffer, this->bufferSize);
			int idx = 0;
			while(buffer[idx]!='\0'){
				vector<bool> prefixCode = this->codeMapping[buffer[idx]];
				for(auto i:prefixCode){
					if(i)
						byteCombination = byteCombination<<1 | bitset<8>(1);
					else
						byteCombination = byteCombination<<1 | bitset<8>(0);
					cnt+=1;
					if(cnt==8){
						unsigned long decimalRep = byteCombination.to_ulong();
						outfile.write(reinterpret_cast<char *>(&decimalRep), sizeof(char));
						cnt = 0;
						byteCombination.reset();
					}
				}
				idx++;
			}
		}

		byteCombination = byteCombination<<(8-cnt);

		unsigned long decimalRep = byteCombination.to_ulong();
		outfile.write(reinterpret_cast<char *>(&decimalRep), sizeof(char));


		delete[] buffer;
		inputfile.close();
		outfile.close();
	}

	void buildHuffmanCodes(vector<bool> prefixCode, huffmanNode *root, map<char, vector<bool> >& codeMapping){
		if(!root)
			return;

		if(LeafNode *leaf = dynamic_cast<LeafNode*>(root)){
			codeMapping[leaf->c] = prefixCode;
		}
		else if(InternalNode *inode = dynamic_cast<InternalNode*>(root)){

			prefixCode.push_back(false);
			buildHuffmanCodes(prefixCode, inode->left, codeMapping);

			prefixCode[prefixCode.size()-1] = true;
			buildHuffmanCodes(prefixCode, inode->right, codeMapping);
		}
		return ;
	}

	huffmanNode* buildTree(map<char, int>& freqMapping){
		priority_queue<huffmanNode*, vector<huffmanNode*>, myFreqComp> huffmanTree;

		for(auto i:freqMapping){
			huffmanTree.push(new LeafNode(i.first, i.second));
		}

		while(huffmanTree.size()!=1){
			huffmanNode* topOne = huffmanTree.top();
			huffmanTree.pop();
			huffmanNode* topSecond = huffmanTree.top();
			huffmanTree.pop();

			InternalNode* inode = new InternalNode(topOne, topSecond);
			huffmanTree.push(inode);
		}
		huffmanNode* root = huffmanTree.top();
		huffmanTree.pop();

		return root;
	}

	void compression(){
		cout<<"Generating Frequency Table"<<endl;
		this->buildFreqTable();
		cout<<"Completed Generating Frequency Table"<<endl;
		huffmanNode* root = this->buildTree(this->freqMapping);
		cout<<"Generating Code Mapping Table"<<endl;
		this->buildHuffmanCodes(vector<bool>(), root, this->codeMapping);
		cout<<"Completed Code Mapping Table"<<endl;
		cout<<"Encoding File"<<endl;
		this->encodeFile();
		cout<<"Completed Encoding File"<<endl;
	}

	void decompression() {
		//To be implemented;
	}
};

int main(int argc, char *argv[]) {
	if(argc==1)
	{
		cout<<"Filename is not given"<<endl;
		return 0;
	}

	Huffman file(argv[1], argv[2]);

	//Perform compression algorithm
	file.compression();

	return 0;
}



















