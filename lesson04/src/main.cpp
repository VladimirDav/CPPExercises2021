#include <iostream>
#include <filesystem>
#include <libutils/rasserts.h>

#include "disjoint_set.h"
#include "morphology.h"

#include <opencv2/highgui.hpp>

// TODO 100 реализуйте систему непересекающихся множеств - см. файлы disjoint_set.h и disjoint_set.cpp
// чтобы их потестировать - постарайтесь дописать сюда много разных интересных случаев:
void testingMyDisjointSets() {
    DisjointSet set(5);
    rassert(set.count_differents() == 5, 2378923791);
    for (int element = 0; element < 5; ++element) {
        rassert(set.get_set(element) == element, 23892803643);
        rassert(set.get_set_size(element) == 1, 238928031);
        DisjointSet set2(5);
        cout<<set2.get_set(0)<<endl;
        set2.union_sets(0,1);
        cout<<set2.get_set(0)<<endl;
        cout<<set2.get_set(1)<<endl;
        cout<<set2.count_differents()<<endl;
        DisjointSet set3((int) 1e7);
        for(int i=0;i<(1e7)-1;++i) set3.union_sets(i,i+1);
        cout<<set3.count_differents()<<endl;
        int time2=clock();
        cout<<setprecision(6)<<(time2-time1+0.0)/CLOCKS_PER_SEC<<endl;
    }

    // TODO 100 по мере реализации DisjointSet - добавьте здесь каких-то вызовов операции "объединение двух множеств", сразу после этого проверяя через rassert что после этого результат такой как вы ожидаете
    // TODO 100 затем попробуйте создать СНМ размера 10.000.000 - и пообъединяйте какие-нибудь элементы (в цикле), быстро ли работает? а если при подвешивании одного корня множества к другому мы не будем учитывать ранк (высоту дерева) - как быстро будет работать?
    // TODO 100 попробуйте скомпилировать программу с оптимизациями и посмотреть ускорится ли программа - File->Settings->CMake->Плюсик над Debug->и переключите его в RelWithDebInfo (чтобы были хоть какие-то отладочные символы)
}

// TODO 200 перенесите сюда основную часть кода из прошлого задания про вычитание фона по первому кадру, но:
// 1) добавьте сохранение на диск визуализации картинок:
// 1.1) картинка эталонного фона
// 1.2) картинка текущего кадра
// 1.3) картинка визуализирующая маску "похож ли пиксель текущего кадра на эталонный фон"
// 1.4) картинка визуализирующая эту маску после применения волшебства морфологии
// 1.5) картинка визуализирующая эту маску после применения волшебства СНМ (системы непересекающихся множеств)
// 2) сохраняйте эти картинки визуализации только для тех кадров, когда пользователем был нажат пробел (код 32)
// 3) попробуйте добавить с помощью нажатия каких то двух кнопок "усиление/ослабление подавления фона"
// 4) попробуйте поменять местами морфологию и СНМ
// 5) попробуйте добавить настройку параметров морфологии и СНМ по нажатию кнопок (и выводите их значения в консоль)
void backgroundMagickStreaming() {

}

int main() {
    try {
        testingMyDisjointSets();
//        backgroundMagickStreaming();
        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
