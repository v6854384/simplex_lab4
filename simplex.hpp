#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
#include "Fraqtion.hpp"

using namespace std;

const Fraqtion INF(1, 0); // бесконечность
const Fraqtion EPS(0, 1); // точность

// тип симплекс решения
enum class SimplexMode {
    Max,
    Min
};

// структура для решения
struct SimplexSolve {
    vector<Fraqtion> x;
    Fraqtion f;
};

class Simplex {
    vector<Fraqtion> c; // значения целевой функции
    SimplexMode mode; // режим решения
    int n; // количество переменных
    int m; // количество ограничений

    vector<int> basis; // базис
    vector<Fraqtion> deltas; // дельты
    vector<vector<Fraqtion>> table; // таблица

    // начальные условия
    vector<vector<Fraqtion>> initialA;
    vector<Fraqtion> initialC;
    vector<Fraqtion> initialB;

    int padding; // отступ

    void PrintVector(const vector<Fraqtion> &v) const;
    void PrintHeader() const;
    void PrintLine() const;

    int GetNegativeRowB(); // получение строки с максимальным по модулю отрицательным значением b
    int GetNegativeColumnB(int row); // получение столбца с максимальным по модулю элементом в строке
    bool RemoveNegativeB(); // удаление отрицательных элементов в b

    void DivideRow(int row, Fraqtion value); // деление строки на число
    void SubstractRow(int row1, int row2, Fraqtion value); // вычитание строки row2 * value из row1
    void Gauss(int row, int column); // исключение гаусса
    Fraqtion Part(Fraqtion x) const; // получение дробной части

    bool IsBasis(int index) const; // базисная ли переменная
    bool IsOptimal(); // проверка плана на оптимальность
    bool CheckSolve(const vector<Fraqtion> x) const; // подходит ли решение по условию
    void CalculateDeltas(); // расчёт дельт
    vector<Fraqtion> CalculateSimplexRelations(int columnIindex); // расчёт симплекс отношений

    int GetSolveColumn(); // получение разрешающего столбца
    int GetSolveRow(const vector<Fraqtion> &q); // получение разрешающей строки
    SimplexSolve GetSolve(); // получение решения

    int GetRealIndex(const vector<Fraqtion> &x); // получение индекса вещественного решения
public:
    Simplex(const vector<vector<Fraqtion>> &a, const vector<Fraqtion> &b, const vector<Fraqtion> &c, SimplexMode mode, int padding = 0);

    void ConvertToDual(); // перевод в двойственную
    void PrintTable() const; // вывод таблицы
    void PrintTask() const; // вывод задачи
    void PrintSolve(SimplexSolve solve) const; // вывод решения
    bool Solve(bool debug = true); // решение задачи

    vector<SimplexSolve> SolveIntegerBranchesAndBorders(bool debug = false, int depth = 0); // получение целочисленных решений
    vector<SimplexSolve> SolveIntegerBruteforce(int nmax); // поиск решений методом грубой силы
    vector<SimplexSolve> SolveGomory(bool debug = false); // поиск решения методом Гомори

    void FindBestSolve(const vector<SimplexSolve> &solves) const; // поиск лучшего из решений
};

Simplex::Simplex(const vector<vector<Fraqtion>> &a, const vector<Fraqtion> &b, const vector<Fraqtion> &c, SimplexMode mode, int padding) {
    this->mode = mode;

    this->n = a[0].size(); // считаем количество основных переменных
    this->m = a.size(); // считаем количество ограничений
    this->padding = padding; // запоминаем значение отступа

    // добавляем базисные переменные
    for (int i = 0; i < this->m; i++)
        this->basis.push_back(i + this->n);

    // заполняем дельты и вектор функции
    for (int i = 0; i < n + m + 1; i++) {
        this->deltas.push_back(0);
        this->c.push_back(i < n ? c[i] : 0);
    }

    for (int i = 0; i < m; i++) {
        this->table.push_back(vector<Fraqtion>(n + m + 1, 0)); // добавляем строку в таблицу

        for (int j = 0; j < n; j++)
            this->table[i][j] = a[i][j]; // копируем основные ограничения

        for (int j = 0; j < m; j++)
            this->table[i][j + n] = i == j; // добавляем единичную матрицу

        this->table[i][n + m] = b[i]; // копируем свободный член
    }

    initialA = vector<vector<Fraqtion>>(a);
    initialB = vector<Fraqtion>(b);
    initialC = vector<Fraqtion>(c);
}

void Simplex::PrintVector(const vector<Fraqtion> &v) const {
    for (int i = 0; i < v.size(); i++)
        cout << " " << setw(9) << v[i] << " |";

    cout << endl;
}

void Simplex::PrintHeader() const {
    cout << string(padding, ' ');
    cout << "|  basis  |";

    for (int i = 0; i < n + m; i++) {
        string s = "x";
        s += to_string(i + 1);

        cout << " " << setw(9) << s << " |";
    }

    cout << "         b |" << endl;
}

void Simplex::PrintLine() const {
    cout << string(padding, ' ');
    cout << "+---------+";
    for (int i = 0; i < n + m + 1; i++)
        cout << "-----------+";
    cout << endl;
}

void Simplex::PrintSolve(SimplexSolve solve) const {
    cout << string(padding, ' ');
    cout << "x: [ ";
    for (int i = 0; i < n; i++)
        cout << solve.x[i] << " ";

    cout << "], F: " << solve.f << endl;
}

// вывод задачи
void Simplex::PrintTask() const {
    cout << string(padding, ' ');
    for (int i = 0; i < n + m; i++) {
        if (c[i] == 0)
            continue;

        if (i > 0)
            cout << (c[i] > 0 ? " + " : " - ");

        if (fabs(c[i]) != 1)
            cout << fabs(c[i]);

        cout << "x" << (i + 1);
    }

    cout << " -> " << (mode == SimplexMode::Max ? "max" : "min") << endl;

    for (int i = 0; i < m; i++) {
        cout << string(padding, ' ');
        bool wasPrinted = false;

        for (int j = 0; j < n + m; j++) {
            if (table[i][j] == 0)
                continue;

            if (j > 0 && wasPrinted)
                cout << (table[i][j] > 0 ? " + " : " - ");

            if (!wasPrinted && table[i][j] < 0)
                cout << "-";

            if (fabs(table[i][j]) != 1)
                cout << fabs(table[i][j]);

            cout << "x" << (j + 1);
            wasPrinted = true;
        }

        cout << " <= " << table[i][n + m] << endl;
    }
}

// вывод таблицы
void Simplex::PrintTable() const {
    PrintLine();
    cout << string(padding, ' ') << "|    C    |";
    PrintVector(c);
    PrintLine();
    PrintHeader();

    for (int i = 0; i < m; i++) {
        cout << string(padding, ' ') << "|   x" << left << setw(4) << (basis[i] + 1) << " |" << right;
        PrintVector(table[i]);
    }

    PrintLine();
    cout << string(padding, ' ') << "|    F    |";
    PrintVector(deltas);
    PrintLine();
}

// получение строки с максимальным по модулю отрицательным значением b
int Simplex::GetNegativeRowB() {
    int index = -1;

    for (int i = 0; i < m; i++) {
        Fraqtion bi = table[i][n + m];

        if ((bi < 0) && (index == -1 || bi < table[index][n + m]))
            index = i;
    }

    return index;
}

// получение столбца с максимальным по модулю элементом в строке
int Simplex::GetNegativeColumnB(int row) {
    int index = -1;

    for (int i = 0; i < n + m; i++) {
        if (table[row][i] >= 0)
            continue;

        if (index == -1 || fabs(table[row][i]) > fabs(table[row][index]))
            index = i;
    }

    return index;
}

// деление строки на число
void Simplex::DivideRow(int row, Fraqtion value) {
    for (int i = 0; i < n + m + 1; i++)
        table[row][i] /= value;
}

// вычитание строки row2 * value из row1
void Simplex::SubstractRow(int row1, int row2, Fraqtion value) {
    for (int i = 0; i < n + m + 1; i++)
        table[row1][i] -= table[row2][i] * value;
}

// исключение гаусса
void Simplex::Gauss(int row, int column) {
    DivideRow(row, table[row][column]);

    for (int i = 0; i < m; i++)
        if (i != row)
            SubstractRow(i, row, table[i][column]);

    basis[row] = column; // меняем базисный элемент
}

// получение дробной части
Fraqtion Simplex::Part(Fraqtion x) const {
    return x.GetRealPart();
}

// удаление отрицательных элементов в b
bool Simplex::RemoveNegativeB() {
    int row = GetNegativeRowB();

    // пока есть отрицательные элементы
    while (row != -1) {
        int column = GetNegativeColumnB(row); // получение столбца

        if (column == -1) // если не нашли
            return false; // значит нельзя избавиться

        Gauss(row, column); // выполняем исключение Гауса
        row = GetNegativeRowB(); // получаем следующую строку с отрицательным b
    }

    return true; // всё удалили
}

// базисная ли переменная
bool Simplex::IsBasis(int index) const {
    for (int i = 0; i < m; i++)
        if (basis[i] == index)
            return true;

    return false;
}

// проверка плана на оптимальность
bool Simplex::IsOptimal() {
    for (int i = 0; i < n + m; i++) {
        if (mode == SimplexMode::Max && deltas[i] < 0)
            return false;

        if (mode == SimplexMode::Min && deltas[i] > 0)
            return false;
    }
    
    return true; // план оптимален
}

// подходит ли решение по условию
bool Simplex::CheckSolve(const vector<Fraqtion> x) const {
    for (int i = 0; i < m; i++) {
        Fraqtion sum = 0;

        for (int j = 0; j < n; j++)
            sum += table[i][j] * x[j];

        if (sum > table[i][n + m]) // если не выполнено ограничение
            return false; // то решение не подходит
    }

    return true; // решение подходит
}

// расчёт дельт
void Simplex::CalculateDeltas() {
    for (int i = 0; i < n + m + 1; i++) {
        deltas[i] = -c[i];

        for (int j = 0; j < m; j++)
            deltas[i] += c[basis[j]] * table[j][i];
    }
}

// расчёт симплекс отношений
vector<Fraqtion> Simplex::CalculateSimplexRelations(int columnIndex) {
    vector<Fraqtion> q;

    for (int i = 0; i < m; i++) {
        if (table[i][columnIndex] == 0) {
            q.push_back(INF);
        }
        else {
            if (table[i][n + m] >= 0 && table[i][columnIndex] < 0) {
                q.push_back(INF);
            }
            else {
                q.push_back(table[i][n + m] / table[i][columnIndex]);
            }
        }
    }

    return q;
}

// получение разрешающего столбца
int Simplex::GetSolveColumn() {
    int column = 0;

    for (int i = 0; i < n + m; i++) {
        if (mode == SimplexMode::Max && deltas[i] < deltas[column]) {
            column = i;
        }
        else if (mode == SimplexMode::Min && deltas[i] > deltas[column]) {
            column = i;
        }
    }

    return column;
}

// получение разрешающей строки
int Simplex::GetSolveRow(const vector<Fraqtion> &q) {
    int row = -1;

    for (int i = 0; i < m; i++)
        if (q[i] != INF && (row == -1 || q[i] < q[row]))
            row = i;

    return row;
}

// получение решения
SimplexSolve Simplex::GetSolve() {
    SimplexSolve solve;
    solve.x = vector<Fraqtion>(n + m, 0);
    solve.f = deltas[n + m];

    // заполняем из базисных переменных
    for (int i = 0; i < m; i++)
        solve.x[basis[i]] = table[i][n + m];

    return solve; // возвращаем решение
}

// перевод в двойственную
void Simplex::ConvertToDual() {
    mode = mode == SimplexMode::Max ? SimplexMode::Min : SimplexMode::Max; // меняем режим

    for (int i = 0; i < n; i++) {
        Fraqtion ci = c[i];
        c[i] = table[i][n + m];
        table[i][n + m] = -ci;

        for (int j = i + 1; j < n; j++)
            swap(table[i][j], table[j][i]);

        for (int j = 0; j < n + m; j++)
            table[i][j] *= -1;
    }
}

// решение задачи
bool Simplex::Solve(bool debug) {
    if (!RemoveNegativeB()) {
        cout << string(padding, ' ') << "Solve does not exists" << endl << endl;
        return false;
    }

    if (debug) {
        cout << string(padding, ' ') << "Initial table:" << endl;
        CalculateDeltas(); // расчитываем дельты
        PrintTable();
    }

    for (int iteration = 1; true; iteration++) {
        CalculateDeltas(); // расчитываем дельты
        
        if (debug) {
            cout << endl << string(padding, ' ') << "Iteration " << iteration << endl;
            PrintTable();
        }

        // проверка плана на оптимальность, если оптимален, то решение найдено
        if (IsOptimal()) {
            if (debug)
                PrintSolve(GetSolve());

            return true; // решение есть
        }

        int column = GetSolveColumn(); // получаем разрешающий столбец
        vector<Fraqtion> q = CalculateSimplexRelations(column); // рассчитываем симлекс-отношения
        int row = GetSolveRow(q); // получаем разрешающую строку

        // если нет разрешающей строки, то решения нет
        if (row == -1) {
            cout << string(padding, ' ') << "Solve does not exist" << endl << endl;
            return false;
        }

        Gauss(row, column); // выполняем исключение Гауса
    }
}

// получение индекса вещественного решения
int Simplex::GetRealIndex(const vector<Fraqtion> &x) {
    int imax = -1;
    for (int i = 0; i < n; i++) {
        if (x[i].IsInteger()) // если не совпадает с целой частью
            continue;

        if (imax == -1 || x[imax].GetRealPart() < x[i].GetRealPart())
            imax = i;
    }

    return imax; // нет такого
}

// получение целочисленных решений
vector<SimplexSolve> Simplex::SolveIntegerBranchesAndBorders(bool debug, int depth) {
    cout << string(padding, ' ') << "Start solving task:" << endl;
    PrintTask();

    // если решение не было найдено, то добавляем пустое решение
    if (!Solve(debug))
        return {};

    SimplexSolve solve = GetSolve(); // получаем решение
    PrintSolve(solve);
    cout << endl;

    int realIndex = GetRealIndex(solve.x); // ищем вещественное решение

    // если решение содержало только целые числа, то возвращаем решение
    if (realIndex == -1) {
        return { solve };
    }

    Fraqtion b = solve.x[realIndex].GetIntPart(); // получаем новое условие
    
    // готовим новые условия
    vector<vector<Fraqtion>> a1 = initialA;
    vector<vector<Fraqtion>> a2 = initialA;

    vector<Fraqtion> b1 = initialB;
    vector<Fraqtion> b2 = initialB;

    a1.push_back(vector<Fraqtion>(n, 0));
    a1[m][realIndex] = 1;
    b1.push_back(b);

    a2.push_back(vector<Fraqtion>(n, 0));
    a2[m][realIndex] = -1;
    b2.push_back(-b - 1);

    // разбиваем задачу на 2 ветки решения
    Simplex simplex1(a1, b1, initialC, mode, padding + 6);
    Simplex simplex2(a2, b2, initialC, mode, padding + 6);

    cout << string(padding, ' ') << "Divide to tasks: x" << (realIndex + 1) << " <= " << b << " and x" << (realIndex + 1) << " >= " << (b + 1) << endl;

    // запускаем решение для каждой ветки
    vector<SimplexSolve> solve1 = simplex1.SolveIntegerBranchesAndBorders(debug, depth + 1);
    vector<SimplexSolve> solve2 = simplex2.SolveIntegerBranchesAndBorders(debug, depth + 1);
    vector<SimplexSolve> solves;

    solves.insert(solves.begin(), solve1.begin(), solve1.end());
    solves.insert(solves.begin(), solve2.begin(), solve2.end());

    return solves; // возвращаем решения
}

// поиск решений методом грубой силы
vector<SimplexSolve> Simplex::SolveIntegerBruteforce(int nmax) {
    vector<SimplexSolve> solves;
    cout << "Bruteforce solves: " << endl;

    for (int x1 = 0; x1 <= nmax; x1++) {
        for (int x2 = 0; x2 <= nmax; x2++) {
            for (int x3 = 0; x3 <= nmax; x3++) {
                SimplexSolve solve;
                solve.x = { x1, x2, x3 };

                if (!CheckSolve(solve.x)) // если решение не подходит под ограничения
                    continue; // то пропускаем

                solve.f = 0;

                for (int i = 0; i < n; i++)
                    solve.f += c[i] * solve.x[i]; // считаем значение функции

                PrintSolve(solve);
                solves.push_back(solve); // добавляем решение
            }
        }
    }

    return solves; // возвраащем решения
}

// поиск решения методом Гомори
vector<SimplexSolve> Simplex::SolveGomory(bool debug) {
    cout << string(padding, ' ') << "Start solving task:" << endl;
    PrintTask();

    // если решение не было найдено, то добавляем пустое решение
    if (!Solve(debug))
        return {};

    SimplexSolve solve = GetSolve(); // получаем решение
    PrintSolve(solve);
    cout << endl;

    int realIndex = GetRealIndex(solve.x); // ищем вещественное решение

    // если решение содержало только целые числа, то возвращаем решение
    if (realIndex == -1)
        return { solve };

    table.push_back(vector<Fraqtion>(n + m + 2, 0));

    // ищем строку с этой базисной переменной
    int index = 0;
    while (basis[index] != realIndex)
        index++;

    // копируем текущую таблицу
    for (int i = 0; i < m; i++) {
        table[i].push_back(table[i][n + m]);
        table[i][n + m] = 0;
    }

    for (int i = 0; i < n + m; i++)
        table[m][i] = IsBasis(i) ? 0 : -Part(table[index][i]);

    table[m][n + m] = 1;
    table[m][n + m + 1] = -Part(solve.x[realIndex]);

    c.push_back(0);
    deltas.push_back(0);
    basis.push_back(realIndex);

    m++;

    cout << "Add GOMORY restriction" << endl;
    return SolveGomory(debug);
}

// поиск лучшего из решений
void Simplex::FindBestSolve(const vector<SimplexSolve> &solves) const {
    int bestIndex = 0;

    for (int i = 0; i < solves.size(); i++) {
        if (mode == SimplexMode::Max && solves[i].f > solves[bestIndex].f) {
            bestIndex = i;
        }
        else if (mode == SimplexMode::Min && solves[i].f < solves[bestIndex].f) {
            bestIndex = i;
        }
    }

    cout << "Best solve:";
    PrintSolve(solves[bestIndex]);
}
