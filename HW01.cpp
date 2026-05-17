#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <cctype>

using namespace std;

// ==========================================
// 1. 實作 Linked List 節點與 Stack (Template)
// ==========================================
template <typename T>
struct Node {
    T data;
    Node* next;
    Node(T val) : data(val), next(nullptr) {}
};

template <typename T>
class LinkedListStack {
private:
    Node<T>* topNode;

public:
    LinkedListStack() : topNode(nullptr) {}

    ~LinkedListStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    void push(T val) {
        Node<T>* newNode = new Node<T>(val);
        newNode->next = topNode;
        topNode = newNode;
    }

    void pop() {
        if (isEmpty()) {
            throw runtime_error("錯誤: 堆疊下溢 (Stack underflow)。");
        }
        Node<T>* temp = topNode;
        topNode = topNode->next;
        delete temp;
    }

    T peek() const {
        if (isEmpty()) {
            throw runtime_error("錯誤: 堆疊為空 (Stack is empty)。");
        }
        return topNode->data;
    }

    bool isEmpty() const {
        return topNode == nullptr;
    }
};

// ==========================================
// 2. 輔助函式：判斷運算子與優先級
// ==========================================
bool isOperator(const string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || token == "%";
}

int getPrecedence(const string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (op == "^") return 3; // 次方優先級最高
    return 0;
}

// ==========================================
// 3. Tokenizer (字串解析器)
// 處理多位數字、小數、負號與減號的區分
// ==========================================
vector<string> tokenize(const string& expr) {
    vector<string> tokens;
    int n = expr.length();
    int i = 0;
    
    while (i < n) {
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // 判斷是否為數字開頭 (包含小數點開頭)
        bool is_digit_start = isdigit(expr[i]) || (expr[i] == '.' && i + 1 < n && isdigit(expr[i + 1]));
        
        // 判斷是否為一元負號 (位於字串開頭，或前一個 token 是左括號或運算子)
        bool is_negative_sign = (expr[i] == '-') && (tokens.empty() || tokens.back() == "(" || isOperator(tokens.back()));

        if (is_digit_start || is_negative_sign) {
            string num_str = "";
            bool decimal_found = false;
            
            if (is_negative_sign) {
                num_str += "-";
                i++;
            }

            while (i < n && (isdigit(expr[i]) || (expr[i] == '.' && !decimal_found))) {
                if (expr[i] == '.') {
                    decimal_found = true;
                }
                num_str += expr[i];
                i++;
            }
            // 避免只擷取到一個單獨的負號 (例如錯誤的輸入)
            if (num_str == "-") {
                tokens.push_back("-");
            } else {
                tokens.push_back(num_str);
            }
        } 
        else {
            // 處理括號與其他運算子
            tokens.push_back(string(1, expr[i]));
            i++;
        }
    }
    return tokens;
}

// ==========================================
// 4. Infix 轉 Postfix (Shunting Yard 演算法)
// ==========================================
vector<string> infixToPostfix(const vector<string>& tokens) {
    vector<string> postfix;
    LinkedListStack<string> opStack;

    for (const string& token : tokens) {
        // 如果是數字 (包含負數)，直接輸出
        if (!isOperator(token) && token != "(" && token != ")") {
            postfix.push_back(token);
        } 
        else if (token == "(") {
            opStack.push("(");
        } 
        else if (token == ")") {
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                postfix.push_back(opStack.peek());
                opStack.pop();
            }
            if (!opStack.isEmpty() && opStack.peek() == "(") {
                opStack.pop(); // 彈出左括號
            } else {
                throw runtime_error("錯誤: 括號不匹配。");
            }
        } 
        else { // 遇到運算子
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                int precedence_stack = getPrecedence(opStack.peek());
                int precedence_token = getPrecedence(token);
                
                // 次方 (^) 為右結合 (Right Associative)
                if (token == "^") {
                    if (precedence_stack > precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break;
                } else {
                    if (precedence_stack >= precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break;
                }
            }
            opStack.push(token);
        }
    }

    while (!opStack.isEmpty()) {
        if (opStack.peek() == "(") throw runtime_error("錯誤: 括號不匹配。");
        postfix.push_back(opStack.peek());
        opStack.pop();
    }

    return postfix;
}

// ==========================================
// 5. Postfix 運算器 (包含顯示逐步計算過程)
// ==========================================
double evaluatePostfix(const vector<string>& postfix) {
    LinkedListStack<double> valStack;

    cout << "\n--- 逐步計算過程 ---" << endl;
    if (postfix.empty()) throw runtime_error("錯誤: 無效的表達式。");

    for (const string& token : postfix) {
        if (!isOperator(token)) {
            valStack.push(stod(token));
        } 
        else { 
            if (valStack.isEmpty()) throw runtime_error("錯誤: 表達式格式錯誤。");
            double val2 = valStack.peek(); valStack.pop();
            
            if (valStack.isEmpty()) throw runtime_error("錯誤: 表達式格式錯誤。");
            double val1 = valStack.peek(); valStack.pop();

            double current_result = 0.0;

            if (token == "+") current_result = val1 + val2;
            else if (token == "-") current_result = val1 - val2;
            else if (token == "*") current_result = val1 * val2;
            else if (token == "/") {
                if (val2 == 0) throw runtime_error("錯誤: 除數不可為零 (Division by zero)。");
                current_result = val1 / val2;
            }
            else if (token == "^") current_result = pow(val1, val2);
            else if (token == "%") current_result = fmod(val1, val2);

            // 印出當前步驟的計算
            cout << "[執行] " << val1 << " " << token << " " << val2 << " = " << current_result << endl;
            
            valStack.push(current_result);
        }
    }

    if (valStack.isEmpty()) throw runtime_error("錯誤: 計算失敗。");
    
    double final_result = valStack.peek();
    valStack.pop();
    
    // 如果計算結束後，堆疊內還有多餘的數字，代表輸入格式有誤
    if (!valStack.isEmpty()) throw runtime_error("錯誤: 多餘的操作數，表達式無效。");
    
    cout << "--------------------" << endl;
    return final_result;
}

// ==========================================
// 6. 主程式與 REPL 互動介面
// ==========================================
int main() {
    string input;
    
    cout << "==========================================" << endl;
    cout << "          C++ Linked List 計算機          " << endl;
    cout << "==========================================" << endl;
    cout << "支援的運算子: +, -, *, /, ^ (次方), % (取餘)" << endl;
    cout << "輸入您的數學公式，或輸入 'exit' 結束程式。" << endl;
    cout << "------------------------------------------" << endl;

    while (true) {
        cout << "\n>> 請輸入公式: ";
        
        if (!getline(cin, input)) break;
        if (input.empty()) continue;
        if (input == "exit" || input == "quit") {
            cout << "結束計算機程式。" << endl;
            break;
        }

        try {
            // 1. Tokenize
            vector<string> tokens = tokenize(input);
            
            // 2. Infix to Postfix
            vector<string> postfix = infixToPostfix(tokens);
            
            // 印出轉換後的 Postfix 陣列 (幫助除錯與確認解析正確)
            cout << "[解析] 後置表達式 (Postfix): ";
            for (const string& s : postfix) cout << s << " ";
            
            // 3. Evaluate Postfix (內部會印出逐步計算過程)
            double result = evaluatePostfix(postfix);
            
            // 4. 輸出最終結果：如果是整數，強制補上 .0
            if (result == floor(result)) {
                cout << "=> 最終結果: " << result << ".0" << endl;
            } else {
                cout << "=> 最終結果: " << result << endl;
            }
            
        } catch (const exception& e) {
            cout << "=> " << e.what() << endl;
        }
    }

    return 0;
}
