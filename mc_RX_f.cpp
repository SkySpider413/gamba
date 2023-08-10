#include <bits/stdc++.h>
using namespace std;

double mc_RX(int start_fs, int from_lvl)
{
    int q_fs;
    int rounds = 250;
    if (from_lvl == 15) {
        double q_r;
        double tries = 0;

        for (int i = 0; i < rounds; ++i) {
            q_fs = start_fs;
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > min(0.9, .1176 + .01176 * min(50, q_fs) + 0.01176 / 5.0 * max(q_fs - 50, 0))) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
                q_fs += 2;
            }
        }
        return tries / (double)rounds;
    } else if (from_lvl == 16) {
        double q_r;
        double tries = 0;
        for (int i = 0; i < rounds; ++i) {
            q_fs = start_fs;
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > min(0.9, .0769 + .00769 * min(82, q_fs) + 0.00769 / 5.0 * max(q_fs - 82, 0))) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
                q_fs += 3;
            }
        }
        return tries / (double)rounds;
    } else if (from_lvl == 17) {
        double q_r;
        double tries = 0;
        for (int i = 0; i < rounds; ++i) {
            q_fs = start_fs;
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > min(0.9, .0625 + .00625 * min(102, q_fs) + 0.00625 / 5.0 * max(q_fs - 102, 0))) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
                q_fs += 4;
            }
        }
        return tries / (double)rounds;
    } else if (from_lvl == 18) {
        double q_r;
        double tries = 0;
        for (int i = 0; i < rounds; ++i) {
            q_fs = start_fs;
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > min(0.9, .02 + .002 * min(340, q_fs) + 0.002 / 5.0 * max(q_fs - 340, 0))) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
                q_fs += 5;
            }
        }
        return tries / (double)rounds;
    } else if (from_lvl == 19) {
        double q_r;
        double tries = 0;
        for (int i = 0; i < rounds; ++i) {
            q_fs = start_fs;
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > min(0.9, 0.003 + .0003 * min(2324, q_fs) + 0.0003 / 5.0 * max(q_fs - 2324, 0))) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
                q_fs += 6;
            }
        }
        return tries / (double)rounds;
    } else {
        return 0;
    }
}