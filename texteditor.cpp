#include <iostream>
#include <stack>
#include <vector>
#include <conio.h>    
#include <windows.h>   
using namespace std;

class TextEditor {
private:
    stack<char> leftStack;   // Text to the left of the cursor
    stack<char> rightStack;  // Text to the right of the cursor
    vector<stack<char>> lines;  // Each line is a separate stack
    stack<pair<char, string>> undoStack;  // Implemented a new stack for UNDO
    stack<pair<char, string>> redoStack;  // Implementing a new stack which stores REDO operations
    int currentLine = 0;  // Tracks the current line
    int cursorX = 0, cursorY = 0; // Tracks the cursor's x and y position (for console display)

    // Helper function to set cursor position in the console
    void setCursorPosition(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    // Helper function to display the current text
    void displayText() {
        system("cls"); // Clear screen to refresh the view

        for (int i = 0; i <= currentLine; i++) {
            stack<char> temp = lines[i];  // Copy line stack to reverse and display
            stack<char> reversedLeft;

            while (!temp.empty()) {
                reversedLeft.push(temp.top());
                temp.pop();
            }

            // Display the content of the current line
            while (!reversedLeft.empty()) {
                cout << reversedLeft.top();
                reversedLeft.pop();
            }

            // Insert cursor in the current line
            if (i == currentLine) {
                cout << "_";
                temp = rightStack;
                while (!temp.empty()) {
                    cout << temp.top();
                    temp.pop();
                }
            }
            cout << endl;
        }
    }

public:
    TextEditor() {
        // Initially start with one empty line
        lines.push_back(stack<char>());
    }

    // Insert a character at the current cursor position
    void insert(char ch) {
        leftStack.push(ch);
        undoStack.push({ch, "insert"});  // Track insertion for undo
        while (!redoStack.empty()) redoStack.pop();  // Clear redo stack
    }

    // Undo the last operation
    void undo() {
        if (!undoStack.empty()) {
            pair<char, string> lastAction = undoStack.top();
            undoStack.pop();

            if (lastAction.second == "insert") {
                if (!leftStack.empty()) {
                    char removedChar = leftStack.top();
                    leftStack.pop();
                    redoStack.push({removedChar, "insert"});  // Add to redo stack
                }
            }
            // For "delete", you would add characters back to the text (if you implement delete tracking)
        }
    }

    // Redo the last undone operation
    void redo() {
        if (!redoStack.empty()) {
            pair<char, string> lastAction = redoStack.top();
            redoStack.pop();

            if (lastAction.second == "insert") {
                leftStack.push(lastAction.first);
                undoStack.push({lastAction.first, "insert"});  // Add back to undo stack
            }
        }
    }

    // Backspace (delete character before the cursor)
    void backspace() {
        if (!leftStack.empty()) {
            char removedChar = leftStack.top();
            leftStack.pop();
            undoStack.push({removedChar, "delete"});  // Track deletion for undo
            while (!redoStack.empty()) redoStack.pop();  // Clear redo stack
        }
    }

    // Move the cursor to the left
    void moveCursorLeft() {
        if (!leftStack.empty()) {
            rightStack.push(leftStack.top());
            leftStack.pop();
        }
    }

    // Move the cursor to the right
    void moveCursorRight() {
        if (!rightStack.empty()) {
            leftStack.push(rightStack.top());
            rightStack.pop();
        }
    }

    // Move the cursor up
    void moveCursorUp() {
        if (currentLine > 0) {
            lines[currentLine] = leftStack;    // Store current line state
            currentLine--;                     // Move to the previous line
            leftStack = lines[currentLine];    // Load the previous line's leftStack
            rightStack = stack<char>();        // Clear the rightStack (cursor at end of previous line)
            cursorY = max(cursorY - 1, 0);     // Update cursor Y position
        }
    }

    // Move the cursor down
    void moveCursorDown() {
        if (currentLine < lines.size() - 1) {
            lines[currentLine] = leftStack;    // Store current line state
            currentLine++;                     // Move to the next line
            leftStack = lines[currentLine];    // Load the next line's leftStack
            rightStack = stack<char>();        // Clear the rightStack
            cursorY++;                         // Update cursor Y position
        }
    }

    // Insert a newline (Enter key)
    void insertNewLine() {
        lines[currentLine] = leftStack;   // Store current line's state
        currentLine++;                    // Move to the next line
        if (currentLine >= lines.size()) {
            lines.push_back(stack<char>()); // Create a new line if necessary
        }
        leftStack = stack<char>();        // Clear left stack for the new line
        rightStack = stack<char>();       // Clear right stack
        cursorX = 0;                      // Reset cursor position
        cursorY++;                        // Move cursor to the next line
    }

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
                            break;
                        case 83: // Delete key (ASCII code 83)
                            backspace();
                            break;
                        case 72:  // Up arrow key
                            moveCursorUp();
                            cursorY = max(cursorY - 1, 0);
                            break;
                        case 80:  // Down arrow key
                            moveCursorDown();
                            cursorY++;
                            break;
                    }
                } else if (ch == 8) { // Backspace key
                    backspace();
                    cursorX = max(cursorX - 1, 0);
                } else if (ch == 13) {  // Enter key (ASCII 13)
                    insertNewLine();
                } else if (ch == 27) {  // ESC key to quit
                    return;
                } else if (ch == 26) {  // CTRL+Z for undo
                    undo();
                } else if (ch == 25) {  // CTRL+Y for redo
                    redo();
                } else if (ch >= 32 && ch <= 126) {  // Printable characters
                    insert(ch);
                    cursorX++;
                }

                // Update the console display
                displayText();
                setCursorPosition(cursorX, cursorY);  // Update cursor position
            }
        }
    }
};

int main() {
    TextEditor editor;

    cout << "Hi! (Press ESC to quit)\n";
    editor.runEditor();  // Run the editor

    return 0;
}