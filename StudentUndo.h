#ifndef STUDENTUNDO_H_
#define STUDENTUNDO_H_

#include "Undo.h"
#include <stack>
#include <list>
#include <queue>

class StudentUndo : public Undo {
public:

	void submit(Action action, int row, int col, char ch = 0);
	Action get(int& row, int& col, int& count, std::string& text);
	void clear();

private:
	//input struct stores all the necessary input for perfoming undos
	struct input
	{
		input(Action act, int row)
		{
			m_action = act;
			m_row = row;
			m_count = 0;
		}
		Action m_action;
		int m_row;
		int m_count;
		std::queue<int> cols; //keep track of where each undo will be (needed a container for batching)
		std::list<char> chars; //keep track of characters (needed a conatiner for batching)
	};
	std::stack<input> actions; //keep track of all the inputted actions
};

#endif // STUDENTUNDO_H_
