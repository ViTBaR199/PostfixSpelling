#include <iostream>
#include <fstream>
#include <string>

int getPriority(char ch) {
    switch (ch) {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
    case '%':
        return 2;
    case '^':
        return 3;
    default:
        return 0;
    }
}

struct Node {
    int data;
    Node* next;
};

class Stack {
private:
    Node* top;
    int size;

public:
    Stack() {
        top = nullptr;
        size = 0;
    }
    ~Stack()
    {
        while (top != nullptr) {
            Node* temp = top;
            top = top->next;
            delete temp;
        }
    }
    // Конструктор копирования
    Stack(const Stack& other) {
        top = nullptr;
        size = 0;
        Node* temp = other.top;
        while (temp != nullptr) {
            push(temp->data);
            temp = temp->next;
        }
    }

    // Оператор присваивания
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            while (top != nullptr) {
                pop();
            }
            Node* temp = other.top;
            while (temp != nullptr) {
                push(temp->data);
                temp = temp->next;
            }
        }
        return *this;
    }

    // Конструктор перемещения
    Stack(Stack&& other) noexcept : top(other.top), size(other.size) {
        other.top = nullptr;
        other.size = 0;
    }

    // Оператор присваивания перемещением
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            delete top;

            top = other.top;
            size = other.size;

            other.top = nullptr;
            other.size = 0;
        }
        return *this;
    }

    void push(int value) {
        Node* newNode = new Node();

        if (!newNode) {
            //если мы не можем выделить память из кучи
            //для нового узла
            std::cout << "Heap overflow" << std::endl;
            return;
        }

        newNode->data = value;
        newNode->next = top;
        top = newNode;
        size++;
    }

    int pop() {
        if (top == nullptr) {
            std::cout << "Stack underflow" << std::endl;
            return 0;
        }

        Node* temp = top;

        top = top->next;
        int popped_value = temp->data;
        delete temp;
        size--;

        return popped_value;
    }

    void show() {
        if (top == nullptr) {
            std::cout << "Stack underflow" << std::endl;
            return;
        }
        Node* temp = top;
        while (temp != nullptr) {
            std::cout << temp->data << "-> ";
            temp = temp->next;
        }
    }

    int etop() {
        if (top == nullptr) {
            std::cout << "Стек пуст" << std::endl;
            return 0;
        }
        else {
            return top->data;
        }
    }

    bool empty() {
        return top == nullptr;
    }

    int esize() {
        return size;
    }
};

int expression(Stack& postfixStack, int count_line, std::ofstream& outfile) {
    Stack reverse, op, num;

    while (!postfixStack.empty()) {
        reverse.push(postfixStack.pop());
    }

    while (!reverse.empty()) {
        int value = reverse.pop();

        if (value >= 0 && value != 37 && value != 42 && value != 43 && value != 45 && value != 47) {
            num.push(value);
        }
        else if (value < 0) {
            outfile << "Передано отрицательное значение\tв строке: "<< count_line << std::endl;
            return 0;
        }
        else {
            op.push(value);
        }

        if (op.esize() == 1) {
            op.pop();
            int operand2 = num.pop();
            int operand1 = num.pop();
            int result;

            switch (value) {
            case 43:
                result = operand1 + operand2;
                break;
            case 45:
                result = operand1 - operand2;
                break;
            case 42:
                result = operand1 * operand2;
                break;
            case 47:
                if (operand1 == 0 || operand2 == 0) {
                    outfile << "Ошибка: оператор/операнд = 0\tв строке: "<< count_line << std::endl;
                    return 0;
                }
                else {
                    result = operand1 / operand2;
                    break;
                }
            case 37:
                if (operand1 == 0 || operand2 == 0) {
                    outfile << "Ошибка: оператор/операнд = 0\tв строке: " << count_line << std::endl;
                    return 0;
                }
                else {
                    result = operand1 % operand2;
                    break;
                }
            default:
                outfile << "Неизвестный оператор\tв строке: "<< count_line << std::endl;
                return 0;
            }
            num.push(result);
        }
    }
    return num.etop();
}

Stack filling_the_stack(std::string& fileName) {
    std::ifstream file(fileName);
    std::ofstream outfile("errors");

    Stack numberStack;
    Stack operatorStack;
    Stack postfixStack;

    Stack outStack;

    std::string number;
    char ch;
    int count_bracket = 0, count_line = 0;
    while (file.get(ch)) {

        if (ch == '\n' || file.eof()) {
            
            if (!number.empty()) {
                numberStack.push(std::stoi(number));
                postfixStack.push(std::stoi(number));
                number.clear();
            }

            while (!operatorStack.empty()) {
                postfixStack.push(operatorStack.pop());
            }

            int result = expression(postfixStack, count_line, outfile);
            outStack.push(result);

            // Обнуляем стеки для следующей строки
            numberStack = Stack();
            operatorStack = Stack();
            postfixStack = Stack();
            count_line++;
            continue;
        }

        if (std::isdigit(ch)) { //когда встречаем цифру
            number += ch;       //дописываем её к numper
        }
        else if (!number.empty()) {
            numberStack.push(std::stoi(number)); //преобразование number в число и занесение его в Стек
            postfixStack.push(std::stoi(number));
            number.clear();
        }

        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^') {
            while (!operatorStack.empty() && getPriority(operatorStack.etop()) >= getPriority(ch)) {
                postfixStack.push(operatorStack.pop());
            }
            operatorStack.push(ch);
        }
        else if (ch == '(') {
            operatorStack.push(ch);
            count_bracket++;
        }
        else if (ch == ')') {
            count_bracket--;

            if (count_bracket != 0) {
                outfile << "Нарушен баланс скобок!\tв строке: " << count_line << std::endl; 
                numberStack = Stack();
                operatorStack = Stack();
                postfixStack = Stack();

                continue;
            }
            while (!operatorStack.empty()) {
                if (operatorStack.etop() == '(') {
                    break;
                }
                postfixStack.push(operatorStack.pop());
            }
            if (!operatorStack.empty()) {
                operatorStack.pop();
            }
        }
    }

    if (!number.empty()) {
        numberStack.push(std::stoi(number));
        postfixStack.push(std::stoi(number));
    }

    while (!operatorStack.empty()) {
        postfixStack.push(operatorStack.pop());
    }

    return outStack;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    std::string filename;
    std::cout << "Введите путь к файлу: ";
    std::cin >> filename;
    filling_the_stack(filename).show();
    return 0;
}