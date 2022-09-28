#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
TextEditor* createTextEditor(Undo* un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
 : TextEditor(undo) {
	m_editpos.row = 0;
	m_editpos.col = 0;
	m_list.push_back("");
	m_line = m_list.begin();
}

StudentTextEditor::~StudentTextEditor()
{
	// TODO nothing this is a trick
}
//just load in what you got passed in nothin special
bool StudentTextEditor::load(std::string file) {
	ifstream editorFile(file);
	if (!editorFile){
		return false;
	}
	reset();
	string s;
	while (getline(editorFile, s))
	{
		if(!s.empty() && s.back() == '\r')
			s.pop_back();
		m_list.push_back(s);
	}
	m_line = m_list.begin();
	m_editpos.row = 0;
	m_editpos.col = 0;
	return true;
}
//same thing here nothing special
bool StudentTextEditor::save(std::string file) {
	ofstream saveFile(file); //access a file to be saved to  
	if (!saveFile) {
		return false;
	}
	list<string>::iterator endOfList = m_list.end(); //store end of list
	for(list<string>::iterator it = m_list.begin(); it != endOfList; it++) //loop through list 
	{
		saveFile << *it << endl; //add each line and a new '\n' character to the saveFile 
	}
	return true;
}
//just clear out the list of text
void StudentTextEditor::reset() {
	m_list.clear();
	m_editpos.row = 0;
	m_editpos.col = 0;
	m_line = m_list.begin();
}
//move the cursor using the editpos struct 
void StudentTextEditor::move(Dir dir) {
	switch (dir)
	{
		case UP: {
			//if top line
			if(m_editpos.row == 0)
				return;
			m_line--;
			//if you move to a shorter line
			if ((*m_line).size() < m_editpos.col)
				m_editpos.col = (*m_line).size();
			m_editpos.row--;
			break;
		}
		case DOWN: {
			//if you cant move down farther
			if (m_editpos.row + 1 > m_list.size())
				return;
			m_line++;
			//if you move to a shorter line
			if ((*m_line).size() < m_editpos.col)
				m_editpos.col = (*m_line).size();
			m_editpos.row++;
			break;
		}
		case LEFT: {
			//if you on the farthest left spot
			if (m_editpos.col == 0)
			{
				//if you on the top line you cant move up a line so dont move
				if (m_editpos.row == 0)
					return;
				else{
					m_editpos.row--;
					m_line--;
					m_editpos.col = (*m_line).size();
				}
			}
			else 
				m_editpos.col--;
			break;
		}
		case RIGHT: {
			//if you need to move to a new line and arent on the bottom
			if (m_editpos.col == (*m_line).size() && m_editpos.row < m_list.size() - 1)
			{
				m_editpos.row++;
				m_line++;
				m_editpos.col = 0;
			}
			else
				m_editpos.col++;
			break;
		}
		case HOME: {
			//nothing special
			m_editpos.col = 0;
			break;
		}
		case END: {
			//nothing special
			m_editpos.col = (*m_line).size();
			break;
		}
	}
}

void StudentTextEditor::del() {
	//normal delete case
	if (m_editpos.col < m_line->size())
	{
		char ch = (*m_line).at(m_editpos.col);
		getUndo()->submit(Undo::Action::DELETE, m_editpos.row, m_editpos.col, ch);
		m_line->erase(m_line->begin() + m_editpos.col);
	}
	//joining two lines case
	else if (m_editpos.row + 1 < m_list.size())
	{
		string temp = (*m_line);
		m_line = m_list.erase(m_line);
		(*m_line) = temp + (*m_line);
		getUndo()->submit(Undo::Action::JOIN, m_editpos.row, m_editpos.col);
	}
}

void StudentTextEditor::backspace() {
	if (m_editpos.col == 0 && m_editpos.row == 0)
		return;
	//if its not on the first spot
	if (m_editpos.col > 0)
	{
		m_editpos.col--;
		getUndo()->submit(Undo::Action::DELETE, m_editpos.row, m_editpos.col, (*m_line).at(m_editpos.col));
		m_line->erase(m_line->begin() + m_editpos.col);
	}
	//else if it will be in the first spot, and will be joining two lines
	else if (m_editpos.col == 0)
	{
		m_editpos.row--;
		m_line--;
		m_editpos.col = m_line->length();
		string temp = (*m_line);
		m_line = m_list.erase(m_line);
		(*m_line) = temp + (*m_line);
		//should i be passing in the previous row we were on or the new one?
		getUndo()->submit(Undo::Action::JOIN, m_editpos.row, m_editpos.col);
	}
}

void StudentTextEditor::insert(char ch) {
	//handle tab special case 
	if(ch == '\t')
	{
		for (int t = 0; t < 4; t++){
			(*m_line).insert(m_editpos.col, 1 , ' '); //insert 4 spaces
			m_editpos.col ++; //move editpos accordingly
			getUndo()->submit(Undo::Action::INSERT, m_editpos.row, m_editpos.col, ch);
		}
	}
	//any other character is treated the same
	else
	{
		(*m_line).insert((*m_line).begin() + m_editpos.col, ch);
		m_editpos.col++;
		getUndo()->submit(Undo::Action::INSERT, m_editpos.row, m_editpos.col, ch);
	}
}

void StudentTextEditor::enter() {
	getUndo()->submit(Undo::Action::SPLIT, m_editpos.row, m_editpos.col);
	string firstHalf = m_line->substr(0, m_editpos.col);
	string secondHalf = m_line->substr(m_editpos.col);
	*m_line = firstHalf; //up until enter key stays on current line
	m_list.insert(next(m_line), ""); //create a new line
	m_line++;
	m_editpos.row++;
	*m_line = secondHalf;
	m_editpos.col = 0;
}

void StudentTextEditor::getPos(int& row, int& col) const {
	row = m_editpos.row;
	col = m_editpos.col;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const {
	if (startRow < 0 || numRows < 0 || m_list.size() < startRow)
		return -1;
	auto line = m_line;
	//how far are we from the start row
	int dis = abs(m_editpos.row - startRow);
	//move to the start row
	//if we're behind the start line
	if (m_editpos.row < startRow)
	{
		for(int t = 0; t < dis; t++)
		{
			line++;
		}
	}
	//if we're in front of it
	else
	{
		for (int t = 0; t < dis; t++)
		{
			line--;
		}
	}
	lines.clear();
	//add fromm starting row
	for(int count = 0; count < numRows && line != m_list.end(); count++)
	{
		lines.push_back(*line);
		line++;
	}
	return lines.size();
}

void StudentTextEditor::undo() {
	int count, row, col; count = row = col = 0;
	string text = "";
	Undo::Action input = getUndo()->get(row, col, count, text);
	if (input == Undo::Action::ERROR)
		return;
	int dis = abs(m_editpos.row - row);
	//move to the start row
	//if we're behind the start line
	if (m_editpos.row < row)
	{
		for(int t = 0; t < dis; t++)
		{
			m_line++;
		}
	}
	//if we're in front of it
	else
	{
		for (int t = 0; t < dis; t++)
		{
			m_line--;
		}
	}
	m_editpos.col = col;
	m_editpos.row = row;
	switch(input)
	{
		case Undo::Action::DELETE:
		{
			(*m_line).erase(m_editpos.col, count);
			break;
		}
		case Undo::Action::INSERT:
		{
			(*m_line).insert(m_editpos.col, text);
			break;
		}
		case Undo::Action::JOIN:
		{
			if(m_editpos.row == m_list.size() - 1)
				return;
			auto iter = m_line;
			iter++;
			(*m_line) += (*iter);
			m_list.erase(iter);
			break;
		}
		case Undo::Action::SPLIT:
		{
			string firstHalf = m_line->substr(0, m_editpos.col);
			string secondHalf = m_line->substr(m_editpos.col);
			*m_line = firstHalf; //up until enter key stays on current line
			auto iter = m_line; //create a new line
			iter++;
			m_list.insert(iter, secondHalf);
			break;
		}
	}
}