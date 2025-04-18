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

const int MAX_UNDO_SIZE = 100; // Set a limit for the undo stack size
const int DEFAULT_COLOR = 7; // Default console color
const int SUGGESTION_COLOR = 10; // Green color for suggestions

long long int Word_Count = 0;   //Set initial word count as 0
const vector<int> Colours = {7,9,12,13,14};  //Default : 7(white)
                                                //Blue : 9
                                                //Red : 12
                                                //Magenta : 13
                                                //Yellow : 14                                                                                               
auto Colour_Itr = Colours.begin();    //To iterate through colours

class TextEditor {
private:
    stack<char> leftStack;   // Stores the text to the left of the cursor
    stack<char> rightStack;  // Stores the text to the right of the cursor
    vector<stack<char>> lines;  // Each line is a separate stack
    int currentLine = 0;        // Track which line the cursor is on
    int cursorX = 0, cursorY = 0; // Tracks the cursor's x and y position (for console display)

    vector<vector<stack<char>>> undoStack; // Undo stack to store previous states
    vector<vector<stack<char>>> redoStack; // Redo stack for redo functionality
    string filename;
    unordered_map<string,string> autocompleteWords = {
    {"add", "address"},
    {"adm", "administration"},
    {"agr", "agree"},
    {"ans", "answer"},
    {"app", "application"},
    {"arg", "argue"},
    {"assi", "assignment"},
    {"aut", "automatic"},
    {"beg", "beginning"},
    {"bel", "believe"},
    {"ben", "benefit"},
    {"bet", "between"},
    {"bro", "brother"},
    {"bu", "business"},
    {"cal", "calendar"},
    {"cap", "capacity"},
    {"cha", "character"},
    {"cho", "choice"},
    {"cla", "class"},
    {"cli", "client"},
    {"com", "communication"},
    {"con", "contract"},
    {"cor", "correction"},
    {"cou", "country"},
    {"cre", "credit"},
    {"dec", "decision"},
    {"del", "delivery"},
    {"dep", "department"},
    {"dev", "development"},
    {"dir", "direction"},
    {"dis", "discussion"},
    {"doc", "document"},
    {"dra", "draft"},
    {"edu", "education"},
    {"eff", "effect"},
    {"emp", "employee"},
    {"enc", "encourage"},
    {"equ", "equipment"},
    {"est", "establish"},
    {"eve", "event"},
    {"exp", "experience"},
    {"fin", "financial"},
    {"fol", "following"},
    {"for", "formation"},
    {"fun", "function"},
    {"gen", "general"},
    {"gro", "group"},
    {"gui", "guidance"},
    {"hea", "health"},
    {"his", "history"},
    {"ide", "idea"},
    {"imp", "important"},
    {"ind", "individual"},
    {"inf", "information"},
    {"int", "interest"},
    {"inv", "investment"},
    {"jud", "judgment"},
    {"jus", "justice"},
    {"lan", "language"},
    {"lap","laptop"},
    {"leg", "legal"},
    {"lev", "level"},
    {"lib", "library"},
    {"loc", "location"},
    {"man", "management"},
    {"mat", "material"},
    {"mea", "measure"},
    {"mem", "member"},
    {"met", "method"},
    {"mil", "military"},
    {"nat", "national"},
    {"nee", "necessary"},
    {"net", "network"},
    {"not", "notice"},
    {"obj", "object"},
    {"off", "office"},
    {"ope", "operation"},
    {"org", "organization"},
    {"par", "parent"},
    {"pat", "pattern"},
    {"per", "performance"},
    {"pla", "platform"},
    {"pol", "policy"},
    {"pos", "position"},
    {"pre", "presentation"},
    {"pro", "program"},
    {"pub", "public"},
    {"qui", "quickly"},
    {"rea", "reason"},
    {"rec", "recommend"},
    {"rel", "relationship"},
    {"rep", "report"},
    {"res", "resource"},
    {"res", "response"},
    {"rev", "review"},
    {"sec", "section"},
    {"ser", "service"},
    {"sig", "significant"},
    {"sim", "similar"},
    {"soc", "social"},
    {"sta", "standard"},
    {"str", "structure"},
    {"sys", "system"},
    {"the", "theory"},
    {"typ", "typical"},
    {"uni", "university"},
    {"val", "value"},
    {"vie", "view"},
    {"wor", "worker"}
    
};


    // Helper function to set cursor position in the console
    void setCursorPosition(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    
// Helper function to display the current text
void displayText() {
    system("cls"); // Clear the console
    for (int i = 0; i < lines.size(); i++) {
        stack<char> temp = lines[i];
        stack<char> reversedStack;

        // Reverse the stack to maintain original order of characters
        while (!temp.empty()) {
            reversedStack.push(temp.top());
            temp.pop();
        }

        // Print the line
        while (!reversedStack.empty()) {
            cout << reversedStack.top();
            reversedStack.pop();
        }

        // Check for autocomplete suggestion
        if (i == currentLine) {
            // Check if there's a suggestion
            string str = "";
            stack<char> tempStack = leftStack;
            while (!tempStack.empty() && tempStack.top() != ' ') {
                str += tempStack.top();
                tempStack.pop();
            }
            reverse(str.begin(), str.end());
            if (autocompleteWords.find(str) != autocompleteWords.end()) {
                // Display the suggestion in a different color
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), SUGGESTION_COLOR);
                cout << " *"; // Asterisk indicating an autocomplete suggestion
                cout << " " << autocompleteWords[str]; // Show the suggestion
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEFAULT_COLOR); // Reset color
            }

            cout << "_"; // Cursor position
            temp = rightStack;
            while (!temp.empty()) {
                cout << temp.top();
                temp.pop();
            }
        }
        cout << endl; // Move to the next line
    }

    // Set the cursor position at the end of the current line
    setCursorPosition(cursorX, cursorY);
}
    // Function to check if the state has changed
    bool isStateChanged() {
        if (undoStack.empty()) return true; // If the undo stack is empty, consider it a change

        const auto& lastState = undoStack.back();
        if (lastState.size() != lines.size()) return true; // If the number of lines is different

        for (size_t i = 0; i < lines.size(); ++i) {
            if (lines[i].size() != lastState[i].size()) return true; // Different line sizes

            stack<char> temp1 = lines[i];
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
        if (leftStack.empty()) return true;  // Capitalize at the start of a line

        // Check if the last character before the cursor is a period, space, or newline
        char lastChar = leftStack.top();
        if (lastChar == '.' || lastChar == '\n' || isspace(lastChar)) return true;

        return false;
    }


public:void updateTextFile() {
    ofstream file(filename); // Use the member variable for the filename
    for (const auto& lineStack : lines) {
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
    // Add line gaps between the text and word count
    for (int i = 0; i < 4; ++i) {
        file << '\n';
    }
    // Write the live word count at the end of the file
    file << "Current Word Count: " << word_count() << endl;
    file.close();
}
    TextEditor() {
    // Initially start with one empty line
    lines.push_back(stack<char>());
    undoStack.push_back(lines); // Initialize undo stack with the initial state

    // Determine the filename
    filename = "myDoc.txt"; // Initialize with the default filename
    int fileIndex = 1;

    // Check for existing files and increment the index
    while (ifstream(filename)) {
        filename = "myDoc" + to_string(fileIndex) + ".txt";
        fileIndex++;
    }

    // Create the file and write initial content
    ofstream file(filename);
    file.close(); // Close the file after creating it

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), *Colour_Itr);  // Set initial colour attribute to default
    updateTextFile(); // Update the text file with initial content
}

void insert_capital_i(stack<char>& stack) {
    vector<char> temp; //a temporary vector of three charecters to observe " i "/" i.?!"
    
    // Retrieve the last three characters from the stack
    if(stack.size()>=3){
        for(int i=0;i<3;i++){
            temp.push_back(stack.top());
            stack.pop();
        }
        // Check if we have " i " pattern
        if (temp[2] == ' ' && temp[1] == 'i' && (temp[0] == ' ' || temp[0] == '.' || temp[0] == '?' || temp[0] == '!')) {
            temp[1] = 'I';  // Capitalize 'i'
        }

        // Push the characters back to the stack in reverse order
        for (int i = 2; i >= 0; i--) {
            stack.push(temp[i]);
        }
    } else {
        // Push characters back if we didn’t find the pattern or stack size is less than 3
        for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
            stack.push(*it);
        }
    }
}


    // Insert a character at the current cursor position with auto-capitalization
    void insert_capital(char ch) {
        // Capitalize if it's the first character being inserted or follows punctuation
        bool caps = false; 
        if (leftStack.empty()) {
            caps = true;  // Capitalize the first character, matlab its the first charecter.
        } else {
            // Create a temporary stack to check the previous characters
            stack<char> tempStack = leftStack;
            char lastChar = tempStack.top();
            tempStack.pop();

            // Check if the last character is punctuation that should trigger capitalization
            if (lastChar == '\n' || lastChar == '.' || lastChar == '?' || lastChar == '!') {
                caps = true;
            } else if (lastChar == ' ' && !tempStack.empty()) {
                // Check if a period, question mark, or exclamation mark precedes a space
                char secondLastChar = tempStack.top();
                if (secondLastChar == '.' || secondLastChar == '?' || secondLastChar == '!') {
                    caps = true;
                }
            }
        }
        leftStack.push(caps ? toupper(ch) : ch);
        insert_capital_i(leftStack);

        // Update the states
        lines[currentLine] = leftStack;
        redoStack.clear(); // Clear the redo stack when a new character is inserted

        if (isStateChanged()) {
            undoStack.push_back(lines); // Push to the undo stack if the state has changed
        }

        updateTextFile();
}




    // Move the cursor to the left
    void moveCursorLeft() {
        if (!leftStack.empty()) {
            rightStack.push(leftStack.top());
            leftStack.pop();
            lines[currentLine] = leftStack;
        }
    }

    // Move the cursor to the right
    void moveCursorRight() {
        if (!rightStack.empty()) {
            leftStack.push(rightStack.top());
            rightStack.pop();
            lines[currentLine] = leftStack;
        }
    }

    // Move the cursor up
void moveCursorUp() {
    if (currentLine > 0) {
        // Store the current line state before moving up
        lines[currentLine] = leftStack;    
        currentLine--;                     // Move to the previous line
        leftStack = lines[currentLine];    // Load the previous line's leftStack
        rightStack = stack<char>();        // Clear the rightStack (cursor at end of previous line)
        cursorX = leftStack.size();        // Move the cursor to the end of the previous line
    }
    displayText(); // Refresh the display after moving the cursor
    setCursorPosition(cursorX, cursorY); // Update cursor position
}

    // Move the cursor down
    void moveCursorDown() {
        if (currentLine < lines.size() - 1) {
            lines[currentLine] = leftStack;    // Store current line state
            currentLine++;                     // Move to the next line
            leftStack = lines[currentLine];    // Load the next line's leftStack
            rightStack = stack<char>();        // Clear rightStack
            cursorY++;                         // Update cursor Y position
            cursorX = leftStack.size();        // Move the cursor to the end of the next line
        }
    }

    // Backspace (delete character before the cursor)
    void backspace() {
        if (!leftStack.empty()) {
            leftStack.pop();
            lines[currentLine] = leftStack;

            if (isStateChanged()) {
                undoStack.push_back(lines); // Push only if the state has changed
                // Limit the undo stack size
                if (undoStack.size() > MAX_UNDO_SIZE) {
                    undoStack.erase(undoStack.begin()); // Remove the oldest state if limit exceeded
                }
            }

            cursorX = max(cursorX - 1, 0);
        } else if (currentLine > 0) {
            cursorX = lines[currentLine - 1].size();  // Move cursor to the end of the previous line
            stack<char> previousLine = lines[currentLine - 1];
            while (!rightStack.empty()) {
                previousLine.push(rightStack.top());
                rightStack.pop();
            }
            lines[currentLine - 1] = previousLine;    // Merge the current line's rightStack with the previous line
            lines.erase(lines.begin() + currentLine); // Remove the current empty line
            currentLine--;                            // Move to the previous line
            leftStack = lines[currentLine];           // Load the left stack of the previous line
        }
        updateTextFile();
    }

    // Delete (delete character after the cursor)
    void deleteChar() {
        if (!rightStack.empty()) {
            rightStack.pop();
            redoStack.clear(); // Clear the redo stack when delete is used
        }
        updateTextFile();
    }

    // Insert a newline, capitalize that charecter
    void insertCapitalNewLine() {
    lines[currentLine] = leftStack;   // Store current line's state
    redoStack.clear(); // Clear the redo stack when a new line is inserted
    currentLine++;                    // Move to the next line
    if (currentLine >= lines.size()) {
        lines.push_back(stack<char>()); // Create a new line if necessary
    }
    leftStack = stack<char>();        // Clear left stack for the new line
    rightStack = stack<char>();       // Clear right stack
    cursorX = 0;                      // Reset cursor position
    cursorY++;                        // Move cursor to the next line

    updateTextFile();
}


    void undo() {
        if (undoStack.size() > 1) { // Check if there's an undo state available
            redoStack.push_back(lines);  // Store current state in redo before undoing
            undoStack.pop_back();        // Remove the current state
            lines = undoStack.back();    // Restore the previous state
            if (currentLine >= lines.size()) currentLine = lines.size() - 1;
            leftStack = lines[currentLine];
            rightStack = stack<char>();  // Clear the rightStack after undo
        }
        updateTextFile();
    }

    void redo() {
        if (!redoStack.empty()) {
            undoStack.push_back(lines);   // Save current state to undo stack
            lines = redoStack.back();     // Restore the redo state
            redoStack.pop_back();         // Remove the redo state
            if (currentLine >= lines.size()) currentLine = lines.size() - 1;
            leftStack = lines[currentLine];
            rightStack = stack<char>();  // Clear rightStack after redo
        }
        updateTextFile();
    }


     long long int word_count() {
    long long int count = 0; // Set initial count to 0
    string all_lines = ""; // Create a string to store all the data so far

    // Traverse through lines to get all data in one string
    for (const auto& lineStack : lines) {
        stack<char> tempStack = lineStack;
        stack<char> reversedStack;

        // Reverse the stack to maintain original order of characters
        while (!tempStack.empty()) {
            reversedStack.push(tempStack.top());
            tempStack.pop();
        }

        // Write characters from the reversed stack into the all_lines string
        while (!reversedStack.empty()) {
            all_lines += reversedStack.top();
            reversedStack.pop();
        }
        all_lines += '\n'; // Add a newline character after each line
    }

    // Use istringstream to count words
    istringstream stream(all_lines);
    string word;
    while (stream >> word) count++; // Count words

    return count;
}


    void setTextColor(int color)
    {
       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);  //Set text colour to given colour
    }

    // Main function to handle real-time editing
    // Main function to handle real-time editing
void runEditor() {
    system("cls");
    displayText();
    setCursorPosition(cursorX, cursorY);

    while (true) {
        if (_kbhit()) {  // Checks if the keyboard gave a signal
            int ch = _getch();

            if (ch == 224) {  // Special keys (arrows, delete)
                ch = _getch();  // Get the actual code

                switch (ch) {
                    case 75: // Left arrow key
                        moveCursorLeft();
                        cursorX = max(cursorX - 1, 0);
                        break;
                    case 77: // Right arrow key
                        moveCursorRight();
                        cursorX++;
                        if(rightStack.empty()) leftStack.push(' ');
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
            // else if (ch == 19) {  // Ctrl + S (Save)
            //     save();
            // }
            else if (ch == 27) {  // ESC key to exit
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Colours[0]);   //Revert to original colour
                break;
            }
            else if (ch==18)    //Ctrl + R (Change Colour)
            {
                Colour_Itr++;   //Move to next colour in the palette
                if(Colour_Itr==Colours.end()) Colour_Itr = Colours.begin();
                setTextColor(*Colour_Itr);      //Set colour to next colour
                displayText();
                setCursorPosition(cursorX, cursorY);
                continue;
            }
            else if (ch == 9) { // TAB key for autocomplete
    string str = "";
    stack<char> temp = leftStack;

    // Collect characters until the last space (or the beginning of the line)
    while (!temp.empty() && temp.top() != ' ') {
        str += temp.top(); // Append each character to str
        temp.pop();
    }

    // Reverse the extracted string to get the actual word
    reverse(str.begin(), str.end());

    // Check if the word exists in the hashmap
    if (autocompleteWords.find(str) != autocompleteWords.end()) {
        string suggestion = autocompleteWords[str];

        // Preserve the current right stack
        stack<char> tempRightStack = rightStack;

        // Remove the extracted word from leftStack
        for (int i = 0; i < str.size(); ++i) {
            leftStack.pop();
        }

        // Insert the suggestion into leftStack
        for (char c : suggestion) {
            leftStack.push(c);
        }

        // Update cursor position
        cursorX = leftStack.size(); // Move cursor to the end of the newly inserted suggestion

        // Restore the right stack
        rightStack = tempRightStack;

        lines[currentLine] = leftStack; // Update the current line
        
        // Set text color to the current color before displaying text
        setTextColor(*Colour_Itr);
        
        displayText();
        updateTextFile();
        setCursorPosition(cursorX, cursorY);
    }
}            else {  // Regular character input
                insert_capital(ch);
                cursorX++;
            }

            displayText();
            updateTextFile();
            setCursorPosition(cursorX, cursorY);
        }
    }
}
};

int main() {
    TextEditor editor;
    editor.runEditor();
    return 0;
}
