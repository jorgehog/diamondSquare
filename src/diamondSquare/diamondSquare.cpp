#include <src/diamondSquare/diamondSquare.h>

vector<vector<double > >&DiamondSquare::generate(const uint power2,
        const double H,
        const vector<double> corners,
        const long seed,
        const double sigma,
        const bool addition,
        const bool PBC,
        const uint RNG) {
    this->power2 = power2;
    this->addition = addition;
//    this->initialSigma = sigma;
    this->PBC = PBC;
    this->RNG = RNG;
    systemSize = pow(2.0, power2) + 1;
    rnd = new Random(-(abs(seed)));

    R.resize(systemSize);
    for(uint i=0; i<systemSize; i++) R[i].resize(systemSize,0);

    if (PBC) { // We need the same value in the corners if we are using periodic boundaries
        R[0][0]                       = corners[0];
        R[0][systemSize-1]            = R[0][0];
        R[systemSize-1][0]            = R[0][0];
        R[systemSize-1][systemSize-1] = R[0][0];
    } else {
        R[0][0]                       = corners[0];
        R[0][systemSize-1]            = corners[1];
        R[systemSize-1][0]            = corners[2];
        R[systemSize-1][systemSize-1] = corners[3];
    }

    runDiamondSquare(R, H, sigma);

    return R;
}

// DEBUG //
#include <iomanip>
template<typename T>
void print_matrix(const vector<vector<T> > mat) {
    for (typename std::vector<vector<T> >::const_iterator i = mat.begin(); i != mat.end(); ++i) {
        for (typename std::vector<T>::const_iterator j = i->begin(); j != i->end(); ++j) {
            cout << setw(10) << setprecision(5) << fixed << *j << ' ';
        }
        cout << endl;
    }
}
// DEBUG //

void DiamondSquare::runDiamondSquare(vector<vector<double> >& R, const double H, double initialSigma) {

    double sigma = initialSigma;
    uint stepLength = systemSize-1;
    uint halfStepLength = stepLength/2;

    for (uint depth = 1; depth <= power2; depth++) {

        // Squares //
        for (uint x = halfStepLength; x < systemSize - halfStepLength; x += stepLength) {
            for (uint y = halfStepLength; y < systemSize - halfStepLength; y += stepLength) {
                R[x][y] = meanOfSquare(x, y, halfStepLength, R) + sigma*random();
            }
        }
        if (addition) {
            // Add random number to all old points
            // TODO: Could optimize this loop by not looping over the right and bottom edges if using PBC...
            for (uint x = 0; x < systemSize; x += stepLength) {
                for (uint y = 0; y < systemSize; y += stepLength) {
                    R[x][y] += sigma*random();
                }
            }
            if (PBC) {
                // Set corners equal to R[0][0]
                R[0][systemSize-1] = R[0][0];
                R[systemSize-1][0] = R[0][0];
                R[systemSize-1][systemSize-1] = R[0][0];

                // Set bottom edge equal to top edge, and right edge equal to left edge
                for (uint idx = 0; idx < systemSize; idx += stepLength) {
                    R[idx][systemSize-1] = R[idx][0]; // Right/left edge
                    R[systemSize-1][idx] = R[0][idx]; // Bottom/top edge
                }
            }
        }
        sigma *= pow(0.5, 0.5*H);

        // Diamonds //
        // Every other row of diamond points, starting with the one at (0, halfStepLength)
        for (uint x = 0; x < systemSize - halfStepLength; x += stepLength) {
            for (uint y = halfStepLength; y < systemSize - halfStepLength; y += stepLength) {
                R[x][y] = meanOfDiamond(x, y, halfStepLength, R) + sigma*random();
            }
        }
        // Every other row of diamond points, starting with the one at (halfStepLength, 0)
        for (uint x = halfStepLength; x < systemSize - halfStepLength; x += stepLength) {
            for (uint y = 0; y < systemSize - halfStepLength; y += stepLength) {
                R[x][y] = meanOfDiamond(x, y, halfStepLength, R) + sigma*random();
            }
        }
        if (PBC) {
            // Set bottom edge equal to top, and right edge equal to left
            for (uint idx = halfStepLength; idx < systemSize-1; idx += halfStepLength) {
                R[idx][systemSize-1] = R[idx][0]; // Right/left edge
                R[systemSize-1][idx] = R[0][idx]; // Bottom/top edge
            }
        } else {
            // Bottom edge diamonds
            for (uint y = halfStepLength; y < systemSize - halfStepLength; y += stepLength) {
                uint x = systemSize-1;
                R[x][y] = nonPBCbottomEdgeDiamonds(x, y, halfStepLength, R) + sigma*random();
            }

            // Right edge diamonds
            for (uint x = halfStepLength; x < systemSize - halfStepLength; x+= stepLength) {
                uint y = systemSize-1;
                R[x][y] = nonPBCrightEdgeDiamonds(x, y, halfStepLength, R) + sigma*random();
            }
        }
        if (addition) {
            // Add a random number to all old points
            uint limit;
            if (PBC) {
                limit = systemSize - halfStepLength;
            } else {
                limit = systemSize;
            }
            for (uint x = 0; x < limit; x += stepLength) {
                for (uint y = 0; y < limit; y += stepLength) {
                    R[x][y] += sigma*random();
                }
            }
            for (uint x = halfStepLength; x < systemSize-halfStepLength; x += stepLength) {
                for (uint y = halfStepLength; y < systemSize-halfStepLength; y += stepLength) {
                    R[x][y] += sigma*random();
                }
            }
            if (PBC) {
                // Set corners equal
                R[0][systemSize-1] = R[0][0];
                R[systemSize-1][0] = R[0][0];
                R[systemSize-1][systemSize-1] = R[0][0];

                // Set bottom edge equal to top, and right edge equal to left
                for (uint idx = halfStepLength; idx < systemSize-1; idx += halfStepLength) {
                    R[idx][systemSize-1] = R[idx][0]; // Right/left edge
                    R[systemSize-1][idx] = R[0][idx]; // Bottom/top edge
                }
            }
        }
        sigma *= pow(0.5, 0.5*H);

        stepLength /= 2;
        halfStepLength /= 2;
    }
}

double DiamondSquare::meanOfSquare(
        const uint x,
        const uint y,
        const uint halfStepLength,
        const vector<vector<double> >&R) {

    return 0.25*(
        R[x+halfStepLength][y+halfStepLength] +
        R[x+halfStepLength][y-halfStepLength] +
        R[x-halfStepLength][y+halfStepLength] +
        R[x-halfStepLength][y-halfStepLength]);
}

double DiamondSquare::meanOfDiamond(
        const uint x,
        const uint y,
        const uint halfStepLength,
        const vector<vector<double> > &R) {

    double average;

    if (x == 0) { // At top edge of system
        if (PBC) {
            average = 0.25*(
                R[x][y+halfStepLength] +
                R[x][y-halfStepLength] +
                R[x+halfStepLength][y] +
                R[R.size()-1-halfStepLength][y]);
        } else {
            average = (1.0/3.0)*(
                R[x][y+halfStepLength] +
                R[x][y-halfStepLength] +
                R[x+halfStepLength][y]);
        }
    } else if (y == 0) { // At left edge of system
        if (PBC) {
            average = 0.25*(
                R[x][y+halfStepLength] +
                R[x][R[0].size()-1-halfStepLength] +
                R[x+halfStepLength][y] +
                R[x-halfStepLength][y]);
        } else {
            average = (1.0/3.0)*(
                R[x][y+halfStepLength] +
                R[x+halfStepLength][y] +
                R[x-halfStepLength][y]);
        }
    } else {
        average = 0.25*(
            R[x][y+halfStepLength] +
            R[x][y-halfStepLength] +
            R[x+halfStepLength][y] +
            R[x-halfStepLength][y]);
    }

    return average;
}

double DiamondSquare::nonPBCbottomEdgeDiamonds(const uint x, const uint y, const uint halfStepLength, vector<vector<double> >& R) {

    return (1.0/3.0)*(
        R[x-halfStepLength][y] +
        R[x][y+halfStepLength] +
        R[x][y-halfStepLength]);
}

double DiamondSquare::nonPBCrightEdgeDiamonds(const uint x, const uint y, const uint halfStepLength, vector<vector<double> >& R) {

    return (1.0/3.0)*(
        R[x][y-halfStepLength] +
        R[x+halfStepLength][y] +
        R[x-halfStepLength][y]);
}

inline double DiamondSquare::random() {
    // Returns random number with mean 0
    if (RNG == 0) {
        return 0.0;
    } else if (RNG == 1) {
        return rnd->next_double() - 0.5;
    } else if (RNG == 2) {
        return rnd->next_gauss();
    } else {
        return NAN;
    }
}
