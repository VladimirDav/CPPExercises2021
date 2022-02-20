#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <chrono>
#include <iostream>
#include <filesystem>
#include <memory>

#include <libutils/rasserts.h>

using namespace std;
void drawText(cv::Mat img, std::string text, double fontScale, int &yOffset) {
    cv::Scalar white(255, 255, 255);

    float textHeight = cv::getTextSize(text, cv::FONT_HERSHEY_DUPLEX, fontScale, 1, nullptr).height;
    yOffset += textHeight; // увеличиваем сдвиг на высоту текста в пикселях
    cv::putText(img, text, cv::Point(0, yOffset), cv::FONT_HERSHEY_DUPLEX, fontScale, white);
}

std::vector<cv::Point2f> filterPoints(std::vector<cv::Point2f> points, std::vector<unsigned char> matchIsGood) {
    rassert(points.size() == matchIsGood.size(), 234827348927016);

    vector<cv::Point2f> goodPoints;
    for (int i = 0; i < matchIsGood.size(); ++i) {
        if (matchIsGood[i]) {
            goodPoints.push_back(points[i]);
        }
    }
    return goodPoints;
}

void run() {
    const bool useWebcam = true;
    bool drawOver = true; // рисовать ли поверх наложенную картинку (можно включить-включить чтобы мигнуть картинкой и проверить качество выравнивания)
    bool drawDebug = true; // рисовать ли поверх отладочную информацию (например красный кант вокруг нарисованной поверх картинки)
    bool useSIFTDescriptor = true; // SIFT работает довольно медленно, попробуйте использовать ORB + не забудьте что тогда вам нужен другой DescriptorMatcher

    cv::Mat imgFrame, imgForDetection, imgToDraw;
    // если у вас не работает через веб. камеру - будут использоваться заготовленные картинки
    imgFrame = cv::imread("lesson14/data/1_box2/box0.png"); // пример кадра с вебкамеры, на нем мы хотим найти объект и вместо него нарисовать другую картинку
    imgForDetection = cv::imread("lesson14/data/1_box2/box1.png"); // пример картинки которую мы хотим найти на видеокадре
    imgToDraw = cv::imread("lesson14/data/1_box2/box1_nesquik.png"); // пример картинки которую мы хотим нарисовать заместо искомой
    rassert(!imgFrame.empty(), 324789374290023);
    rassert(!imgForDetection.empty(), 3789572984290019);
    rassert(!imgToDraw.empty(), 3789572984290021);

    shared_ptr<cv::VideoCapture> video;
    if (useWebcam) {
        std::cout << "Trying to open web camera..." << std::endl;
        video = std::make_shared<cv::VideoCapture>(0);
        rassert(video->isOpened(), 3482789346289027);
        cout << "Web camera video stream opened." << std::endl;
    }

    while (true) {
        cv::Mat currentFrame; // текущий кадр с веб. камеры
        if (useWebcam) {
            bool isSuccess = video->read(currentFrame);
            rassert(isSuccess, 347283947290039);
            rassert(!currentFrame.empty(), 347283947290040);
        } else {
            currentFrame = imgFrame; // или если у вас не работает OpenCV с веб. камерой - то пусть хотя бы картинка-пример используется
        }

        auto frameProcessingStartTime = std::chrono::steady_clock::now(); // замеряем сколько сейчас времени чтобы оценить FPS

        cv::Mat mainWindowImage = currentFrame.clone(); // делаем копию чтобы на ней рисовать любую отладочную информацию не портя оригинальную картинку

        {
            // TODO сопоставьте две картинки: currentFrame и imgForDetection, затем нарисуйте imgToDraw в соответствии с матрицей Гомографии
            cv::Ptr<cv::FeatureDetector> detector;
            cv::Ptr<cv::DescriptorMatcher> matcher;
            if (useSIFTDescriptor) {
                detector = cv::SIFT::create();
                matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
            } else {
                // TODO SIFT работает довольно медленно, попробуйте использовать ORB + не забудьте что тогда вам нужен другой DescriptorMatcher
                detector = cv::ORB::create();
                matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);


            }

            vector<cv::KeyPoint> keypoints0, keypoints1; // здесь будет храниться список ключевых точек
            cv::Mat descriptors0, descriptors1; // здесь будут зраниться дескрипторы этих ключевых точек
            detector->detectAndCompute(imgForDetection, cv::noArray(), keypoints0, descriptors0);
            detector->detectAndCompute(currentFrame, cv::noArray(), keypoints1, descriptors1);

            // TODO детектируйте и постройте дескрипторы у ключевых точек

            vector<std::vector<cv::DMatch>> matches01;
            matcher->knnMatch(descriptors0, descriptors1, matches01, 2); // k: 2 - указывает что мы ищем ДВЕ ближайшие точки, а не ОДНУ САМУЮ БЛИЖАЙШУЮ
            // т.к. мы для каждой точки keypoints0 ищем ближайшую из keypoints1, то сопоставлений найдено столько же сколько точек в keypoints0:
            rassert(keypoints0.size() == matches01.size(), 234728972980049);
            for (int i = 0; i < matches01.size(); ++i) {
                rassert(matches01[i].size() == 2, 3427890347902051);
                rassert(matches01[i][0].queryIdx == i, 237812974128941); // queryIdx - это индекс ключевой точки в первом векторе точек, т.к. мы для всех точек keypoints0
                rassert(matches01[i][1].queryIdx == i, 237812974128942); // ищем ближайшую в keypoints1, queryIdx == i, т.е. равен индексу очередной точки keypoints0

                rassert(matches01[i][0].trainIdx < keypoints1.size(), 237812974128943); // trainIdx - это индекс точки в keypoints1 самой похожей на keypoints0[i]
                rassert(matches01[i][1].trainIdx < keypoints1.size(), 237812974128943); // а этот trainIdx - это индекс точки в keypoints1 ВТОРОЙ по похожести на keypoints0[i]

                rassert(matches01[i][0].distance <= matches01[i][1].distance, 328493778); // давайте явно проверим что расстояние для этой второй точки - не меньше чем для первой точки
            }

            // TODO сопоставьте ключевые точки

            // Теперь давайте сопоставим ключевые точки между картинкой 1 и картинкой 0 (т.е. в обратную сторону):
            std::vector<std::vector<cv::DMatch>> matches10;
            matcher->knnMatch(descriptors1, descriptors0, matches10, 2); // k: 2 - указывает что мы ищем ДВЕ ближайшие точки, а не ОДНУ САМУЮ БЛИЖАЙШУЮ
            // т.к. мы для каждой точки keypoints0 ищем ближайшую из keypoints1, то сопоставлений найдено столько же сколько точек в keypoints0:
            rassert(keypoints1.size() == matches10.size(), 234728972980049);
            for (int i = 0; i < matches10.size(); ++i) {
                rassert(matches10[i].size() == 2, 3427890347902051);
                rassert(matches10[i][0].queryIdx == i, 237812974128941); // queryIdx - это индекс ключевой точки в первом векторе точек, т.к. мы для всех точек keypoints0
                rassert(matches10[i][1].queryIdx == i, 237812974128942); // ищем ближайшую в keypoints1, queryIdx == i, т.е. равен индексу очередной точки keypoints0

                rassert(matches10[i][0].trainIdx < keypoints0.size(), 237812974128943); // trainIdx - это индекс точки в keypoints1 самой похожей на keypoints0[i]
                rassert(matches10[i][1].trainIdx < keypoints0.size(), 237812974128943); // а этот trainIdx - это индекс точки в keypoints1 ВТОРОЙ по похожести на keypoints0[i]

                rassert(matches10[i][0].distance <= matches10[i][1].distance, 328493778); // давайте явно проверим что расстояние для этой второй точки - не меньше чем для первой точки
            }


            // TODO пофильтруйте сопоставления, как минимум через K-ratio test, но лучше на ваш выбор

            vector<cv::Point2f> points0, points1; // здесь сохраним координаты ключевых точек для удобства позже
            vector<unsigned char> matchIsGood; // здесь мы отметим true - хорошие сопоставления, и false - плохие
            int nmatchesGood = 0; // посчитаем сколько сопоставлений посчиталось хорошими
            vector<double> distances;
            for (int i = 0; i < matches01.size(); ++i) {
                distances.push_back(matches01[i][0].distance);
            }
            sort(distances.begin(), distances.end());
            for (int i = 0; i < keypoints0.size(); ++i) {
                cv::DMatch match = matches01[i][0];
                rassert(match.queryIdx == i, 234782749278097); // и вновь - queryIdx это откуда точки (поэтому всегда == i)
                int j = match.trainIdx; // и trainIdx - это какая точка из второго массива точек оказалась к нам (к queryIdx из первого массива точек) ближайшей
                rassert(j < keypoints1.size(), 38472957238099); // поэтому явно проверяем что индекс не вышел за пределы второго массива точек

                points0.push_back(keypoints0[i].pt);
                points1.push_back(keypoints1[j].pt);
                bool isOk = true;

                if (match.distance > distances[distances.size() / 2]) {
                    isOk = false;
                }

                cv::DMatch match2 = matches01[i][1];
                double K = 0.7;
                if (match.distance > K * match2.distance) {
                    isOk = false;
                }

                cv::DMatch match01 = match;
                cv::DMatch match10 = matches10[match01.trainIdx][0];
                if (match10.trainIdx != i) {
                    isOk = false;
                }

                if (isOk) {
                    ++nmatchesGood;
                    matchIsGood.push_back(true);
                } else {
                    matchIsGood.push_back(false);
                }
            }
            rassert(points0.size() == points1.size(), 3497282579850108);
            rassert(points0.size() == matchIsGood.size(), 3497282579850109);
            std::cout << nmatchesGood << "/" <<  keypoints0.size() << " good matches left" << std::endl;
            rassert(points0.size() == points1.size(), 234723947289089);

            const double ransacReprojThreshold = 3.0;
            vector<cv::Point2f> pointsGood0 = filterPoints(points0, matchIsGood);
            vector<cv::Point2f> pointsGood1 = filterPoints(points1, matchIsGood);
            vector<unsigned char> inliersMask; // в этот вектор RANSAC запишет флажки - какие сопоставления он посчитал корректными (inliers)
            cv::Mat H01;
            if(pointsGood0.size() >= 4) H01 = cv::findHomography(pointsGood0, pointsGood1, cv::RANSAC, ransacReprojThreshold, inliersMask);
            if (H01.empty()) {
                // см. документацию https://docs.opencv.org/4.5.1/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780
                // "Note that whenever an H matrix cannot be estimated, an empty one will be returned."
                std::cout << "FAIL 24123422!" << std::endl;
            } else {
                cv::Mat overlapImg = imgToDraw.clone();
                if (drawDebug) {
                    // рисуем красный край у накладываемой картинки
                    cv::Scalar red(0, 0, 255);
                    cv::rectangle(overlapImg, cv::Point(0, 0), cv::Point(overlapImg.cols-1, overlapImg.rows-1), red, 2);
                }
                if (drawOver) {


                    cv::warpPerspective(overlapImg, mainWindowImage, H01, mainWindowImage.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

                }
            }
        }

        if (drawDebug) {
            int textYOffset = 0;

            auto frameProcessingEndTime = std::chrono::steady_clock::now();
            int timeForFrame = std::chrono::duration_cast<std::chrono::milliseconds>(frameProcessingEndTime - frameProcessingStartTime).count();
            int fps;
            if (timeForFrame == 0) {
                fps = 99999;
            } else {
                fps = (int) std::round(1000.0 / timeForFrame);
            }
            drawText(mainWindowImage, to_string(fps) + " FPS", 0.5, textYOffset);


            drawText(mainWindowImage, "Controls: ", 0.5, textYOffset);
            drawText(mainWindowImage, "1 - image for detection", 0.5, textYOffset);
            drawText(mainWindowImage, "2 - image to draw", 0.5, textYOffset);
            drawText(mainWindowImage, "3 - debug info", 0.5, textYOffset);
            drawText(mainWindowImage, "H - hide/show", 0.5, textYOffset);
            drawText(mainWindowImage, "S - SIFT/ORB", 0.5, textYOffset);


        }

        // Рисуем все три окошка:
        cv::imshow("Frame", mainWindowImage);
        cv::imshow("Image for detection", imgForDetection);
        cv::imshow("Image to draw", imgToDraw);

        // Смотрим нажал ли пользователь какую-нибудь кнопку
        int key = cv::waitKey(10); // число в скобочках - максимальное число миллисекунд которые мы ждем кнопки от пользователя, а иначе - считаем что ничего не нажато
        if (key == -1) {
            // прошло 5 миллисекунд но ничего не было нажато - значит идем обрабатывать следующий кадр с веб. камеры
        } else if (key == 27) { // Esc - выключаем программу
            break;
        } else if (useWebcam && key == 49) {
            // TODO если пользователь нажал кнопку 1 (как исследовать какой код у такой кнопки? опытным путем!) - текущий кадр надо использовать как imgForDetection
            imgForDetection = currentFrame.clone();
        } else if (useWebcam && key == 50) {
            // TODO если пользователь нажал кнопку 2 - текущий кадр надо использовать как imgToDraw
            imgToDraw = currentFrame.clone();
        } else if (key == 104) {
            // TODO если пользователь нажал кнопку H (Hide) - выключите/включите рисовать картинку поверх (т.е. drawOver)
            drawOver = !drawOver;
        } else if (key == 51) {
            // TODO если пользователь нажал кнопку 3 - включить/выключить отрисовку разной отладочной информации (т.е. drawDebug)
            drawDebug = !drawDebug;
        } else if (key == 115) {
            // TODO если пользователь нажал кнопку S (SIFT) - включить/выключить использование SIFT/ORB
            useSIFTDescriptor = !useSIFTDescriptor;
        } else {
            std::cerr << "UKNOWN KEY " << key << " WAS PRESSED" << std::endl;
        }
    }
}


int main() {
    try {
        run();

        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
