#include <iostream>
#include <vector>
#include "Simplex.hpp"

using namespace std;

bool debug = true;

// поиск методом ветвей и границ
void BranchesAndBorders(const vector<vector<Fraqtion>> &a, const vector<Fraqtion> &b, const vector<Fraqtion> &c, SimplexMode mode) {
    cout << endl;
    cout << "=========================================================================================================" << endl;
    cout << "BRANCHES AND BORDERS METHOD" << endl;
    cout << "=========================================================================================================" << endl;

    Simplex simplex(a, b, c, mode);
    vector<SimplexSolve> solves = simplex.SolveIntegerBranchesAndBorders(debug); // ищем решения методом ветвей и границ
    simplex.FindBestSolve(solves); // находим лучшее решение
}

// поиск методом перебора
void Bruteforce(const vector<vector<Fraqtion>> &a, const vector<Fraqtion> &b, const vector<Fraqtion> &c, SimplexMode mode) {
    cout << endl;
    cout << "=========================================================================================================" << endl;
    cout << "BRUTE FORCE METHOD" << endl;
    cout << "=========================================================================================================" << endl;

    Simplex simplex(a, b, c, mode);
    vector<SimplexSolve> solves = simplex.SolveIntegerBruteforce(25); // ищем решения перебором
    simplex.FindBestSolve(solves); // находим лучшее решение
}

// поиск методом Гомори
void Gomory(const vector<vector<Fraqtion>> &a, const vector<Fraqtion> &b, const vector<Fraqtion> &c, SimplexMode mode) {
    cout << endl;
    cout << "=========================================================================================================" << endl;
    cout << "GOMORY METHOD" << endl;
    cout << "=========================================================================================================" << endl;

    Simplex simplex(a, b, c, mode);
    vector<SimplexSolve> solves = simplex.SolveGomory(debug); // ищем решение методом Гомори

    if (solves.size())
        simplex.PrintSolve(solves[0]); // выводим решение
}

int main() {
    vector<vector<Fraqtion>> a = {
        { 4, 1, 1 },
        { 1, 2, 0 },
        { 0, Fraqtion(1, 2), 1 }
    };

    vector<Fraqtion> b = { 4, 3, 2 };
    vector<Fraqtion> c = { 7, 5, 3 };


    SimplexMode mode = SimplexMode::Max; // режим работы

    Bruteforce(a, b, c, mode); // ищем переборным методом
    BranchesAndBorders(a, b, c, mode); // ищем методом ветвей и границ
    Gomory(a, b, c, mode); // ищем решение методом Гомори
}
