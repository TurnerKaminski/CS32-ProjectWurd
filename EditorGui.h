#ifndef _EDITORGUI_H_
#define _EDITORGUI_H_

// Contains the class for our Editor Graphical User Interface (GUI). This class is used to run the overall
// text editor, incorporating your core text editor, spell checker, and your undo system. We provide this
// code.
// 
// ** Students must NOT modify this file as YOU WILL NOT TURN THIS FILE IN WITH YOUR PROJECT!

#include "Undo.h"
#include "TextEditor.h"
#include "SpellCheck.h"
#include "TextIO.h"
class EditorGui {
public:
	// Construct our text editor GUI which orchestrates all aspects of text editing.
	// rows: # of rows in the text editor window
	// cols: # of columns in the tetx editor window
	// filename: A path/filename used to load up a file upon initialization.
	EditorGui(int rows, int cols) {
		undo_ = createUndo();
		te_ = createTextEditor(undo_);
		spell_check_ = createSpellCheck();
		rows_ = rows - 1; // leave the last row for status/loading files.
		cols_ = cols;
		top_ = 0;
		left_ = 0;
		loaded_dictionary_ = false;
	}

	// EditorGui destructor.
	~EditorGui() {
		delete te_;
		delete undo_;
		delete spell_check_;
	}

	// Used to load the specified dictionary.
	// dictionary: The fill path and filename of the dictionary.txt file, e.g., c:\cs32\proj4\dictionary.txt
	// Returns true if the dictionary was successfully loaded.
	bool loadDictionary(const std::string& dictionary) {
		if (spell_check_->load(dictionary))
			loaded_dictionary_ = true;

		return loaded_dictionary_;
	}

	void promptAndLoadDictionary() {
		std::string dictionary;
		if (getInput("Enter dictionary path/filename: ", dictionary)) {
			if (loadDictionary(dictionary)) {
				writeStatus("Loaded dictionary successfully!");
			}
			else
				writeStatus("Unable to load dictionary.");
		}
		else
			writeStatus("No dictionary entered.");
		redisplayTheEditorWindowAndPositionCursor(false);
	}

	// Used to load a text file into your text editor.
	// file_to_load: The name of the file to load.
	void loadFileToEdit(const std::string& file_to_load = "") {
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		std::string filename = file_to_load;

		// If the user didn't pass in a filename, then we're going to prompt them on the screen
		// for a valid path/filename to load.
		if (filename.empty()) {
			const bool got_file = getInput("Enter path/file to load: ", filename);
			if (!got_file) {
				redisplayTheEditorWindowAndPositionCursor(false);
				return;
			}
		}

		// Load the file and display the appropriate status (success/fail) on the screen's status line.
		const bool loaded = te_->load(filename);
		if (loaded) {
			filename_ = filename;
			resetCursorToTopOfFile();
			writeStatus("Loaded file successfully!");
			redisplayTheEditorWindowAndPositionCursor(false);
		}
		else
			writeStatus("Unable to load file.");
	}

	// Run our main text editor. When this function returns, it means the user decided to quit/exit
	// from the editor.
	void run() {
		bool cont;
		do {
			const int ch = TextIO::getChar();
			cont = processKey(ch);
		} while (cont);
	}

	// Print the status line on the bottom of the screen, overwriting other text that might have been there before.
	// line: The status line to display.
	void writeStatus(const std::string& line) {
		TextIO::move(rows_, 0);
		if (line.length() <= cols_) {
			TextIO::print(line);
			TextIO::print(std::string(cols_ - line.length(), ' '));
		}
		else
			TextIO::print(line.substr(0, cols_));
	}

private:

	// Process each key that the user presses and call the appropriate function in the student's
	// editor class.
	// ch: The character that was pressed (e.g., a letter, backspace, tab, enter, delete, ctrl-L, ctrl-S, ctrl-X).
	// Returns true if the user wants to keep editing, and false if they want to quit editing (Ctrl-X).
	bool processKey(const int ch) {
		switch (ch) {
		case KEY_UP:
			te_->move(TextEditor::Dir::UP);
			break;
		case KEY_DOWN:
			te_->move(TextEditor::Dir::DOWN);
			break;
		case KEY_LEFT:
			te_->move(TextEditor::Dir::LEFT);
			break;
		case KEY_RIGHT:
			te_->move(TextEditor::Dir::RIGHT);
			break;
		case KEY_HOME:	// Home key was hit; move to front of line
			te_->move(TextEditor::Dir::HOME);
			break;
		case KEY_END:	// End key was hit; move to end of line
			te_->move(TextEditor::Dir::END);
			break;
		case KEY_PPAGE:	// Previous page (page up)
			prevPage();
			break;
		case KEY_NPAGE:	// Next page (page down)
			nextPage();
			break;
		case KEY_DC:	// Delete key was hit
			te_->del();
			break;
		case KEY_BACKSPACE:
			te_->backspace();
			break;
		case KEY_ENTER:
			te_->enter();
			break;
		case CTRL_S:	// Save the current changes
			save();
			return true;
		case CTRL_L:	// Throw away existing file and load up a new file
			loadFileToEdit();
			return true;
		case CTRL_Z:	// Undo last change
			te_->undo();
			break;
		case CTRL_D:
			promptAndLoadDictionary();
			break;
		case CTRL_X:
			if (quit()) return false;
			break;
		default:
			// A regular key was hit (e.g., qwerty); insert it into the document.
			if (ch < 256) te_->insert(static_cast<char>(ch));
			break;
		}
		redisplayTheEditorWindowAndPositionCursor();
		return true;
	}

	// This addresses a page-up keypress, moving the window up by one screen's worth.
	void prevPage() {
		int cursor_dist_from_top = getCurDistFromTopRow();

		// Send a cursor up command to the editor for each of the rows on the screen.
		for (int i = 0; i < rows_; ++i)
			te_->move(TextEditor::Dir::UP);

		// Make sure the GUI positions the cursor on the proper row of the screen.
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		top_ = cur_row - cursor_dist_from_top;
		if (top_ < 0) top_ = 0;
	}

	// This addresses a page-down keypress, moving the window down by one screen's worth.
	void nextPage() {
		int cursor_dist_from_top = getCurDistFromTopRow();

		// Send a cursor down command to the editor for each of the rows on the screen.
		for (int i = 0; i < rows_; ++i)
			te_->move(TextEditor::Dir::DOWN);

		// Make sure the GUI positions the cursor on the proper row of the screen.
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		top_ = cur_row - cursor_dist_from_top;
		if (top_ < 0) top_ = 0;
	}

	// Get the distance from the top of the screen to the current row where the cursor
	// is being displayed. 
	// Returns the vertical distance of the user's cursor in the editor from the top of the screen.
	int getCurDistFromTopRow() const {
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		return cur_row - top_;
	}

	// Get a list of spelling suggestions for the word in the editor that the cursor is currently
	// positioned on top of. If the word is spelled correctly, this returns the empty string.
	// Otherwise it returns a string like: "Spelling suggestions: apple, ample" if there are
	// suggestions or "No spelling suggestions." if there are no suggestions.
	// Returns the suggestion string.
	std::string getSuggestionString() {
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		std::vector<std::string> lines;
		te_->getLines(cur_row, 1, lines);
		if (lines.empty()) return "";  // empty line
		if (cur_col >= lines[0].length()) return ""; // at end of line
		if (!isWordChar(lines[0][cur_col])) return "";  // not on a word

		// Extract the full word that the cursor is sitting on.
		while (cur_col >= 0 && isWordChar(lines[0][cur_col]))
			--cur_col;
		++cur_col;
		std::string cur_word;
		while (cur_col != lines[0].length() && isWordChar(lines[0][cur_col])) {
			cur_word += lines[0][cur_col];
			++cur_col;
		}

		// Ask the student's spell checker if the word is spelled correctly, and if not
		// for up to kNumSuggestions suggestions.
		const int kNumSuggestions = 20;
		std::vector<std::string> suggestions;
		suggestions.clear();
		if (spell_check_->spellCheck(cur_word, kNumSuggestions, suggestions)) return "";

		// Create a string with the suggestions (if any).
		std::string sugg_line;
		std::string sugg_base = "Spelling suggestions: ";
		const int kCommaLength = 2;
		for (const auto& s : suggestions) {
			if (sugg_line.length() + s.length() + sugg_base.length() + kCommaLength <= cols_) {
				if (!sugg_line.empty()) sugg_line += ", ";
				sugg_line += s;
			}
		}

		if (sugg_line.empty()) return "No spelling suggestions.";

		return sugg_base + sugg_line;
	}

	// Check to see if a character is part of a word. This includes all letters as well as the apostrophe
	// character ' right now. You may wish to expand this to include hyphens in the future.
	// ch: The character to check.
	// Returns true if the character is one that's considered part of a word.
	bool isWordChar(const char ch) {
		if (ch < 0) return false;
		return isalpha(ch) || ch == '\'';
	}

	// Redisplay the entire editor window (all text being edited, in white and red) and then
	// reposition the cursor in the right place after displaying all of the text.
	// clear_status_line: If true, this causes the function to clear the status line at
	// the bottom of the screen.
	void redisplayTheEditorWindowAndPositionCursor(bool clear_status_line = true) {

		// Compute how the screen should have shifted based on the keypress (e.g., pg-up, down-arrow)
		// This is not as trivial as it seems. For example, a left key-press doesn't always just take
		// you to the left edge of the text. If you're at the front of a line, a left key press will
		// take you to the end of the line above.
		int dist_from_top = getCurDistFromTopRow();
		if (dist_from_top < 0) {
			top_ += dist_from_top;
			dist_from_top = 0;
		}
		else if (dist_from_top >= rows_) {
			top_ += (dist_from_top - rows_ + 1);
			dist_from_top -= (dist_from_top - rows_ + 1);
		}

		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		int dist_from_left = cur_col - left_;
		if (dist_from_left < 0) {
			if (cur_col < cols_)
				left_ = 0;
			else
				left_ += dist_from_left;
			dist_from_left = cur_col - left_;
		}
		else if (dist_from_left >= cols_) {
			left_ += (dist_from_left - cols_) + 1;
			dist_from_left = cur_col - left_;
		}

		// Obtain the relevant lines from the student's Text Editor class and display them
		// on the screen, displaying blank lines as filler at the end of the current file.
		std::vector<std::string> lines;
		te_->getLines(top_, rows_, lines);
		for (int i = 0; i < rows_; ++i) {
			if (i < lines.size()) {
				writeLine(i, lines[i]);
			}
			else {
				clearLine(i);
			}
		}
		// If instructed to do so, clear the status line at the bottom of the screen.
		if (clear_status_line) clearLine(rows_);
		// If the cursor is on a misspelled word, then display spelling suggestions (if there
		// are any) at the bottom of the screen.
		displaySpellingSuggestionsIfNecessary();

		// Reposition the cursor on the line where the user was editing.
		TextIO::move(dist_from_top, dist_from_left);
	}

	// Display correct spellings for the current word (that the cursor is on) if there are any
	// spelling suggestions (and only if it's misspelled).
	void displaySpellingSuggestionsIfNecessary() {
		const std::string suggestions = getSuggestionString();
		TextIO::move(rows_, 0);
		TextIO::print(suggestions, TextIO::COLOR::RED);
	}

	// Compute a pattern of spaces and asterisks for the current line indicating where spelling
	// mistakes were found. A space indicates a spot where a word is spelled properly, and an
	// asterisk indicates that the letter is part of a word that's spelled improperly. e.g.:
	// For this line:    "Thys is spelt wrong."
	// Would yield this: "****    *****       " 
	// This is used by the GUI to hilight misspellings in red.
	// line: The input line from the text editor
	// prob_str: The spaces and asterisks that show the locations of the spelling mistakes.
	void produceBadPattern(const std::string& line, std::string& prob_str) {
		if (line.empty()) return;
		// Create a string of all spaces that is the same length of the input line. We start by
		// assuming all words are spelled correctly.
		prob_str = std::string(line.length(), kGoodChar);
		if (loaded_dictionary_) {
			std::vector<SpellCheck::Position> problems;
			// Get a list of all problems on the specified line.
			spell_check_->spellCheckLine(line, problems);
			// Add asterisks to problem spots in the string.
			for (const auto& p : problems) {
				for (int i = p.start; i <= p.end; ++i)
					prob_str[i] = kBadChar;
			}
		}
	}

	// Write a line to the console at the specified location, optionally hilighting
	// misspelled words in red.
	// row: What row of the screen to print the line on.
	// line: The line to output
	void writeLine(int row, const std::string& line) {
		std::string prob_str;
		produceBadPattern(line, prob_str);

		TextIO::move(row, 0);
		// Determine what to actually print out. Since lines can be very long, we need to compute
		// what columns of the line is currently being displayed within the GUI.
		std::string print_me;
		if (line.length() >= left_) {
			print_me = line.substr(left_, cols_);
			prob_str = prob_str.substr(left_, cols_);
		}
		// Pad with spaces as necessary to overwrite other text from before.
		if (cols_ > print_me.length()) {
			print_me.insert(print_me.length(), cols_ - print_me.length(), ' ');
			prob_str.insert(prob_str.length(), cols_ - prob_str.length(), ' ');
		}
		// Print the text in white/red to hilight errors.
		for (int i = 0; i < print_me.length(); ++i)
			TextIO::print(print_me[i], prob_str[i] == kBadChar ? TextIO::COLOR::RED : TextIO::COLOR::WHITE);
	}

	// Display a prompt and get some input from the user (like a filename) on the status line.
	// prompt: What to display to the user, e.g. "Quit [y/N]?"
	// input: The result that the user typed
	// Returns true if the user typed something other than a blank line.
	bool getInput(const std::string& prompt, std::string& input) {
		writeStatus(prompt);
		TextIO::move(rows_, static_cast<int>(prompt.length()));
		TextIO::getString(input);
		clearLine(rows_);

		return !input.empty();
	}

	// Clears the specified row on the screen.
	void clearLine(const int row) const {
		TextIO::move(row, 0);
		const std::string empty(cols_, ' ');
		TextIO::print(empty);
	}

	// Lets the user save the current edited text into the edited file or a new file if one has not yet been specified.
	void save() {
		// Get the current position of the user's cursor so we can restore it after saving the file.
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);

		// The user has not yet specified a filename.
		if (filename_.empty()) {
			std::string filename;
			const bool got_file = getInput("Enter path/file to save to: ", filename);
			if (!got_file) {
				TextIO::move(cur_row, cur_col);
				return;
			}
			filename_ = filename;
		}
		else {
			// The user will overwrite an existing file.
			std::string input;
			const std::string prompt = "Save to file " + filename_ + " [Y/n]: ";
			getInput(prompt, input);
			if (!input.empty() && (input[0] != 'y' && input[0] != 'Y')) {
				writeStatus("Not saving file.");
				TextIO::move(cur_row, cur_col);
				return;
			}
		}

		// Save the current text to the specified file.
		const bool saved = te_->save(filename_);
		if (saved)
			writeStatus("Saved file successfully!");
		else
			writeStatus("Unable to save file.");

		// Place the cursor back on the proper row where the user was editing.
		TextIO::move(cur_row, cur_col);
	}

	// Check to see if the user really wants to exit the editor.
	// Returns true if the user wants to exit, false otherwise.
	bool quit() {
		int cur_row, cur_col;
		te_->getPos(cur_row, cur_col);
		std::string input;
		const bool got_input = getInput("Quit [y/N]: ", input);
		if (got_input && (input[0] == 'y' || input[0] == 'Y')) return true;
		TextIO::move(cur_row, cur_col);
		return false;
	}

	// Places the user cursor at the top of the file.
	void resetCursorToTopOfFile() {
		top_ = left_ = 0;
	}

	// Private variables and constants.
	static const char kGoodChar = ' ', kBadChar = '*';
	std::string filename_;
	TextEditor* te_;
	Undo* undo_;
	SpellCheck* spell_check_;
	bool loaded_dictionary_;
	int top_, left_;
	int rows_, cols_;
};

#endif // #ifndef _EDITORGUI_H_
