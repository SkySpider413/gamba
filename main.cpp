#include <bits/stdc++.h>
using namespace std;
#define ff first
#define ss second
#define MPTAX 0.84825
#define MIL 1000000.0
const double CLEANSE = -100000;
const int BASE_FS = 0;
const double HARDCAP_CHANCE = 0.9;
const int CALCULATION_LIMIT = 6990;

const int DEVOUR_ARMOR[21] = { 0, 0, 0, 0, 0, 0, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 25, 30, 40, 50, 70 };
const int DEVOUR_WEAPON[21] = { 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 8, 10, 12, 14, 16, 28, 33, 43, 53, 75 };

struct Material {
    string mat_name = "uninitialized";
    double get_profit = numeric_limits<double>::infinity();
};

struct Accessory {
    string acc_name = "uninitialized";
    int item_id = -1;
    int soft_cap[5] = { 18, 40, 44, 110, 490 };
    double per_fs[5] = { .025, .01, .0075, .0025, .0005 };
    double post_soft[5] = { .005, .002, .0015, .0005, .0001 };
    double base_chance[5] = { .25, .1, .075, .025, .005 };
    double mp_price[6];
    double get_profit[6];

    Accessory()
    {
        for (int i = 0; i < 6; ++i)
            get_profit[i] = -numeric_limits<double>::infinity();
    }

    void blank()
    {
        acc_name = "uninitialized";
        item_id = -1;
        for (int i = 0; i < 6; ++i) {
            mp_price[6] = 0;
            get_profit[i] = -numeric_limits<double>::infinity();
        }
    }

    double ench_chance(int fs, int from_lvl)
    {
        fs += BASE_FS;
        return min(0.9, base_chance[from_lvl] + per_fs[from_lvl] * min(soft_cap[from_lvl], fs) + post_soft[from_lvl] * max(fs - soft_cap[from_lvl], 0));
    }
};

struct FStack {
    int stack_height;
    double get_profit = numeric_limits<double>::infinity();
    double value = -numeric_limits<double>::infinity();
    string get_comments = "uninitialized";
    string val_comments = "uninitialized";
    bool is_profit() { return (get_profit + value > 0); }
};

unordered_map<int, FStack> fs_info;
unordered_map<int, Accessory> acc_info;
unordered_map<int, Material> mat_info;

void zbicie_kijem(double v) // cout v with k/M/b/T
{
    if (abs(v) < 1000) {
        cout << setprecision(3) << v;
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "k";
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "M";
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "b";
        return;
    }
    v /= 1000;
    cout << setprecision(3) << v << "T";
    return;
}

int u_hash(string s) // hashuje string do inta
{
    int h = 0;
    for (int i = 0; i < s.size(); ++i) {
        h *= 2137;
        h += s[i];
    }
    return h;
}

void init();
void mc6990();

int main()
{
    init();
    return 0;
}

void init() // initialize item data
{
    cout << "init start----------------------------------------------------------\n";

    // crescent
    {
        Accessory q_acc;
        q_acc.acc_name = "Crescent";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 62000000, 112000000, 437000000, 1390000000, 5250000000, 48000000000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i];
        acc_info[q_acc.item_id] = q_acc;
    }

    // narc
    {
        Accessory q_acc;
        q_acc.acc_name = "Narc";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 44400000, 103000000, 416000000, 1160000000, 5000000000, 44000000000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i];
        acc_info[q_acc.item_id] = q_acc;
    }

    // tungr
    {
        Accessory q_acc;
        q_acc.acc_name = "TungradRing";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 149000000, 401000000, 1080000000, 3120000000, 10200000000, 94500000000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i];
        acc_info[q_acc.item_id] = q_acc;
    }

    // distro
    {
        Accessory q_acc;
        q_acc.acc_name = "BlackDistortion";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 339000000, 740000000, 2100000000, 6100000000, 20300000000, 140000000000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i];
        acc_info[q_acc.item_id] = q_acc;
    }

    // valtarra
    {
        Accessory q_acc;
        q_acc.acc_name = "Valtarra";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 53.5, 91.5, 406, 1230, 4950, 48900 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }
}

void mc6990() // monte carlo v(fs=6990) using TET->PEN acc
{
    cout << "mc6990 start----------------------------------------------------------\n";
    unordered_map<int, Accessory>::iterator itr;
    for (itr = acc_info.begin(); itr != acc_info.end(); ++itr) {
        pair<int, Accessory> q;
        q = *itr;

        double BD0 = q.second.mp_price[0];
        double BD4 = q.second.mp_price[4];
        double chance = 0.9;
        double BD5 = q.second.mp_price[5] * chance * MPTAX;
        int rounds = 1000000;
        double tries = 0;
        double q_r;
        for (int i = 0; i < rounds; ++i) {
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > chance) {
                q_r = (double)rand() / (double)RAND_MAX;
                ++tries;
            }
        }
        double q_av = BD5 - (BD0 + BD4) * double(tries) / rounds;
        if (fs_info[CALCULATION_LIMIT].value < q_av) {
            fs_info[CALCULATION_LIMIT].value = q_av;
            fs_info[CALCULATION_LIMIT].val_comments = "mc6900 with " + q.second.acc_name;
        }
    }
    cout << " calc limit stack worth ";
    zbicie_kijem(fs_info[CALCULATION_LIMIT].value);
    cout << " by " << fs_info[CALCULATION_LIMIT].val_comments << "\n";
    return;
}