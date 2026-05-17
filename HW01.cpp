#include <iostream>   
#include <string>      // 引入字串庫，用於處理數學表達式字串
#include <vector>      // 引入動態陣列庫，用於儲存解析後的 Token 陣列
#include <cmath>       // 引入數學庫，用於計算次方 (pow)、浮點數取餘 (fmod) 與無條件捨去 (floor)
#include <stdexcept>   // 引入標準例外庫，用於拋出錯誤訊息 (如 runtime_error)
#include <cctype>      // 引入字元處理庫，用於判斷是否為數字 (isdigit) 或空白 (isspace)

using namespace std;  

// ==========================================
// 1. 實作 Linked List 節點與 Stack (Template)
// ==========================================

// 定義 Linked List 的節點結構，使用 Template 支援各種類型
template <typename T>
struct Node {
    T data;             // 節點儲存的資料
    Node* next;         // 指向底下一格節點的指標
    Node(T val) : data(val), next(nullptr) {} // 建構子：初始化資料並將指標設為空
};

// 使用 Linked List 實作的堆疊類別
template <typename T>
class LinkedListStack {
private:
    Node<T>* topNode;   // 指向堆疊最頂端節點的指標

public:
    // 建構子：初始化時堆疊為空，頂端指標指向 nullptr
    LinkedListStack() : topNode(nullptr) {}

    // 解構子：在物件生命週期結束時，釋放所有動態配置的記憶體
    ~LinkedListStack() {
        while (!isEmpty()) { // 只要堆疊還沒空
            pop();           // 就持續彈出節點以釋放記憶體
        }
    }

    // 將資料推入堆疊頂端 
    void push(T val) {
        Node<T>* newNode = new Node<T>(val); // 動態配置一個新節點
        newNode->next = topNode;             // 新節點的下一個指向原本的頂端
        topNode = newNode;                   // 將頂端指標更新為新節點
    }

    // 將頂端的資料彈出堆疊 
    void pop() {
        if (isEmpty()) {                     // 若堆疊是空的無法再彈出
            throw runtime_error("錯誤: 堆疊下溢 (Stack underflow)。");
        }
        Node<T>* temp = topNode;             // 暫存目前的頂端節點
        topNode = topNode->next;             // 將頂端指標移至下一個節點
        delete temp;                         // 釋放舊頂端節點的記憶體
    }

    // 查看堆疊頂端的資料，但不移除它
    T peek() const {
        if (isEmpty()) {                     // 若堆疊是空的無法查看
            throw runtime_error("錯誤: 堆疊為空 (Stack is empty)。");
        }
        return topNode->data;                // 回傳頂端節點的資料
    }

    // 檢查堆疊是否為空
    bool isEmpty() const {
        return topNode == nullptr;           // 若頂端指標為空，代表堆疊無資料
    }
};

// ==========================================
// 2. 輔助函式：判斷運算子與優先級
// ==========================================

// 判斷該字串是否為支援的數學運算子
bool isOperator(const string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || token == "%";
}

// 取得四則運算子的權重優先級 (數字越大優先權越高，參數改為接收 string 提高相容性)
int getPrecedence(const string& op) {
    if (op == "+" || op == "-") return 1;       // 加減法優先級最低 (1)
    if (op == "*" || op == "/" || op == "%") return 2; // 乘除、取餘優先級中等 (2)
    if (op == "^") return 3;                     // 次方優先級最高 (3)
    return 0;                                    // 括號或其他字元回傳 0
}

// ==========================================
// 3. Tokenizer (字串解析器)
// 處理多位數字、小數、負號與減號的區分
// ==========================================
vector<string> tokenize(const string& expr) {
    vector<string> tokens; // 儲存分割後的各個 Token (數字、運算子、括號)
    int n = expr.length(); // 取得運算式字串的總長度
    int i = 0;             // 初始化索引指標
    
    while (i < n) {
        if (isspace(expr[i])) { // 如果是空格
            i++;                // 指標直接往後走
            continue;           // 略過不處理
        }

        // 判斷是否為數字開頭
        bool is_digit_start = isdigit(expr[i]) || (expr[i] == '.' && i + 1 < n && isdigit(expr[i + 1]));
        
        // 判斷是否為一元負號
        bool is_negative_sign = (expr[i] == '-') && (tokens.empty() || tokens.back() == "(" || isOperator(tokens.back()));

        if (is_digit_start || is_negative_sign) {
            string num_str = "";       // 用來拼湊完整數字的字串
            bool decimal_found = false; // 旗標：用來記錄該數字是否已經包含小數點，防止出現 5.1.2 這種錯誤
            
            if (is_negative_sign) {
                num_str += "-";        // 如果是負號，先將 '-' 拼接到字串開頭
                i++;                   // 指標往後走一格，準備讀取後續數字
            }

            // 連續讀取接下來的字元，只要是數字、或者尚未出現過的小數點就繼續拼接
            while (i < n && (isdigit(expr[i]) || (expr[i] == '.' && !decimal_found))) {
                if (expr[i] == '.') {
                    decimal_found = true; // 標記已找到小數點，後續此數內不能再有小數點
                }
                num_str += expr[i]; // 拼接到數字字串中
                i++;                // 指標往後移
            }
            // 防呆：避免特殊格式下只擷取到負號
            if (num_str == "-") {
                tokens.push_back("-"); // 當作減號處理
            } else {
                tokens.push_back(num_str); // 將拼湊完成的數字（如 "-3.14" 或 "125"）放入陣列
            }
        } 
        else {
            // 處理非數字的其他字元 (如 +, *, /, %, ^, (, ) )
            tokens.push_back(string(1, expr[i])); // 將該字元轉成字串並放入陣列
            i++;                                 // 指標往後移一格
        }
    }
    return tokens; // 回傳解析完畢的 Token 向量陣列
}

// ==========================================
// 4. Infix 轉 Postfix (Shunting Yard 演算法)
// ==========================================
vector<string> infixToPostfix(const vector<string>& tokens) {
    vector<string> postfix;          // 儲存轉換後的後序運算式結果
    LinkedListStack<string> opStack; // 暫存運算子的堆疊 (型態為 string)

    for (const string& token : tokens) {
        // 如果不是運算子也不是左右括號，那它一定是數字 (包含負數、浮點數)
        if (!isOperator(token) && token != "(" && token != ")") {
            postfix.push_back(token); // 數字在後序表示法中，直接輸出
        } 
        else if (token == "(") {
            opStack.push("("); // 遇到左括號，直接推入運算子堆疊
        } 
        else if (token == ")") {
            // 遇到右括號，開始彈出堆疊內的運算子並輸出，直到遇見左括號為止
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                postfix.push_back(opStack.peek());
                opStack.pop();
            }
            if (!opStack.isEmpty() && opStack.peek() == "(") {
                opStack.pop(); // 找到對應的左括號，將其彈出並捨棄 (括號本身不輸出)
            } else {
                // 如果堆疊清空了都沒找到左括號，代表右括號落單，拋出錯誤
                throw runtime_error("錯誤: 括號不匹配。");
            }
        } 
        else { // 遇到一般的運算子 (+, -, *, /, %, ^)
            // 當堆疊不為空且最頂端不是左括號時，進行優先級比較
            while (!opStack.isEmpty() && opStack.peek() != "(") {
                int precedence_stack = getPrecedence(opStack.peek()); // 取得堆疊頂端運算子的優先級
                int precedence_token = getPrecedence(token);          // 取得當前 Token 運算子的優先級
                
                // 次方 (^) 是「右結合性」(Right Associative)
                if (token == "^") {
                    // 右結合性：只有當堆疊頂端的優先級「嚴格大於」當前運算子時才彈出
                    if (precedence_stack > precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break; // 如果相等或較小，則不彈出，直接跳出
                } else {
                    // 其他運算子（左結合性）：只要堆疊頂端優先級「大於或等於」當前運算子就彈出
                    if (precedence_stack >= precedence_token) {
                        postfix.push_back(opStack.peek());
                        opStack.pop();
                    } else break; // 如果較小，則不彈出，直接跳出
                }
            }
            opStack.push(token); // 將當前的運算子推入堆疊中
        }
    }

    // 當所有 Token 掃描完畢，將堆疊內剩餘的所有運算子依序彈出並輸出
    while (!opStack.isEmpty()) {
        if (opStack.peek() == "(") throw runtime_error("錯誤: 括號不匹配。"); // 如果還有剩餘左括號，代表格式錯誤
        postfix.push_back(opStack.peek());
        opStack.pop();
    }

    return postfix; // 回傳最終轉換好的後序表示法陣列
}

// ==========================================
// 5. Postfix 運算器 (包含顯示逐步計算過程)
// ==========================================
double evaluatePostfix(const vector<string>& postfix) {
    LinkedListStack<double> valStack; // 儲存操作數(數值)的堆疊

    cout << "\n--- 逐步計算過程 ---" << endl;
    if (postfix.empty()) throw runtime_error("錯誤: 無效的表達式。");

    for (const string& token : postfix) {
        if (!isOperator(token)) {
            valStack.push(stod(token)); // 如果是數字，轉成 double 後推入堆疊 (stod: string to double)
        } 
        else { // 如果是運算子，則從堆疊中取出兩個數字進行運算
            if (valStack.isEmpty()) throw runtime_error("錯誤: 表達式格式錯誤。");
            double val2 = valStack.peek(); valStack.pop(); // 先彈出的是右操作數 (後面的數)
            
            if (valStack.isEmpty()) throw runtime_error("錯誤: 表達式格式錯誤。");
            double val1 = valStack.peek(); valStack.pop(); // 後彈出的是左操作數 (前面的數)

            double current_result = 0.0; // 宣告變數儲存當前步驟的計算結果

            // 依據不同的運算子進行對應的數學計算
            if (token == "+") current_result = val1 + val2;
            else if (token == "-") current_result = val1 - val2;
            else if (token == "*") current_result = val1 * val2;
            else if (token == "/") {
                if (val2 == 0) throw runtime_error("錯誤: 除數不可為零 (Division by zero)。"); // 檢查除以零
                current_result = val1 / val2;
            }
            else if (token == "^") current_result = pow(val1, val2);  // 計算 val1 的 val2 次方
            else if (token == "%") current_result = fmod(val1, val2); // 浮點數取餘數

            // 印出目前步驟的計算軌跡 (例如: [執行] 5 * 3 = 15)
            cout << "[執行] " << val1 << " " << token << " " << val2 << " = " << current_result << endl;
            
            valStack.push(current_result); // 將當前計算結果重新推回堆疊，供後續計算使用
        }
    }

    if (valStack.isEmpty()) throw runtime_error("錯誤: 計算失敗。");
    
    double final_result = valStack.peek(); // 取得堆疊頂端的最終結果
    valStack.pop();                        // 彈出頂端
    
    // 嚴密檢查：如果計算結束後，堆疊內還有殘留數字，代表使用者輸入了過多數字（例如 "5 5 + 3" 這種無效格式）
    if (!valStack.isEmpty()) throw runtime_error("錯誤: 多餘的操作數，表達式無效。");
    
    cout << "--------------------" << endl;
    return final_result; // 回傳最終答案
}

// ==========================================
// 6. 主程式與 REPL 互動介面
// ==========================================
int main() {
    string input; // 宣告用來接收使用者輸入的字串變數
    
    // 列印歡迎介面與操作說明
    cout << "==========================================" << endl;
    cout << "          C++ Linked List 計算機          " << endl;
    cout << "==========================================" << endl;
    cout << "支援的運算子: +, -, *, /, ^ (次方), % (取餘)" << endl;
    cout << "輸入您的數學公式，或輸入 'exit' 結束程式。" << endl;
    cout << "------------------------------------------" << endl;

    // 啟動 REPL (Read-Eval-Print Loop) 無限迴圈
    while (true) {
        cout << "\n>> 請輸入公式: ";
        
        // 讀取整行輸入（包含空白），避免 cin >> input 遇到空格就斷掉
        if (!getline(cin, input)) break; // 如果遇到串流結束 EOF (如 Ctrl+D)，跳出迴圈
        if (input.empty()) continue;     // 如果使用者直接按 Enter，跳過本次迴圈重新詢問
        
        // 檢查是否輸入退出指令
        if (input == "exit" || input == "quit") {
            cout << "結束計算機程式。" << endl;
            break; // 終止迴圈，結束程式
        }

        try {
            // 1. Tokenize：將輸入字串解析為多個 Token 元素
            vector<string> tokens = tokenize(input);
            
            // 2. Infix to Postfix：將 中序 轉換為 後序 表示法
            vector<string> postfix = infixToPostfix(tokens);
            
            // 印出轉換後的 Postfix 陣列，讓使用者一目了然轉換結果
            cout << "[解析] 後置表達式 (Postfix): ";
            for (const string& s : postfix) cout << s << " ";
            cout << endl;
            
            // 3. Evaluate Postfix：計算後序表達式 (內部會自動印出逐步計算過程)
            double result = evaluatePostfix(postfix);
            
            // 4. 輸出最終結果：如果是整數值，強制補上 .0 (例如 5 變成 5.0)，以維持浮點數風格的顯示
            if (result == floor(result)) {
                cout << "=> 最終結果: " << result << ".0" << endl;
            } else {
                cout << "=> 最終結果: " << result << endl;
            }
            
        } catch (const exception& e) {
            // 捕捉執行過程中所有的錯誤（如：括號不匹配、除以零、公式不完整等）
            // 並列印出具體的錯誤原因，保證程式不會崩潰直接關閉
            cout << "=> " << e.what() << endl;
        }
    }

    return 0; 
}
