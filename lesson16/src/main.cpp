#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <chrono>
#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <set>

#include <libutils/rasserts.h>
using namespace std;
using namespace cv;
bool isPixelEmpty(cv::Vec3b color) {
    // TODO 1 реализуйте isPixelEmpty(color):
    return !(color[0]+color[1]+color[2]);
    // - верните true если переданный цвет - полностью черный (такие пиксели мы считаем пустыми)
    // - иначе верните false
    rassert(false, "325235141242153: You should do TODO 1 - implement isPixelEmpty(color)!");
    return true;
}

void run(std::string caseName) {
    cv::Mat img0 = cv::imread("lesson16/data/" + caseName + "/0.png");
    cv::Mat img1 = cv::imread("lesson16/data/" + caseName + "/1.png");
    rassert(!img0.empty(), 324789374290018);
    rassert(!img1.empty(), 378957298420019);

    int downscale = 1; // уменьшим картинку в два раза столько раз сколько указано в этой переменной (итоговое уменьшение в 2^downscale раз)
    for (int i = 0; i < downscale; ++i) {
        cv::pyrDown(img0, img0); // уменьшаем картинку в два раза (по каждой из осей)
        cv::pyrDown(img1, img1); // уменьшаем картинку в два раза (по каждой из осей)
    }

    cv::Ptr<cv::FeatureDetector> detector = cv::SIFT::create();
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);

    // Детектируем и описываем дескрипторы ключевых точек
    std::vector<cv::KeyPoint> keypoints0, keypoints1;
    cv::Mat descriptors0, descriptors1;
    detector->detectAndCompute(img0, cv::noArray(), keypoints0, descriptors0);
    detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
    std::cout << "Keypoints initially: " << keypoints0.size() << ", " << keypoints1.size() << "..." << std::endl;

    // Сопоставляем ключевые точки (находя для каждой точки из первой картинки - две ближайшие, т.е. две самые похожие, с правой картинки)
    std::vector<std::vector<cv::DMatch>> matches01;
    matcher->knnMatch(descriptors0, descriptors1, matches01, 2);
    rassert(keypoints0.size() == matches01.size(), 349723894200068);

    // Фильтруем сопоставления от шума - используя K-ratio тест
    std::vector<cv::Point2f> points0;
    std::vector<cv::Point2f> points1;
    for (int i = 0; i < keypoints0.size(); ++i) {
        int fromKeyPoint0 = matches01[i][0].queryIdx;
        int toKeyPoint1Best = matches01[i][0].trainIdx;
        float distanceBest = matches01[i][0].distance;
        rassert(fromKeyPoint0 == i, 348723974920074);
        rassert(toKeyPoint1Best < keypoints1.size(), 347832974820076);

        int toKeyPoint1SecondBest = matches01[i][1].trainIdx;
        float distanceSecondBest = matches01[i][1].distance;
        rassert(toKeyPoint1SecondBest < keypoints1.size(), 3482047920081);
        rassert(distanceBest <= distanceSecondBest, 34782374920082);

        // простой K-ratio тест, но могло иметь смысл добавить left-right check
        if (distanceBest < 0.7 * distanceSecondBest) {
            points0.push_back(keypoints0[i].pt);
            points1.push_back(keypoints1[toKeyPoint1Best].pt);
        }
    }
    rassert(points0.size() == points1.size(), 234723947289089);
    std::cout << "Matches after K-ratio test: " << points0.size() << std::endl;

    // Находим матрицу преобразования второй картинки в систему координат первой картинки
    cv::Mat H10 = cv::findHomography(points1, points0, cv::RANSAC, 3.0);
    rassert(H10.size() == cv::Size(3, 3), 3482937842900059); // см. документацию https://docs.opencv.org/4.5.1/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780
    // "Note that whenever an H matrix cannot be estimated, an empty one will be returned."

    // создаем папку в которую будем сохранять результаты - lesson16/resultsData/ИМЯ_НАБОРА/
    std::string resultsDir = "lesson16/resultsData/";
    if (!std::filesystem::exists(resultsDir)) { // если папка еще не создана
        std::filesystem::create_directory(resultsDir); // то создаем ее
    }
    resultsDir += caseName + "/";
    if (!std::filesystem::exists(resultsDir)) { // если папка еще не создана
        std::filesystem::create_directory(resultsDir); // то создаем ее
    }

    cv::imwrite(resultsDir + "0img0.jpg", img0);
    cv::imwrite(resultsDir + "1img1.jpg", img1);

    // находим куда переходят углы второй картинки
    std::vector<cv::Point2f> corners1(4);
    corners1[0] = cv::Point(0, 0); // верхний левый
    corners1[1] = cv::Point(img1.cols, 0); // верхний правый
    corners1[2] = cv::Point(img1.cols, img1.rows); // нижний правый
    corners1[3] = cv::Point(0, img1.rows); // нижний левый
    std::vector<cv::Point2f> corners10(4);
    perspectiveTransform(corners1, corners10, H10);

    // находим какой ширины и высоты наша панорама (как минимум - разрешение первой картинки, но еще нужно учесть куда перешли углы второй картинки)
    int max_x = img0.cols;
    int max_y = img0.rows;
    for (int i = 0; i < 4; ++i) {
        max_x = std::max(max_x, (int) corners10[i].x);
        max_y = std::max(max_y, (int) corners10[i].y);
    }
    int pano_rows = max_y;
    int pano_cols = max_x;

    // преобразуем обе картинки в пространство координат нашей искомой панорамы
    cv::Mat pano0(pano_rows, pano_cols, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat pano1(pano_rows, pano_cols, CV_8UC3, cv::Scalar(0, 0, 0));
    rassert(img0.type() == CV_8UC3, 3423890003123093);
    rassert(img1.type() == CV_8UC3, 3423890003123094);
    // вторую картинку просто натягиваем в соответствии с ранее найденной матрицей Гомографии
    cv::warpPerspective(img1, pano1, H10, pano1.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
    // первую картинку надо положить без каких-то смещений, т.е. используя единичную матрицу:
    cv::Mat identity_matrix = cv::Mat::eye(3, 3, CV_64FC1);
    cv::warpPerspective(img0, pano0, identity_matrix, pano1.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

    cv::imwrite(resultsDir + "2pano0.jpg", pano0);
    cv::imwrite(resultsDir + "3pano1.jpg", pano1);

    // давайте сделаем наивную панораму - наложим вторую картинку на первую:
    cv::Mat panoBothNaive = pano0.clone();
    cv::warpPerspective(img1, panoBothNaive, H10, panoBothNaive.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
    cv::imwrite(resultsDir + "4panoBothNaive.jpg", panoBothNaive);

    // TODO 1 реализуйте isPixelEmpty(color) объявленную в начале этого файла - она пригодится нам чтобы легко понять какие пиксели в панораме пустые, какие - нет
    // (т.е. эта функция позволит дальше понимать в этот пиксель наложилась исходная картинка или же там все еще тьма)

    cv::Mat panoDiff(pano_rows, pano_cols, CV_8UC3, cv::Scalar(0, 0, 0));
    // TODO 2 вам надо заполнить panoDiff картинку так чтобы было четко ясно где pano0 картинка (объявлена выше) и pano1 картинка отличаются сильно, а где - слабо:
    vector<vector<int> > a(pano_rows);for(auto &a1:a) a1.resize(pano_cols);
    for(int i=0;i<a.size();++i) for(int j=0;j<a[i].size();++j) a[i][j]=0;
    for(int i=0;i<pano_rows;++i)
    {
        for(int j=0;j<pano_cols;++j)
        {
            Vec3b color0=pano0.at<Vec3b>(i,j);Vec3b color1=pano1.at<cv::Vec3b>(i,j);
            bool emp0=isPixelEmpty(color0);bool emp1=isPixelEmpty(color1);
            if((emp0^emp1)) {panoDiff.at<cv::Vec3b>(i,j)={255,255,255};a[i][j]=1000;} else if(emp0 && emp1) {panoDiff.at<cv::Vec3b>(i,j)={0,0,0};a[i][j]=1000;}
            else {int diff=abs(color0[0]-color1[0])+abs(color0[1]-color1[1])+abs(color0[2]-color1[2]);
                a[i][j]=diff;
                uchar col=std::min((uchar) 255,(uchar) diff);
                panoDiff.at<cv::Vec3b>(i,j)={col,col,col};}
        }
    }
    int n1=pano_rows;int m1=pano_cols;
    vector<vector<bool> > used(pano_rows);vector<vector<bool> > ok(pano_rows);vector<vector<int> > is(pano_rows);vector<vector<pair<int,int> > > pr(n1);for(int i=0;i<used.size();++i) {for(int j=0;j<pano_cols;++j) {used[i].push_back(0);ok[i].push_back(0);is[i].push_back(1e9);pr[i].push_back({-1,-1});}}
    is[n1-1][0]=0;set<pair<int,pair<int,int> > > d;for(int i=0;i<pano_rows;++i) for(int j=0;j<pano_cols;++j) d.insert({is[i][j],{i,j}});
    while(!d.empty())
    {
        pair<int,pair<int,int> > o=(*d.begin());d.erase(d.begin());
        pair<int,int> t=o.second;int x=t.first;int y=t.second;
        int w=is[x][y]+a[x][y];
        if(x-1>=0 && is[x-1][y]>w)
        {
            pr[x-1][y]={x,y};
            d.erase({is[x-1][y],{x-1,y}});
            is[x-1][y]=w;d.insert({is[x-1][y],{x-1,y}});
        }
        if(y+1<m1 && is[x][y+1]>w)
        {
            pr[x][y+1]={x,y};
            d.erase({is[x][y+1],{x,y+1}});
            is[x][y+1]=w;d.insert({is[x][y+1],{x,y+1}});
        }
    }
    vector<pair<int,int> > h;pair<int,int> cur={0,m1-1};
    while(cur.first!=(n1-1) || cur.second!=0)
    {
        h.push_back(cur);cur=pr[cur.first][cur.second];
    }
    h.push_back(cur);
    Mat panoDiff2=panoDiff.clone();
    for(auto h1:h)
    {
        ok[h1.first][h1.second]=true;
        panoDiff2.at<cv::Vec3b>(h1.first,h1.second)={0,0,255};
    }
    Mat panodiff3=panoDiff.clone();
    for(int i=0;i<n1;++i)
    {
        bool f=false;
        for(int j=0;j<m1;++j)
        {
            f=(f || ok[i][j]);
            if(f) {panodiff3.at<Vec3b>(i,j)=pano1.at<Vec3b>(i,j);}
            else {panodiff3.at<Vec3b>(i,j)=pano0.at<Vec3b>(i,j);}
        }
    }
    // сравните в этих двух картинках пиксели по одинаковым координатам (т.е. мы сверяем картинки) и покрасьте соответствующий пиксель panoDiff по этой логике:
    // - если оба пикселя пустые - проверяйте это через isPixelEmpty(color) (т.е. цвета черные) - результат тоже пусть черный
    // - если ровно один их пикселей пустой - результат пусть идеально белый
    // - иначе пусть результатом будет оттенок серого - пусть он тем светлее, чем больше разница между цветами пикселей
    // При этом сделайте так чтобы самый сильно отличающийся пиксель - всегда был идеально белым (255), т.е. выполните нормировку с учетом того какая максимальная разница яркости присутствует
    // Напоминание - вот так можно выставить цвет в пикселе:
    //  panoDiff.at<cv::Vec3b>(j, i) = cv::Vec3b(blueValue, greenValue, redValue);

    cv::imwrite(resultsDir + "5panoDiff.jpg", panoDiff);
    cv::imwrite(resultsDir + "6panoDiff2.jpg", panoDiff2);
    cv::imwrite(resultsDir + "7panoDiff3.jpg", panodiff3);
}


int main() {
    try {
        run("1_hanging"); // TODO 3 проанализируйте результаты по фотографиям с дрона - где различие сильное, где малое? почему так?
        run("2_hiking"); // TODO 4 проанализируйте результаты по фотографиям с дрона - где различие сильное, где малое? почему так?
        run("3_aero"); // TODO 5 проанализируйте результаты по фотографиям с дрона - где различие сильное, где малое? почему так?
        //run("4_your_data"); // TODO 6 сфотографируйте что-нибудь сами при этом на второй картинке что-то изменив, проведите анализ
        // TODO 7 проведите анализ результатов на базе Вопросов-Упражнений предложенных в последней статье "Урок 19: панорама и визуализация качества склейки"

        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
