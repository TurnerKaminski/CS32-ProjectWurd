#include "StudentSpellCheck.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;
SpellCheck* createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::~StudentSpellCheck() {
	killAll(m_trieRoot);
}
//load the trie dictionary
bool StudentSpellCheck::load(std::string dictionaryFile) {
	ifstream dicFile(dictionaryFile);
	if (!dicFile)
	{
		return false;
	}
	delete m_trieRoot;
	m_trieRoot = new Trie('\0');
	Trie* current = m_trieRoot;
	string s;
	while (getline(dicFile, s))
	{
		insert(current, s);
	}
	return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) {
	//for me it was easiest to just create an array of characters to test replacement
	char replacementChar[27] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\''};
	//make sure its all in lower for searching the trie
	for (int t = 0; t < word.length(); t++)
	{
		word[t] = tolower(word[t]);
	}
	//if its in the dic just leave
	if (search(m_trieRoot, word)){
		return true;
	}
	//make sure we clean
	suggestions.clear();
	for(int t = 0; t < word.length(); t++) //each letter in word
	{
		string temp = word; //dont want to change the original word
		for (int z = 0; z < 27; z++) //each possible character
		{
			temp[t] = replacementChar[z]; //swap out character
			if(search(m_trieRoot, temp) && suggestions.size() <= max_suggestions) //search for new word as a possible suggestion
			{
				suggestions.push_back(temp);
			}
		}
	}
	return false;
}
//this function gave me the most trouble, was giving me a ton of seg faults, pain in the ass
void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) {
	int start = 0;
	int lastStart = -69; //nice
	int end = 0;
	vector<string> validWords;
	vector<Position> positions;
	string tempWord = "";
	//go through line
	for (int t = 0; t < line.length(); t++)
	{
		//store each valid char and build each word char by char
		if (isValidChar(line[t]))
		{
			if (tempWord == "")
				start = t;
			tempWord += tolower(line[t]);
		}
		//not an alpha or ' so it must be considered a break between words
		else
		{
			if (t != 0 && tempWord != "")
			{
				validWords.push_back(tempWord);
			}
			end = t;
			tempWord = "";
		}
		//mark start of a new word
		if (start < end && start != lastStart)
		{
			if (!isValidChar(line[start]))
				continue;
			lastStart = start;
			Position pos;
			pos.start = start;
			pos.end = end-1;
			positions.push_back(pos);
		}
	}
	//check last word in each line, wasnt being covered in the upper loop
	if(!tempWord.empty() && isValidChar(tempWord[tempWord.length() - 1]))
	{
		validWords.push_back(tempWord);
		Position pos;
		pos.start = start;
		pos.end = line.length() - 1;
		positions.push_back(pos);
	}
	//search through all our created words
	for (int t = 0; t < validWords.size(); t++)
	{
		//if they problematic add their position
		if(!search(m_trieRoot, validWords[t]))
		{
			problems.push_back(positions[t]);
		}
	}
}
//avoid some repeat code
bool StudentSpellCheck::isValidChar(char ch)
{
	return isalpha(ch) || ch == '\'';
}
//standard trie insert function, plus a special case for \'
void StudentSpellCheck::insert(struct Trie* root, string str)
{
	struct Trie* current = root; //as to not confuse the root with a current node when moving through
	for(int t = 0; t < str.length(); t++)
	{
		char currentChar = tolower(str[t]);
		if(isalpha(currentChar))
		{
			if(current->child[currentChar - 'a'] == nullptr)
				current->child[currentChar - 'a'] = new Trie(currentChar);
			current = current->child[currentChar - 'a'];
		}
		else if (currentChar == '\'')
		{
			if(current->child[26] == nullptr)
				current->child[26] = new Trie(currentChar);
			current = current->child[26];
		}
	}
	current->endOfWord = true;
}
//wasnt really working with the work inside this function so i used a section function to do most of its work
bool StudentSpellCheck::search(struct Trie* root, string word)
{
	for(int t = 0; t < word.length(); t++)
	{
		word[t] = tolower(word[t]);
	}
	Trie* check = getWord(root, word);
	return (check != nullptr && check->endOfWord);
}
//easier way to implement the destructor imo
void StudentSpellCheck::killAll(struct Trie* root)
{
	struct Trie* current = root;
	if (current == nullptr)
		return;
	for (int t = 0; t < 27; t++)
	{
		killAll(current->child[t]);
	}
	delete current;
}
//heres what i used to do most of the work for the searching of my Trie
StudentSpellCheck::Trie* StudentSpellCheck::getWord(Trie* root, string word)
{
	struct Trie* current = root;
	for (int t = 0; t < word.length(); t++)
	{
		char currentChar = word[t];
		if (isalpha(currentChar))
		{
			if(!current->child[currentChar - 'a'])
				return nullptr;
			current = current->child[currentChar - 'a'];
		}
		else if (currentChar == '\'')
		{
			if(!current->child[26])
				return nullptr;
			current = current->child[26];
		}
		else
			return nullptr;
	}
	return current;
}
