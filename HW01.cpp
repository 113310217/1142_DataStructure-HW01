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

// 定義鏈結串列的節點結構，使用 Template 以支援不同資料型態 (如 string, double)
template <typename T>
struct Node {
    T data;         // 節點儲存的資料
    Node* next;     // 指向下一個節點的指標
    // 建構子：初始化資料並將 next 設為 nullptr
    Node(T val) : data(val), next(nullptr) {} 
};

// 使用鏈結串列實作的堆疊 (Stack) 類別
template <typename T>
class LinkedListStack {
private:
    Node<T>* topNode; // 指向堆疊頂端的節點

public:
    // 初始化時，堆疊為空
    LinkedListStack() : topNode(nullptr) {}

    // 解構子：當堆疊被銷毀時，釋放所有節點的記憶體以避免 Memory Leak
    ~LinkedListStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    // 將資料推入堆疊 (新增節點至鏈結串列頭部)
    void push(T val) {
        Node<T>* newNode = new Node<T>(val);
        newNode->next = topNode; 
        topNode = newNode;       
    }

    // 將資料從堆疊彈出 (移除鏈結串列頭部的節點)
    void pop() {
        if (isEmpty()) {
            throw runtime_error("Stack underflow");
        }
        Node<T>* temp = topNode;
        topNode = topNode->next; 
        delete temp;             
    }

    // 查看堆疊頂端的資料 (不移除)
    T peek() const {
        if (isEmpty()) {
            throw runtime_error("Stack is empty");
        }
        return topNode->data;
    }

    // 檢查堆疊是否為空
    bool isEmpty() const {
        return topNode == nullptr;
    }
};

// ==========================================
// 2. 輔助函式：判斷運算子與優先級
// ==========================================

// 判斷傳入的字串是否為我們支援的運算子
bool isOperator(const string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || token == "%";
}

// 取得運算子的優先權 (數字越大優先權越高)
int getPrecedence(const string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (op == "^") return 3; // 次方優先級最高
    return 0;
}

// ==========================================
// 3. Tokenizer (字串解析器)
// 負責處理多位數字、小數、負號與減號的區分
// ==========================================
vector<string> tokenize(const string& expr) {
    vector<string> tokens; // 用來存放切割好的 token
    int n = expr.length();
    int i = 0;
    
    while (i < n) {
        // 遇到空白字元直接跳過
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // 判斷當前字元是否為數字的開頭 (包含小數點開頭)
        bool is_digit_start = isdigit(expr[i]) || (expr[i] == '.' && i + 1 < n && isdigit(expr[i + 1]));
        
        // 判斷當前字元是否為「負號」(一元運算子)
        // 條件：當前是 '-'，且 (位於字串開頭，或前一個 token 是左括號，或前一個是運算子)
        bool is_negative_sign = (expr[i] == '-') && (tokens.empty() || tokens.back() == "(" || isOperator(tokens.back()));

        // 如果是數字開頭或是負號，擷取完整數字
        if (is_digit_start || is_negative_sign) {
            string num_str = "";
            bool decimal_found = false; 
            
            // 如果判定為負號，先加入負號並往後移
            if (is_negative_sign) {
                num_str += "-";
                i++;
            }

            // 連續讀取數字或小數點
            while (i < n && (isdigit(expr[i]) || (expr[i] == '.' && !decimal_found))) {
                if (expr[i] == '.') {
                    decimal_found = true;
                }
                num_str += expr[i];
                i++;
            }
            
            // 避免只擷取到單獨的 "-"
            if (num_str == "-") {
                tokens.push_back("-");
            } else {
                tokens.push_back(num_str); 
            }
        } 
        else {
            // 括號或一般運算子，直接存入
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
        // 規則 1：如果是數字，直接輸出到 postfix 陣列
        if (!isOperator(token) && token != "(" && token != ")") {
            postfix.push_back(token);
        } 
        // 規則 2：左括號推入堆疊
        else if (token == "(") {
            opStack.push("(");
        } 
        // 規則 3：右括號彈出運算子直到遇到左括號
        else if (token == ")") {
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                postfix.push_back(opStack.peek());
                opStack.pop();
            }
            if (!opStack.isEmpty() && opStack.peek() == "(") {
                opStack.pop(); 
            } else {
                throw runtime_error("Mismatched parentheses");
            }
        } 
        // 規則 4：遇到運算子，比較優先權
        else { 
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                int precedence_stack = getPrecedence(opStack.peek()); 
                int precedence_token = getPrecedence(token);          
                
                // 次方 (^) 是右結合，優先權相等時不彈出
                if (token == "^") {
                    if (precedence_stack > precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break;
                } else {
                    // 一般運算子，堆疊頂端優先權 >= 當前優先權，則彈出
                    if (precedence_stack >= precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break;
                }
            }
            opStack.push(token);
        }
    }

    // 將堆疊剩下的運算子清空
    while (!opStack.isEmpty()) {
        if (opStack.peek() == "(") throw runtime_error("Mismatched parentheses");
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
    if (postfix.empty()) throw runtime_error("Empty expression");

    for (const string& token : postfix) {
        // 如果是數字，轉換成 double 後推入堆疊
        if (!isOperator(token)) {
            valStack.push(stod(token)); 
        } 
        // 如果是運算子，從堆疊彈出兩個數值進行運算
        else { 
            if (valStack.isEmpty()) throw runtime_error("Format error");
            double val2 = valStack.peek(); valStack.pop(); // 先彈出的是右運算元
            
            if (valStack.isEmpty()) throw runtime_error("Format error");
            double val1 = valStack.peek(); valStack.pop(); // 後彈出的是左運算元

            double current_result = 0.0;

            if (token == "+") current_result = val1 + val2;
            else if (token == "-") current_result = val1 - val2;
            else if (token == "*") current_result = val1 * val2;
            else if (token == "/") {
                if (val2 == 0) throw runtime_error("Division by zero");
                current_result = val1 / val2;
            }
            else if (token == "^") current_result = pow(val1, val2);
            else if (token == "%") current_result = fmod(val1, val2);

            // 印出當前步驟
            cout << "[執行] " << val1 << " " << token << " " << val2 << " = " << current_result << endl;
            
            // 將結果推回堆疊
            valStack.push(current_result);
        }
    }

    if (valStack.isEmpty()) throw runtime_error("Evaluation failed");
    
    // 最後剩下的一個數值就是最終結果
    double final_result = valStack.peek();
    valStack.pop();
    
    if (!valStack.isEmpty()) throw runtime_error("Invalid expression format");
    
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

    // 無窮迴圈接收使用者輸入
    while (true) {
        cout << "\n>> 請輸入公式: ";
        
        if (!getline(cin, input)) break;
        if (input.empty()) continue;
        if (input == "exit" || input == "quit") {
            cout << "結束計算機程式。" << endl;
            break;
        }

        try {
            vector<string> tokens = tokenize(input);
            vector<string> postfix = infixToPostfix(tokens);
            
            // 檢查轉換後的 postfix，幫助確認解析無誤
            cout << "[解析] 後置表達式 (Postfix): ";
            for (const string& s : postfix) cout << s << " ";
            
            double result = evaluatePostfix(postfix);
            
            // 輸出結果：整數強制補上 .0
            if (result == floor(result)) {
                cout << "=> 最終結果: " << result << ".0" << endl;
            } else {
                cout << "=> 最終結果: " << result << endl;
            }
            
        } 
        // 捕捉所有預期或未預期的錯誤，統一輸出 "Error"
        catch (...) {
            cout << "=> Error" << endl;
        }
    }

    return 0;
}
