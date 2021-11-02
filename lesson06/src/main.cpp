#include <filesystem>
#include <iostream>
#include <libutils/rasserts.h>
#include <vector>
#include "blur.h" // TODO реализуйте функцию блюра с произвольной силой размытия в файле blur.cpp
using namespace std;
void testSomeBlur() {
    cout<<" vfd "<<endl;
    // TODO выберите любую картинку и любую силу сглаживания - проверьте что результат - чуть размытая картинка
    cv::Mat img=cv::imread("lesson03/data/dota2.jpg").clone();
    cout<<img.rows<<' '<<img.cols<<endl;
    cv::Mat img2=blur(img,0.3);
    cv::imwrite("lesson06/resultsData/dota3.jpg",img2);
    // Входные картинки для тестирования возьмите из предыдущего урока (т.е. по пути lesson05/data/*).
    // Результирующие картинки сохарняйте в эту папку (т.е. по пути lesson06/resultsData/*).
}

void testManySigmas() {
    // TODO возьмите ту же самую картинку но теперь в цикле проведите сглаживание для нескольких разных сигм
    for(double sigma=10.0;sigma<=10.0;sigma+=1.0)
    {
        cv::Mat img=cv::imread("lesson03/data/dota2.jpg").clone();
        cv::Mat img2=blur(img,sigma);
        string h="lesson06/resultsData/dota";
        cout<<sigma<<" sigma "<<endl;
        h+=to_string(sigma);
        h+=".jpg";
        cv::imwrite(h,img2);
    }
    // при этом результирующую картинку сохраняйте с указанием какая сигма использовалась:
    // для того чтобы в название файла добавить значение этой переменной -
    // воспользуйтесь функцией преобразующей числа в строчки - std::to_string(sigma)
}

int main() {
    try {
        testSomeBlur();
        testManySigmas();

        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}

