#include <vector>
#include <sstream>
#include <iostream>
#include <set>
#include <stdexcept>

#include <libutils/rasserts.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>

using namespace std;
struct edge {
    int u, v; // номера вершин которые это ребро соединяет
    int w; // длина ребра (т.е. насколько длинный путь предстоит преодолеть переходя по этому ребру между вершинами)

    edge(int u, int v, int w) : u(u), v(v), w(w)
    {}
};

// Эта биективная функция по координате пикселя (строчка и столбик) + размерам картинки = выдает номер вершины
int encodeVertex(int row, int column, int nrows, int ncolumns) {
    rassert(row < nrows, 348723894723980017);
    rassert(column < ncolumns, 347823974239870018);
    int vertexId = row * ncolumns + column;
    return vertexId;
}

// Эта биективная функция по номеру вершины говорит какой пиксель этой вершине соовтетствует (эта функция должна быть симметрична предыдущей!)
cv::Point2i decodeVertex(int vertexId, int nrows, int ncolumns) {

    // TODO: придумайте как найти номер строки и столбика пикселю по номеру вершины (просто поймите предыдущую функцию и эта функция не будет казаться сложной)
    int row = vertexId/ncolumns;
    int column =  vertexId%ncolumns;

    // сверим что функция симметрично сработала:
    rassert(encodeVertex(row, column, nrows, ncolumns) == vertexId, 34782974923035);

    rassert(row < nrows, 34723894720027);
    rassert(column < ncolumns, 3824598237592030);
    return cv::Point2i(column, row);
}

void run(int mazeNumber) {
    cout<<mazeNumber<<" mazenumber "<<endl;
    cv::Mat maze = cv::imread("lesson15/data/mazesImages/maze" + std::to_string(mazeNumber) + ".png");
    cout<<" hytg "<<endl;
    rassert(!maze.empty(), 324783479230019);
    rassert(maze.type() == CV_8UC3, 3447928472389020);
    std::cout << "Maze resolution: " << maze.cols << "x" << maze.rows << std::endl;

    int nvertices = maze.rows*maze.cols; // TODO
    bool isinv=(mazeNumber==4);
    std::vector<std::vector<edge>> a(nvertices);
    for (int j = 0; j < maze.rows; ++j) {
        for (int i = 0; i < maze.cols; ++i) {
            cv::Vec3b color = maze.at<cv::Vec3b>(j, i);
            unsigned char blue = color[0];
            unsigned char green = color[1];
            unsigned char red = color[2];
            int cell=encodeVertex(j,i,maze.rows,maze.cols);int w=blue+green+red;if(isinv) w=255*3-w;
            if(i!=(maze.cols-1)) a[cell].push_back(edge(cell,cell+1,w));
            if(i!=0) a[cell].push_back(edge(cell,cell-1,w));
            if(j!=(maze.rows-1)) a[cell].push_back(edge(cell,cell+maze.cols,w));
            if(j!=0)  a[cell].push_back(edge(cell,cell-maze.cols,w));
        }
    }
    int start, finish;
    if (mazeNumber >= 1 && mazeNumber <= 3) { // Первые три лабиринта очень похожи но кое чем отличаются...
        start = encodeVertex(300, 300, maze.rows, maze.cols);
        finish = encodeVertex(0, 305, maze.rows, maze.cols);
    } else if (mazeNumber == 4) {
        start = encodeVertex(154, 312, maze.rows, maze.cols);
        finish = encodeVertex(477, 312, maze.rows, maze.cols);
    } else if (mazeNumber == 5) { // Лабиринт в большом разрешении, добровольный (на случай если вы реализовали быструю Дейкстру с приоритетной очередью)
        start = encodeVertex(1200, 1200, maze.rows, maze.cols);
        finish = encodeVertex(0, 1220, maze.rows, maze.cols);
    } else {
        rassert(false, 324289347238920081);
    }

    const int INF = std::numeric_limits<int>::max();

    cv::Mat window = maze.clone(); // на этой картинке будем визуализировать до куда сейчас дошла прокладка маршрута

    std::vector<int> is(nvertices, INF);
    vector <int> pr(nvertices);pr[start]=(-1);
    vector<bool> used(nvertices);for(int i=0;i<nvertices;++i) used[i]=false;
    set<pair<int,int> > d;d.insert({0,start});
    is[start]=0;
    for(int i=0;i<is.size();++i) if(i!=start) d.insert({INF,i});
    int cyc=0;
    vector <int> z;
    bool dbg=false;
    while(!d.empty())
    {
        ++cyc;
        pair <int,int> o=(*d.begin());d.erase(d.begin());z.push_back(o.second);int x=o.second;used[x]=true;if(x==finish) break;
        if(dbg) if(cyc%100==0) {
                for (auto h:z) {
                    cv::Point2i p = decodeVertex(h, maze.rows, maze.cols);
                    window.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 255, 0);
                }
                z.clear();
                cv::imshow("Maze", window);
                cv::waitKey(1);
            }
        for(auto ed:a[x])
        {
            int v=ed.v;int w=ed.w;
            if(!used[v] && is[x]+w<is[v])
            {
                pr[v]=x;
                d.erase({is[v],v});is[v]=is[x]+w;d.insert({is[v],v});
            }
        }
    }
    // TODO в момент когда вершина становится обработанной - красьте ее на картинке window в зеленый цвет и показывайте картинку:
    //    cv::Point2i p = decodeVertex(the_chosen_one, maze.rows, maze.cols);
    //    window.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 255, 0);
    //    cv::imshow("Maze", window);
    //    cv::waitKey(1);
    // TODO это может тормозить, в таком случае показывайте window только после обработки каждой сотой вершины

    int cur=finish;while(cur!=(-1)) {cv::Point2i p = decodeVertex(cur, maze.rows, maze.cols);
        window.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 0, 255);cur=pr[cur];}
    // TODO обозначьте найденный маршрут красными пикселями

    // TODO сохраните картинку window на диск
    cv::imwrite("lesson15/resultsdata/maze" + std::to_string(mazeNumber) + ".png",window);
    std::cout << "Finished!" << std::endl;

    // Показываем результат пока пользователь не насладиться до конца и не нажмет Escape
    while (cv::waitKey(10) != 27) {
        cv::imshow("Maze", window);
    }
}

int main() {
    try {
        for(int i=1;i<=5;++i) run(i);
        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
