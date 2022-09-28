#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>
using namespace std;

class StudentSpellCheck : public SpellCheck {
public:
    StudentSpellCheck() { 
		m_trieRoot = new Trie('\0');
	}
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
	struct Trie 
	{
		Trie(char t)
		{
			ch = t;
		}
		char ch;
		Trie* child[27] = {nullptr}; 
		bool endOfWord;
	};
	struct Trie* m_trieRoot;
	void insert(struct Trie* root, string str);
	bool search(struct Trie* root, string word);
	StudentSpellCheck::Trie* getWord(struct Trie* root, string word);
	void killAll(struct Trie* root);
	bool isValidChar(char ch);
};

#endif  // STUDENTSPELLCHECK_H_
