#include <iostream>
#include <vector>
#include <cmath>        // pow,
#include <cstdlib>      // atoi, atof, atol
#include <fstream>

#include <src/diamondSquare/diamondSquare.h>
#include <src/printImage.h>

using namespace std;

double estimate_hurst_exponent(const vector<vector<double> > &f);

int main(int nArgs, const char *argv[]) {
    int power2;
    double H;
    bool randomCorners;
    vector<double> corners; // If DiamondSquare::generate() receives an empty corners vector, it will generate random corners
    double sigma;
    double randomFactor;
    bool addition;
    bool PBC;
    int RNG;
    int seed;

    if (nArgs < 3) {
        cout << "Usage: ./diamondSquare  power2  H  optional:(randomCorners[0|1]  corner(0,0)  corner(1,0)  corner(0,1)  corner(1,1)  initial_RNG_stddv  randomFactor  addition[0|1]  PBC[0|1]  RNG[0|1|2]  seed[unsigned int])" << endl;
        exit(1);
    }

    int i = 1; // argument counter

    // arguments that are needed
    power2 = atoi(argv[i++]);
    H      = atof(argv[i++]);

    // argument that have default values
    randomCorners = nArgs > i ? atoi(argv[i++])  : true;
    if (!randomCorners) {
        corners.resize(4);
        corners[0] = nArgs > i ? atof(argv[i++])  : 0.0;
        corners[1] = nArgs > i ? atof(argv[i++])  : corners[0];
        corners[2] = nArgs > i ? atof(argv[i++])  : corners[0];
        corners[3] = nArgs > i ? atof(argv[i++])  : corners[0];
    }
    sigma        = nArgs > i ? atof(argv[i++]) : 1.0;
    randomFactor = nArgs > i ? atof(argv[i++]) : 1.0/sqrt(2.0);
    addition     = nArgs > i ? atoi(argv[i++]) : true;
    PBC          = nArgs > i ? atoi(argv[i++]) : true;
    RNG          = nArgs > i ? atoi(argv[i++]) : 2;
    seed         = nArgs > i ? atoi(argv[i++]) : 1;

    if (!addition && abs(randomFactor-1.0/sqrt(2.0)) > 0.0005) {
        cout << "Warning: If not using addition, the random number factor should be 1/sqrt(2) ~ 0.707." << endl;
    }

    cout << "--- Diamond-square settings --------------------" << endl;
    cout << "power2             = " << power2  << endl;
    cout << "H (Hurst exponent) = " << H << endl;
    cout << "random corners     = " << std::boolalpha << randomCorners << std::noboolalpha << endl;
    if (!randomCorners) {
    cout << "corners:" << endl;
        for (uint i = 0; i < 4; i++) {
            cout << "                   = ";
            if (i < corners.size())
                cout << corners[i] << " " << endl;
            else
                cout << "not set" << endl;
        }
    }
    cout << "sigma              = " << sigma << endl;
    cout << "randomFactor       = " << randomFactor << endl;
    cout << "addition           = " << std::boolalpha << addition << std::noboolalpha << endl;
    cout << "PBC                = " << std::boolalpha << PBC << std::noboolalpha << endl;
    cout << "RNG                = " << RNG << " (0 == no RNG, 1 == uniform, 2 == standard normal distribution)" << endl;
    cout << "seed               = " << seed << endl;
    cout << endl;
    long n = pow(2.0, power2) + 1;
    cout << "size               = " << n << "x" << n << endl;
    cout << "total number of points in grid = " << n*n << endl;
    cout << "------------------------------------------------" << endl;

    DiamondSquare generator(power2, RNG, seed);
    vector<vector<double> > heightMap = generator.generate(H, corners, sigma, randomFactor, addition, PBC);

//    estimate_hurst_exponent(heightMap);

    printMap(heightMap, "test.bmp");

    return 0;
}

double mean(const vector<vector<double> > &f, uint iMin, uint iMax, uint jMin, uint jMax) {
    double m = 0.0;
    for (uint i = iMin; i <= iMax; i++) {
        for (uint j = jMin; j <= jMax; j++) {
            m += f[i][j];
        }
    }
    m /= (iMax-iMin+1)*(jMax-jMin+1);
    return m;
}

double estimate_hurst_exponent(const vector<vector<double> > &f) {
    uint nMin = 2;
    uint nMax = 10;
    vector<uint> nVec;
    for (uint i = nMin; i <= nMax; i++) nVec.push_back(i);

    double theta = 0.0;
    uint N = f.size();
    vector<double> sigma_DMA_squared(nVec.size(), 0.0);
    for (uint k = 0; k < nVec.size(); k++) {
        uint n = nVec[k];
        uint mLower = ceil((n-1.0)*(1.0-theta));
        uint mUpper = -floor((n-1.0)*theta);
        for (uint i = mLower; i < N+mUpper; i++) {
            uint iMax = i - mUpper;
            uint iMin = i - mLower;
            for (uint j = mLower; j < N+mUpper; j++) {
                uint jMax = j - mUpper;
                uint jMin = j - mLower;
                sigma_DMA_squared[k] += pow(f[i][j] + mean(f, iMin, iMax, jMin, jMax), 2.0);
            }
        }
        sigma_DMA_squared[k] /= pow(N-nMax, 2.0);
    }

//    cout << "sigma_DMA^2" << endl;
//    for (uint i = 0; i < sigma_DMA_squared.size(); i++) {
//        cout << sigma_DMA_squared[i] << endl;
//    }

    return 1.0;
}

// to get QtCreator to run/debug programs correctly:
// $ echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
