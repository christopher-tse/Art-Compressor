
#include "stats.h"

stats::stats(PNG & im){

  int imWidth = im.width();
  int imHeight = im.height();
  vector < vector < double >> tempSumHueX(imWidth, vector < double>(imHeight));
  vector < vector < double >> tempSumHueY(imWidth, vector < double>(imHeight));
  vector < vector < double >> tempSumSat(imWidth, vector < double>(imHeight));
  vector < vector < double >> tempSumLum(imWidth, vector < double>(imHeight));
  vector< vector< vector< int>>> tempSumHist(imWidth, vector< vector< int>>(imHeight, vector< int>(36)));
  sumHueX = tempSumHueX;
  sumHueY = tempSumHueY;
  sumSat = tempSumSat;
  sumLum = tempSumLum;
  hist = tempSumHist;

  for (int x = 0; x < imWidth; x++) {
    for (int y = 0; y < imHeight; y++) {

      double hueX = cos(im.getPixel(x,y)->h * (PI/180));
      double hueY = sin(im.getPixel(x,y)->h * (PI/180));
      double sat = im.getPixel(x,y)->s;
      double lum = im.getPixel(x,y)->l;
      double hue = im.getPixel(x,y)->h;

      if ( x == 0 && y == 0) {
        sumHueX[x][y] = hueX;
        sumHueY[x][y] = hueY;
        sumSat[x][y] = sat;
        sumLum[x][y] = lum;

        hist[x][y][hue / 10] = 1;
      }
      if ( x != 0 && y != 0) {
        sumHueX[x][y] = sumHueX[x-1][y] + sumHueX[x][y-1] + hueX - sumHueX[x-1][y-1];
        sumHueY[x][y] = sumHueY[x-1][y] + sumHueY[x][y-1] + hueY - sumHueY[x-1][y-1];
        sumSat[x][y] = sumSat[x-1][y] + sumSat[x][y-1] + sat - sumSat[x-1][y-1];
        sumLum[x][y] = sumLum[x-1][y] + sumLum[x][y-1] + lum - sumLum[x-1][y-1];

        vector< int > t1 = hist[x-1][y];
        vector< int > t2 = hist[x][y-1];
        vector< int > t3 = hist[x-1][y-1];
        vector< int > v(36);
        for (int i = 0; i < 36; i++) {
          v[i] = (t1[i] + t2[i]) - t3[i];
        }
        hist[x][y] = v;
        hist[x][y][hue / 10] = hist[x][y][hue / 10] + 1;
      }
      if ( x == 0 && y!= 0) {
        sumHueX[x][y] = sumHueX[x][y-1] + hueX;
        sumHueY[x][y] = sumHueY[x][y-1] + hueY;
        sumSat[x][y] = sumSat[x][y-1] + sat;
        sumLum[x][y] = sumLum[x][y-1] + lum;

        hist[x][y] = hist[x][y-1];
        hist[x][y][hue / 10] = hist[x][y][hue / 10] + 1;
      }
      if ( x!= 0 && y==0) {
        sumHueX[x][y] = sumHueX[x-1][y] + hueX;
        sumHueY[x][y] = sumHueY[x-1][y] + hueY;
        sumSat[x][y] = sumSat[x-1][y] + sat;
        sumLum[x][y] = sumLum[x-1][y] + lum;

        hist[x][y] = hist[x-1][y];
        hist[x][y][hue / 10] = hist[x][y][hue / 10] + 1;
      }
    }
  }
}


long stats::rectArea(pair<int,int> ul, pair<int,int> lr){
  return ((lr.first - ul.first) + 1) * ((lr.second-ul.second) + 1);
}

HSLAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){
  int numPixels;
  double avgHueX;
  double avgHueY;
  double avgSat;
  double avgLum;
  double avgHue;
  numPixels = rectArea(ul, lr);
  avgHueX = calcAvgHueX(ul, lr, numPixels);
  avgHueY = calcAvgHueY(ul, lr, numPixels);
  avgSat = calcAvgSat(ul, lr, numPixels);
  avgLum = calcAvgLum(ul, lr, numPixels);
  avgHue = atan2(avgHueY, avgHueX) * (180 / PI);

  if (avgHue < 0) {
    avgHue = avgHue + 360;
  }

  HSLAPixel pixel(avgHue, avgSat, avgLum, 1.0);
  return pixel;
}

double stats:: calcAvgHueX(pair<int,int> ul, pair<int,int> lr, int pixels) {
  double t;

  if (ul.first == 0 && ul.second == 0) {
    t = sumHueX[lr.first][lr.second];
  } else if (ul.first == 0) {
    t = sumHueX[lr.first][lr.second] - sumHueX[lr.first][ul.second-1];
  } else if (ul.second == 0) {
    t = sumHueX[lr.first][lr.second] - sumHueX[ul.first-1][lr.second];
  } else {
    t = sumHueX[lr.first][lr.second] -
        (sumHueX[ul.first-1][lr.second] + sumHueX[lr.first][ul.second-1] - sumHueX[ul.first-1][ul.second-1]);
  }

  return t / pixels;
}

double stats:: calcAvgHueY(pair<int,int> ul, pair<int,int> lr, int pixels) {
  double t;

  if (ul.first == 0 && ul.second == 0) {
    t = sumHueY[lr.first][lr.second];
  } else if (ul.first == 0) {
    t = sumHueY[lr.first][lr.second] - sumHueY[lr.first][ul.second-1];
  } else if (ul.second == 0) {
    t = sumHueY[lr.first][lr.second] - sumHueY[ul.first-1][lr.second];
  } else {
    t = sumHueY[lr.first][lr.second] -
        (sumHueY[ul.first-1][lr.second] + sumHueY[lr.first][ul.second-1] - sumHueY[ul.first-1][ul.second-1]);
  }

  return t / pixels;
}

double stats:: calcAvgSat(pair<int,int> ul, pair<int,int> lr, int pixels) {
  double t;

  if (ul.first == 0 && ul.second == 0) {
    t = sumSat[lr.first][lr.second];
  } else if (ul.first == 0) {
    t = sumSat[lr.first][lr.second] - sumSat[lr.first][ul.second-1];
  } else if (ul.second == 0) {
    t = sumSat[lr.first][lr.second] - sumSat[ul.first-1][lr.second];
  } else {
    t = sumSat[lr.first][lr.second] -
        (sumSat[ul.first-1][lr.second] + sumSat[lr.first][ul.second-1] - sumSat[ul.first-1][ul.second-1]);
  }

  return t / pixels;
}

double stats:: calcAvgLum(pair<int,int> ul, pair<int,int> lr, int pixels) {
  double t;

  if (ul.first == 0 && ul.second == 0) {
    t = sumLum[lr.first][lr.second];
  } else if (ul.first == 0) {
    t = sumLum[lr.first][lr.second] - sumLum[lr.first][ul.second-1];
  } else if (ul.second == 0) {
    t = sumLum[lr.first][lr.second] - sumLum[ul.first-1][lr.second];
  } else {
    t = sumLum[lr.first][lr.second] -
        (sumLum[ul.first-1][lr.second] + sumLum[lr.first][ul.second-1] - sumLum[ul.first-1][ul.second-1]);
  }

  return t / pixels;
}

vector<int> stats::buildHist(pair<int,int> ul, pair<int,int> lr){
  vector< int > newHist(36);

  if (ul.first == 0 && ul.second == 0) {
    newHist = hist[lr.first][lr.second];
  } else if (ul.first == 0) {
    vector< int > t1;
    vector< int > t2;
    t1 = hist[lr.first][ul.second-1];
    t2 = hist[lr.first][lr.second];
    for (int i = 0; i < 36; i++) {
      newHist[i] = t2[i] - t1[i];
    }
  } else if (ul.second == 0) {
    vector < int > t1;
    vector < int > t2;
    t1 = hist[ul.first-1][lr.second];
    t2 = hist[lr.first][lr.second];
    for (int i = 0; i < 36; i++) {
      newHist[i] = t2[i] - t1[i];
    }
  } else {
    vector< int > t3;
    vector< int > t4;
    vector< int > t5;
    vector< int > t6;
    t3 = hist[ul.first-1][lr.second];
    t4 = hist[lr.first][ul.second-1];
    t5 = hist[ul.first-1][ul.second-1];
    t6 = hist[lr.first][lr.second];
    for (int i = 0; i < 36; i++) {
      newHist[i] = t6[i] - ((t3[i] + t4[i]) - t5[i]);
    }
  }

  return newHist;
}

// takes a distribution and returns entropy
// partially implemented so as to avoid rounding issues.
double stats::entropy(vector<int> & distn,int area){
    double entropy = 0;

    for (int i = 0; i < 36; i++) {
        if (distn[i] > 0 )
            entropy += ((double) distn[i]/(double) area)
                                    * log2((double) distn[i]/(double) area);
    }

    return  -1 * entropy;
}

double stats::entropy(pair<int,int> ul, pair<int,int> lr){
  vector< int > distn = buildHist(ul, lr);
  int area = rectArea(ul, lr);
  return entropy(distn, area);
}
