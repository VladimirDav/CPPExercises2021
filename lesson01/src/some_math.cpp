#include "some_math.h"

int fibbonachiRecursive(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fibbonachiRecursive(n - 1) + fibbonachiRecursive(n - 2);

    }
    // TODO 03 реализуйте функцию которая считает числа Фибоначчи - https://ru.wikipedia.org/wiki/%D0%A7%D0%B8%D1%81%D0%BB%D0%B0_%D0%A4%D0%B8%D0%B1%D0%BE%D0%BD%D0%B0%D1%87%D1%87%D0%B8
    vector<vector<int> > v3 (vector<vector<int> > v1, vector<vector<int> > v2) {
        vector<vector<int> > ans;
        for (int i = 0; i < 2; ++i) ans[i].resize(2);
        for (auto &a:ans) for (auto &a1:a) a1 = 0;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                for (int k = 0; k < 2; ++k)
                    ans[i][k] += v1[i][j] * v2[j][k];
        return ans;
    };
vector<vector<int> > v4(vector<vector<int> > v, int n) {
    if (n == 1) return v;
    if (n % 2 == 0) {
        vector<vector<int> > c = v4(v, n / 2);
        return bh(u, u);
    } else {
        vector<vector<int> > c = v4(v, n - 1);
        return bh(u, v);
    }
};
int fibbonachiFast(int n) {
    // 04 реализуйте быструю функцию Фибоначчи с использованием std::vector
    if (n == 0) return 0;
    if(n==1) return 1;
    vector<vector<int> > v;
    v = {{1, 1},
         {1, 0}};
    vector<vector<int> > c = v4(v, n-1);
    return c[0][0];
}
}

// TODO 04 реализуйте быструю функцию Фибоначчи с использованием std::vector



double solveLinearAXB(double a, double b) {
    // TODO 10 решите линейное уравнение a*x+b=0 а если решения нет - верните наибольшее значение double - найдите как это сделать в интернете по запросу "so cpp double max value" (so = stackoverflow = сайт где часто можно найти ответы на такие простые запросы), главное НЕ КОПИРУЙТЕ ПРОСТО ЧИСЛО, ПОЖАЛУЙСТААаа
    // если решений сколь угодно много - верните максимальное значение со знаком минус
    const double md = std::numeric_limits<double>::max();
    if (a == 0 && b == 0) return (-md);
    if (a == 0) return md;
    return (-b / a);
}

std::vector<double> solveSquare(double a, double b, double c) {
    vector<double> result;
    if(a==0)
    {
        if(b==0) return result;
        else return {-c/b};
    }
    if (b * b == (4 * a * c)) {
        result.push_back((-b) / (2 * a));
        return result;
    }
    if (b * b < (4 * a * c)) {
        return result;
    }
    result.push_back((-b + sqrt(b * b - 4 * a * c)) / (2 * a));
    result.push_back((-b - sqrt(b * b - 4 * a * c)) / (2 * a));
    sort(result.begin(), result.end());
    return result;
}
