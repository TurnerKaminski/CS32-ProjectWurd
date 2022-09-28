#include "StudentUndo.h"
#include <list>
Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) {
	//if our stack is empty just add it in normally
	if (actions.empty())
	{
		//store all our values for the inputted action
		input newI(action, row);
		newI.cols.push(col);
		newI.chars.push_back(ch);
		newI.m_count++;
		actions.push(newI);
		return;
	}
	//else we require more specific action for tracking for batching purposes
	else
	{
		input& top = actions.top();
		switch(action)
		{
			case Undo::Action::INSERT:
			{
				if(top.m_action == action && top.m_row == row && top.cols.back() == col - 1)
				{
					top.cols.push(col);
					top.m_count++;
					return;
				}
				break;
			}
			case Undo::Action::DELETE:
			{
				if(top.m_action == action && top.m_row == row)
				{
					//delete and backspace need to handled seperate
					//this is delete
					if(top.cols.back() == col)
					{
						top.cols.push(col);
						top.chars.push_back(ch);
						top.m_count++;
						return;
					}
					//this is backspace
					else if (top.cols.back() == col + 1)
					{
						top.cols.push(col);
						top.chars.push_front(ch);
						top.m_count++;
						return;
					}
				}
				break;
			}
		}
	}
	//if batching wasnt necessary, and input stack wasnt empty just add it normally
	input newI(action, row);
	newI.cols.push(col);
	newI.chars.push_back(ch);
	newI.m_count++;
	actions.push(newI);
	return;
}

StudentUndo::Action StudentUndo::get(int& row, int& col, int& count, std::string& text) {
    if(actions.empty())
		return Action::ERROR;  // TODO
	//grab the action we need to undo
	//create all necessary values and give them defaults
	input top = actions.top();
	actions.pop();
	count = 1;
	text = "";
	row = top.m_row;
	col = top.cols.back();
	//return the opposite of the actiom, with extra code to handle the insert and delete cases
	switch(top.m_action)
	{
		case Undo::Action::DELETE:
		{
			for (std::list<char>::iterator it = top.chars.begin(); it != top.chars.end(); it++)
			{
				text += (*it);
			}
			return Undo::Action::INSERT;
			break;
		}
		case Undo::Action::INSERT:
		{
			col = top.cols.front() - 1;
			count = top.m_count;
			return Undo::Action::DELETE;
			break;
		}
		case Undo::Action::SPLIT:
		{
			return Undo::Action::JOIN;
			break;
		}
		case Undo::Action::JOIN:
		{
			return Undo::Action::SPLIT;
			break;
		}
		default:
		{
			return Undo::Action::ERROR;
			break;
		}
	}
}
//this was my favorite function to write :)
void StudentUndo::clear() {
	while(!actions.empty())
		actions.pop();
}
