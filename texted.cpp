#include <iostream>
#include <stack>
#include <vector>
#include <conio.h>      // For _getch() and _kbhit()
#include <windows.h>    // For SetConsoleCursorPosition()
#include <fstream>
#include <cctype>       // For isspace() and toupper()
#include <unordered_map>
#include <algorithm>
#include<sstream>
using namespace std;
const int MAX_HISTORY = 100; // Limit for undo history
const int DEFAULT_CONSOLE_COLOR = 7; 
const int HIGHLIGHT_COLOR = 10; // Suggestion highlight color (Green)

// Spellchecker and Auto-Suggest 

struct Node {
    Node* links[26];
    bool endOfWord;

    Node() : endOfWord(false) {
        fill(begin(links), end(links), nullptr);
    }
};

class Dictionary {
public:
    Node* base;

    Dictionary() {
        base = new Node();
    }

    // Add word to the dictionary
    void addWord(const string& text) {
        Node* current = base;
        for (char c : text) {
            if (!islower(c)) continue;
            int idx = c - 'a';
            if (idx < 0 || idx >= 26) continue;

            if (current->links[idx] == nullptr) {
                current->links[idx] = new Node();
            }
            current = current->links[idx];
        }
        current->endOfWord = true;
    }

    // Check if word exists
    bool exists(const string& text) {
        Node* current = base;
        for (char c : text) {
            int idx = c - 'a';
            if (idx < 0 || idx >= 26 || current->links[idx] == nullptr)
                return false;
            current = current->links[idx];
        }
        return current->endOfWord;
    }

    // Helper for recursive suggestion collection
    void fetchSuggestions(Node* current, string& prefix, vector<string>& results) {
        if (current->endOfWord) results.push_back(prefix);

        for (int i = 0; i < 26 && results.size() < 5; ++i) {
            if (current->links[i] != nullptr) {
                prefix.push_back('a' + i);
                fetchSuggestions(current->links[i], prefix, results);
                prefix.pop_back();
            }
        }
    }

    // Generate suggestions based on prefix
    vector<string> suggest(const string& query) {
        Node* current = base;
        vector<string> results;
        for (char c : query) {
            int idx = c - 'a';
            if (idx < 0 || idx >= 26 || current->links[idx] == nullptr)
                return results;
            current = current->links[idx];
        }
        string tempPrefix = query;
        fetchSuggestions(current, tempPrefix, results);
        return results;
    }
};

// Load words from file into Dictionary
void initializeDictionary(Dictionary& dict, const string& filename) {
    ifstream file(filename);
    string input;
    while (getline(file, input)) {
        dict.addWord(input);
    }
    file.close();
}

// Spellcheck and offer suggestions
void verifyWord(Dictionary& dict, const string& originalWord, const string& outputPath) {
    ofstream log(outputPath, ios::app);

    string cleaned = originalWord;
    cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), ::ispunct), cleaned.end());
    transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::tolower);

    if (!dict.exists(cleaned)) {
        log << cleaned << " -> Suggestions:\n";
        vector<string> options = dict.suggest(cleaned.substr(0, 2));
        for (int i = 0; i < options.size(); ++i) {
            log << options[i] << " ";
        }
        log << "\n\n";
    }

    log.close();
}

// Global dictionary object
Dictionary wordBase;

// Text Editor Settings

long long int totalWords = 0;
const vector<int> ThemeColors = {7, 9, 10, 12, 13, 14}; // Theme: Default, Blue, Green, Red, Magenta, Yellow
auto currentColor = ThemeColors.begin();
class text_ed {
private:
    stack<char> left_st;     // Characters before the cursor
    stack<char> right_st;    // Characters after the cursor
    vector<stack<char>> textLines;  // One stack per line of text
    int activeLine = 0;         // Current line being edited
    int cursorPosX = 0, cursorPosY = 0; // Console cursor location

    vector<vector<stack<char>>> undoHistory; // Stores snapshots for undo
    vector<vector<stack<char>>> redoHistory; // Stores snapshots for redo

    unordered_map<string, string> auto_comp_word = {
        {"add", "address"}, {"adm", "administration"}, {"agr", "agree"}, {"ans", "answer"},
        {"app", "application"}, {"arg", "argue"}, {"assi", "assignment"}, {"aut", "automatic"},
        {"beg", "beginning"}, {"bel", "believe"}, {"ben", "benefit"}, {"bet", "between"},
        {"bro", "brother"}, {"bu", "business"}, {"cal", "calendar"}, {"cap", "capacity"},
        {"cha", "character"}, {"cho", "choice"}, {"cla", "class"}, {"cli", "client"},
        {"com", "communication"}, {"con", "contract"}, {"cor", "correction"}, {"cou", "country"},
        {"cre", "credit"}, {"dec", "decision"}, {"del", "delivery"}, {"dep", "department"},
        {"dev", "development"}, {"dir", "direction"}, {"dis", "discussion"}, {"doc", "document"},
        {"dra", "draft"}, {"edu", "education"}, {"eff", "effect"}, {"emp", "employee"},
        {"enc", "encourage"}, {"equ", "equipment"}, {"est", "establish"}, {"eve", "event"},
        {"exp", "experience"}, {"fin", "financial"}, {"fol", "following"}, {"for", "formation"},
        {"fun", "function"}, {"gen", "general"}, {"gro", "group"}, {"gui", "guidance"},
        {"hea", "health"}, {"his", "history"}, {"ide", "idea"}, {"imp", "important"},
        {"ind", "individual"}, {"inf", "information"}, {"int", "interest"}, {"inv", "investment"},
        {"jud", "judgment"}, {"jus", "justice"}, {"lan", "language"}, {"leg", "legal"},
        {"lev", "level"}, {"lib", "library"}, {"loc", "location"}, {"man", "management"},
        {"mat", "material"}, {"mea", "measure"}, {"mem", "member"}, {"met", "method"},
        {"mil", "military"}, {"nat", "national"}, {"nee", "necessary"}, {"net", "network"},
        {"not", "notice"}, {"obj", "object"}, {"off", "office"}, {"ope", "operation"},
        {"org", "organization"}, {"par", "parent"}, {"pat", "pattern"}, {"per", "performance"},
        {"pla", "platform"}, {"pol", "policy"}, {"pos", "position"}, {"pre", "presentation"},
        {"pro", "program"}, {"pub", "public"}, {"qui", "quickly"}, {"rea", "reason"},
        {"rec", "recommend"}, {"rel", "relationship"}, {"rep", "report"}, {"res", "response"},
        {"rev", "review"}, {"sec", "section"}, {"ser", "service"}, {"sig", "significant"},
        {"sim", "similar"}, {"soc", "social"}, {"sta", "standard"}, {"str", "structure"},
        {"sys", "system"}, {"the", "theory"}, {"typ", "typical"}, {"uni", "university"},
        {"val", "value"}, {"vie", "view"}, {"wor", "worker"}
    };

    // Utility to position the console cursor
    void moveConsoleCursor(int x, int y) {
        COORD location;
        location.X = x;
        location.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), location);
    }

    // (Remaining editor methods would be added here)
};
// Helper function to display the current text
void displayText() {
    system("cls");
    for (int i = 0; i <= activeLine; i++) {
        stack<char> temp = textLines[i];
        stack<char> reversedLeft;
        while (!temp.empty()) {
            reversedLeft.push(temp.top());
            temp.pop();
        }
        while (!reversedLeft.empty()) {
            cout << reversedLeft.top();
            reversedLeft.pop();
        }

        // Display suggestion if we’re on the active line
        if (i == activeLine) {
            string str = "";
            stack<char> tempStack = left_st;
            while (!tempStack.empty() && tempStack.top() != ' ') {
                str += tempStack.top();
                tempStack.pop();
            }
            reverse(str.begin(), str.end());

            if (auto_comp_word.find(str) != auto_comp_word.end()) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), SUGGESTION_COLOR);
                cout << " *";  // Suggestion indicator
                cout << " " << auto_comp_word[str];  // Suggested word
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEFAULT_COLOR);  // Reset to normal
            }

            cout << "_";  // Cursor position
            temp = right_st;
            while (!temp.empty()) {
                cout << temp.top();
                temp.pop();
            }
        }
        cout << endl;
    }
}
// Function to check if the state has changed
bool isStateChanged() {
    if (undoStack.empty()) return true; // If the undo stack is empty, consider it a change

    const auto& lastState = undoStack.back();
    if (lastState.size() != textLines.size()) return true; // If the number of lines is different

    for (size_t i = 0; i < textLines.size(); ++i) {
        if (textLines[i].size() != lastState[i].size()) return true; // Different line sizes

        stack<char> temp1 = textLines[i];
        stack<char> temp2 = lastState[i];
        while (!temp1.empty() && !temp2.empty()) {
            if (temp1.top() != temp2.top()) return true; // Characters differ
            temp1.pop();
            temp2.pop();
        }

        // If one stack is empty and the other is not, consider it a change
        if (!temp1.empty() || !temp2.empty()) return true;
    }

    return false; // No changes detected
}
// Helper function to check if the character should be capitalized
bool shouldCapitalize() {
    if (left_st.empty()) return true;  // Capitalize at the start of a line

    // Check if the last character before the cursor is a period, space, or newline
    char lastChar = left_st.top();
    if (lastChar == '.' || lastChar == '\n' || isspace(lastChar)) return true;

    return false;
}
public:
    text_ed() {
        // Initially start with one empty line
        textLines.push_back(stack<char>());
        undoStack.push_back(textLines); // Initialize undo stack with the initial state
        
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), *Colour_Itr);  // Set initial colour attribute to default
        
        FILE *fp;
        fp = fopen("word_count.txt","w");   
        if(!fp) 
        {
            cout << "File did not open" << endl;
            exit(1);
        }
        fprintf(fp, "Current Word Count: %lld", Word_Count);  // Initialize the word count 0 right when text_ed object is created.
        fclose(fp);    
    }
// Insert a character at the current cursor position with auto-capitalization
void insert_capital(char ch) {
    // Capitalize if it's the first character being inserted or follows a newline or period, only works "." and ". " or multiple ".... "
    bool caps = false; // caps stores the state, whether to capitalize or not
    // Check if the left stack is empty which implies that it is the first character
    if (left_st.empty()) {
        caps = true;  // initialize state to be capitalized
    } else {
        // Create a temporary stack to check the last characters as we cannot access the elements before the top.
        stack<char> tempStack = left_st;
        char lastChar = tempStack.top();
        tempStack.pop();

        // Check if the last character is a newline or a full stop
        if (lastChar == '\n' || lastChar == '.' || lastChar == '?' || lastChar == '!') {
            caps = true; // Capitalize the next character
        } else if (lastChar == ' ') {
            // Check the second last character (if it exists)
            if (!tempStack.empty() && (tempStack.top() == '.' || tempStack.top() == '!' || tempStack.top() == '?')) {
                caps = true; // Capitalize after a space following a period
            }
        }
    }
}
// Insert the character (capitalize if needed)
if (caps) {
    left_st.push(toupper(ch));  // Capitalize the current character
} else {
    left_st.push(ch);  // Keep the character as is
}

textLines[currentLine] = left_st;
redoStack.clear(); // Clear the redo stack when a new character is inserted

if (isStateChanged()) {
    undoStack.push_back(textLines); // Push only if the state has changed
}
// Move the cursor to the left
void moveCursorLeft() {
    if (!left_st.empty()) {
        right_st.push(left_st.top());
        left_st.pop();
        textLines[currentLine] = left_st;  // Update the current line with the new left stack
    }
}

// Move the cursor to the right
void moveCursorRight() {
    if (!right_st.empty()) {
        left_st.push(right_st.top());
        right_st.pop();
        textLines[currentLine] = left_st;  // Update the current line with the new left stack
    }
}
// Move the cursor up
void moveCursorUp() {
    if (currentLine > 0) {
        textLines[currentLine] = left_st;    // Store current line state
        currentLine--;                       // Move to the previous line
        left_st = textLines[currentLine];    // Load the previous line's left_st
        right_st = stack<char>();            // Clear the right_st (cursor at end of previous line)
        cursorY = max(cursorY - 1, 0);       // Update cursor Y position
        cursorX = left_st.size();            // Move the cursor to the end of the previous line
    }
}

// Move the cursor down
void moveCursorDown() {
    if (currentLine < textLines.size() - 1) {
        textLines[currentLine] = left_st;    // Store current line state
        currentLine++;                       // Move to the next line
        left_st = textLines[currentLine];    // Load the next line's left_st
        right_st = stack<char>();            // Clear right_st
        cursorY++;                           // Update cursor Y position
        cursorX = left_st.size();            // Move the cursor to the end of the next line
    }
}
// Backspace (delete character before the cursor)
void backspace() {
    if (!left_st.empty()) {
        left_st.pop();
        textLines[currentLine] = left_st;

        if (isStateChanged()) {
            undoStack.push_back(textLines); // Push only if the state has changed
            // Limit the undo stack size
            if (undoStack.size() > MAX_UNDO_SIZE) {
                undoStack.erase(undoStack.begin()); // Remove the oldest state if limit exceeded
            }
        }

        cursorX = max(cursorX - 1, 0);
    } else if (currentLine > 0) {
        cursorX = textLines[currentLine - 1].size();  // Move cursor to the end of the previous line
        stack<char> previousLine = textLines[currentLine - 1];
        while (!right_st.empty()) {
            previousLine.push(right_st.top());
            right_st.pop();
        }
        textLines[currentLine - 1] = previousLine;    // Merge the current line's right_st with the previous line
        textLines.erase(textLines.begin() + currentLine); // Remove the current empty line
        currentLine--;                               // Move to the previous line
        left_st = textLines[currentLine];             // Load the left_st of the previous line
    }
}
// Delete (delete character after the cursor)
void deleteChar() {
    if (!right_st.empty()) {
        right_st.pop();
        redoStack.clear(); // Clear the redo stack when delete is used
    }
}
// Insert a newline, capitalize that character
void insertCapitalNewLine() {
    textLines[currentLine] = left_st;   // Store current line's state
    redoStack.clear(); // Clear the redo stack when a new line is inserted
    currentLine++;                    // Move to the next line
    if (currentLine >= textLines.size()) {
        textLines.push_back(stack<char>()); // Create a new line if necessary
    }
    left_st = stack<char>();        // Clear left stack for the new line
    right_st = stack<char>();       // Clear right stack
    cursorPosX = 0;                      // Reset cursor position
    cursorPosY++;                        // Move cursor to the next line
}
// Undo the last operation
void undo() {
    if (undoStack.size() > 1) { // Check if there's an undo state available
        redoStack.push_back(textLines);  // Store current state in redo before undoing
        undoStack.pop_back();             // Remove the current state
        textLines = undoStack.back();    // Restore the previous state
        if (activeLine >= textLines.size()) activeLine = textLines.size() - 1;
        left_st = textLines[activeLine];
        right_st = stack<char>();        // Clear the right stack after undo
    }
}

// Redo the last undone operation
void redo() {
    if (!redoStack.empty()) {
        undoStack.push_back(textLines);   // Save current state to undo stack
        textLines = redoStack.back();     // Restore the redo state
        redoStack.pop_back();             // Remove the redo state
        if (activeLine >= textLines.size()) activeLine = textLines.size() - 1;
        left_st = textLines[activeLine];
        right_st = stack<char>();         // Clear right stack after redo
    }
}
// Save the content of the text editor to a file
void save() {
    ofstream file("myDoc.txt");
    for (const auto& lineStack : textLines) {
        // We need to output characters in the order they were added, so reverse the stack
        std::stack<char> tempStack = lineStack;  // Make a copy of the current stack
        std::stack<char> reverseStack;

        // Reverse the stack to maintain original order of characters
        while (!tempStack.empty()) {
            reverseStack.push(tempStack.top());
            tempStack.pop();
        }

        // Write characters from the reversed stack into the file
        while (!reverseStack.empty()) {
            file << reverseStack.top();
            reverseStack.pop();
        }
        file << '\n';
    }
    file.close();
}
long long int word_count() {
    if (textLines[0].empty()) return 0;      // If there is no data yet, there are 0 words
    long long int count = 0;        // Set initial count to 0
    string all_lines = "";          // Create a string to store all the data so far

    for (int i = textLines.size() - 1; i >= 0; i--) {      // Traverse through lines to get all data in one string
        stack<char> flag = textLines[i];            
        char curr;
        if (!flag.empty()) curr = flag.top();

        while (!flag.empty()) {
            all_lines += curr;
            curr = flag.top();
            flag.pop();
        }
    }    

    istringstream stream(all_lines);    // Define a stream from all_lines
    string word;
    while (stream >> word) {
        count++;  // Take a word as input from the stream and keep count
    }        
    return count;                       
}
void display_word_count() {
    Word_Count = word_count();  
    FILE* fp;
    fp = fopen("word_count.txt", "w");   
    if (!fp) {
        cout << "File did not open" << endl;
        exit(1);
    }
    fprintf(fp, "Current Word Count: %lld", Word_Count);  // Store the live word count
    fclose(fp);
}
void setTextColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);  // Set text colour to given colour
}

// Main function to handle real-time editing
void runEditor() {
    system("cls");
    displayText();
    setCursorPosition(cursorPosX, cursorPosY); // Updated cursor variable names

    while (true) {
        if (_kbhit()) {  // Checks if the keyboard gave a signal
            int ch = _getch();

            if (ch == 224) {  // Special keys (arrows, delete)
                ch = _getch();  // Get the actual code

                switch (ch) {
                    case 75: // Left arrow key
                        moveCursorLeft();
                        cursorPosX = max(cursorPosX - 1, 0);
                        break;
                    case 77: // Right arrow key
                        moveCursorRight();
                        cursorPosX++;
                        if (right_st.empty()) left_st.push(' '); // Updated stack names
                        break;
                    case 72: // Up arrow key
                        moveCursorUp();
                        break;
                    case 80: // Down arrow key
                        moveCursorDown();
                        break;
                    case 83: // Delete key (ASCII code 83)
                        deleteChar();
                        break;
                }
            }
            else if (ch == 8) {  // Backspace
                backspace();
            }
            else if (ch == 13) {  // Enter key
                insertCapitalNewLine();
            }
            else if (ch == 26) {  // Ctrl + Z (Undo)
                undo();
            }
            else if (ch == 25) {  // Ctrl + Y (Redo)
                redo();
            }
            else if (ch == 19) {  // Ctrl + S (Save)
                save();
            }
            else if (ch == 27) {  // ESC key to exit
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Colours[0]);   // Revert to original colour
                
                FILE *fp;
                fp = fopen("word_count.txt", "w");   
                if (!fp) {
                    cout << "File did not open" << endl;
                    exit(1);
                }
                fprintf(fp, "Current Word Count: %d", 0);  // Clear the word count to zero again.
                fclose(fp);    
                break;
            }
            else if (ch == 18) {    // Ctrl + R (Change Colour)
                Colour_Itr++;   // Move to next colour in the palette
                if (Colour_Itr == Colours.end()) Colour_Itr = Colours.begin();
                setTextColor(*Colour_Itr);      // Set colour to next colour
                continue;
            }
            else if (ch == 32) {  // Space for autocorrect
                string str = "";
                stack<char> temp = left_st;

                // Collect characters until the last space (or the beginning of the line)
                while (!temp.empty() && temp.top() != ' ') {
                    str += temp.top(); // Append each character to str
                    temp.pop();
                }
                // Reverse the extracted string to get the actual word
                reverse(str.begin(), str.end());
                transform(str.begin(), str.end(), str.begin(), ::tolower);
                spellcheckAndSuggest(trie, str, "C:dictionary.txt");
                left_st.push(' ');
                cursorPosX++;
            }

            else if (ch == 9) { // TAB key for autocomplete
                string str = "";
                stack<char> temp = left_st;

                // Collect characters until the last space (or the beginning of the line)
                while (!temp.empty() && temp.top() != ' ') {
                    str += temp.top(); // Append each character to str
                    temp.pop();
                }

                // Reverse the extracted string to get the actual word
                reverse(str.begin(), str.end());
                transform(str.begin(), str.end(), str.begin(), ::tolower);
                spellcheckAndSuggest(trie, str, "C:dictionary.txt");
                cursorPosX += str.length(); // Move cursor to the end of the word
            }
        }
    }
}
// Check if the word exists in the hashmap
if (auto_comp_word.find(str) != auto_comp_word.end()) {  // Updated variable name
    string suggestion = auto_comp_word[str];  // Fetch the suggestion from the map

    // Preserve the current right stack
    stack<char> tempRight_st = right_st;  // Renamed rightStack to right_st

    // Remove the extracted word from leftStack
    for (int i = 0; i < str.size(); ++i) {
        left_st.pop();  // Updated leftStack to left_st
    }

    // Insert the suggestion into leftStack
    for (char c : suggestion) {
        left_st.push(c);  // Insert each character of the suggestion
    }

    // Update cursor position
    cursorPosX = left_st.size();  // Move cursor to the end of the newly inserted suggestion

    // Restore the right stack
    right_st = tempRight_st;  // Restore the original rightStack

    lines[currentLine] = left_st;  // Update the current line with the modified leftStack
    displayText();
    display_word_count();
    setCursorPosition(cursorPosX, cursorPosY);  // Update the cursor position
}
else {  // Regular character input
    insert_capital(ch);  // Insert character with capitalization if needed
    cursorPosX++;  // Increment cursor position
}

displayText();  // Redraw the text after insertion
display_word_count();  // Update word count
setCursorPosition(cursorPosX, cursorPosY);  // Adjust cursor position

int main() {
    loadDictionary(trie, "C:dictionary.txt");  // Load words from dictionary.txt
    text_ed editor;  // Changed TextEditor to text_ed
    editor.runEditor();  // Start the text editor
    return 0;
}

