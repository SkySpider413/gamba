#include "modules/mc_RX_f.h"
#include <bits/stdc++.h>
#include <windows.h>
using namespace std;
string random_string;
#define ff first
#define ss second
#define MPTAX 0.84825
#define MIL 1000000.0
HANDLE col;

const double CLEANSE = -100000;
const int BASE_FS = 0;
const double HARDCAP_CHANCE = 0.9;
const int CALCULATION_LIMIT = 6990; // this has to be 6990 or over for mc6990 to work
const int SUMMARY_LIMIT = 120; // this is just to not show a bunch of useless info

const int DEVOUR_ARMOR[21] = { 0, 0, 0, 0, 0, 0, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 25, 30, 40, 50, 70 };
const int DEVOUR_WEAPON[21] = { 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 8, 10, 12, 14, 16, 28, 33, 43, 53, 75 };

// all the const data that does not fit into init
const double u_BSA = .119 * MIL;
const double u_CBSA = 1.5 * MIL;
const double u_BSW = .12 * MIL;
const double u_CBSW = 2.1 * MIL;
const double u_R = 12900.0;
const double u_DSF = .226 * MIL;
const double u_R14_base = 0.02;
const double u_R14_per = 0.002;
const double u_R14_post = 0.0004;
const int u_R14_soft = 340;
// end of const data

// all the needed data calculated once
double u_R_costs[5];
int u_R_fs[5];
string u_R_comments[5];
double u_R_value[5];
// end of 1 time data

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
    double get_profit = -numeric_limits<double>::infinity();
    double value = -numeric_limits<double>::infinity();
    string get_comments = "uninitialized";
    string val_comments = "uninitialized";
    bool is_profit() { return (get_profit + value > 0); }
};

unordered_map<int, FStack> fs_info;
unordered_map<int, Accessory> acc_info;
unordered_map<int, Material> mat_info;

void print_sizes()
{
    cout << "fs info size : " << sizeof(fs_info) << "\n";
    cout << "acc info size : " << sizeof(acc_info) << "\n";
    return;
}

void zbicie_kijem(double v) // cout v with k/M/b/T
{
    if (v > 0.0)
        SetConsoleTextAttribute(col, 10);
    else
        SetConsoleTextAttribute(col, 4);
    if (v > 0.0)
        cout << "+";
    if (abs(v) < 1000) {
        cout << setprecision(3) << v;
        SetConsoleTextAttribute(col, 15);
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "k";
        SetConsoleTextAttribute(col, 15);
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "M";
        SetConsoleTextAttribute(col, 15);
        return;
    }
    v /= 1000;
    if (abs(v) < 1000) {
        cout << setprecision(3) << v << "b";
        SetConsoleTextAttribute(col, 15);
        return;
    }
    v /= 1000;

    cout << setprecision(3) << v << "T";
    SetConsoleTextAttribute(col, 15);
    // cout << "\033[0m";
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

double u_R14(int target_fs)
{
    if (target_fs == 0)
        return 0;
    const double q_p = min(HARDCAP_CHANCE, u_R14_base + min(u_R14_soft, target_fs - 1) * u_R14_per + max(target_fs - 1 - u_R14_soft, 0) * u_R14_post);
    const double q_fs = fs_info[target_fs - 1].get_profit;
    const double q_pc = CLEANSE * q_p;
    const double q_s = -1 * u_BSA;
    const double q_rp = -1 * (1 - q_p) * u_R / 2;
    // cout << q_p << ' ' << q_fs << " " << q_pc << " " << q_s << " " << q_rp << "\n";
    return (q_fs + q_pc + q_s + q_rp) / (1 - q_p);
}

void init();
void fs_init();
void mc6990();
void downcrawl();
void R14();
void R20_init();
void R20_init_2();
void fs_update_2();
void fs_update();
void summary();
void acc_profits(string s, int b, int e);

// TODO: \
include Reblath 16->20 in fs price\
query about specific item \
aka "ask for sicil" -> info like summary but showing just the sicil

int main()
{
    col = GetStdHandle(STD_OUTPUT_HANDLE);
    // initialize items an stuff
    init();

    // initialize base fs costs
    fs_init();

    // calc fs upper bound
    mc6990();

    // R14 heuristic for fs costs
    R14();

    // acc heuristic for fs
    downcrawl();

    // value of Reblath Pri->Pen, possibly fs_get_profit update\
    R20_init();

    summary();

    /**/
    for (int i = 0; i < 2; ++i) {
        R20_init_2();
        summary();
        fs_update_2();
        summary();
    }
    /**/

    /*
    for (int i = 0; i < 5; ++i) // to cos nie dziala
    {
        // update fs prices
        fs_update();
        cout << "post fs update, pre R20\n";
        summary();
        // update 15+ costs
        R20_init();

        // summary
        cout << "post R20\n";
        summary();
    }
    cout << " after 2137 raounds-------------------------------------------------------------------------------------------------------------------\n";
    summary();
    /**/

    string s;
    int q_b;
    int q_e;
    while (true) {
        cin >> s;
        if (acc_info.count(u_hash(s))) {
            cout << "range?\n";
            cin >> q_b >> q_e;
            acc_profits(s, min(q_b, q_e), max(q_b, q_e));
        } else {
            cout << "name not found\n";
            unordered_map<int, Accessory>::iterator q_it1;
            pair<int, Accessory> q_ppp;
            for (q_it1 = acc_info.begin(); q_it1 != acc_info.end(); ++q_it1) {
                q_ppp = *q_it1;
                cout << q_ppp.second.acc_name << ", ";
            }
            cout << "\n";
        }
    }

    return 0;
}
// q.second.ench_chance(i, j) * q.second.mp_price[j + 1] * MPTAX + (1 - q.second.ench_chance(i, j)) * fs_info[i + 1].value - q.second.mp_price[0] - q.second.mp_price[j]
void acc_profits(string s, int b, int e)
{
    Accessory q = acc_info[u_hash(s)];
    double q_g;
    for (int i = b; i <= e; ++i) {
        cout << " fs = " << i << " profits:";
        for (int j = 0; j < 5; ++j) {
            q_g = q.ench_chance(i, j) * q.mp_price[j + 1] * MPTAX + (1 - q.ench_chance(i, j)) * fs_info[i + 1].value - q.mp_price[0] - q.mp_price[j];
            zbicie_kijem(q_g + fs_info[i].get_profit);
            cout << " ";
        }
        cout << "\n";
    }
    return;
}

void fs_init() // init fs prices with bs/dragon scale
{
    fs_info[0].get_profit = 0;
    fs_info[0].get_comments = "base";
    fs_info[0].stack_height = 0;

    fs_info[5].get_profit = -1 * min(u_BSA, u_BSW) * 5;
    fs_info[5].get_comments = "eat BS";
    fs_info[5].stack_height = 5;

    fs_info[10].get_profit = -1 * min(u_BSA, u_BSW) * 12;
    fs_info[10].get_comments = "eat BS";
    fs_info[10].stack_height = 10;

    fs_info[15].get_profit = -1 * min(u_BSA, u_BSW) * 21;
    fs_info[15].get_comments = "eat BS";
    fs_info[15].stack_height = 15;

    fs_info[20].get_profit = -1 * min(u_BSA, u_BSW) * 33;
    fs_info[20].get_comments = "eat BS";
    fs_info[20].stack_height = 20;

    fs_info[25].get_profit = -1 * min(u_BSA, u_BSW) * 53;
    fs_info[25].get_comments = "eat BS";
    fs_info[25].stack_height = 25;

    fs_info[30].get_profit = -1 * min(u_BSA, u_BSW) * 84;
    fs_info[30].get_comments = "eat BS";
    fs_info[30].stack_height = 30;

    if (fs_info[20].get_profit < -1 * u_DSF * 20) {
        fs_info[20].get_profit = -1 * u_DSF * 20;
        fs_info[20].get_comments = "DragonScale";
        fs_info[20].stack_height = 20;
    }
    if (fs_info[30].get_profit < -1 * u_DSF * 50) {
        fs_info[30].get_profit = -1 * u_DSF * 50;
        fs_info[30].get_comments = "DragonScale";
        fs_info[30].stack_height = 30;
    }
    if (fs_info[40].get_profit < -1 * u_DSF * 150) {
        fs_info[40].get_profit = -1 * u_DSF * 150;
        fs_info[40].get_comments = "DragonScale";
        fs_info[40].stack_height = 40;
    }
    if (fs_info[50].get_profit < -1 * u_DSF * 500) {
        fs_info[50].get_profit = -1 * u_DSF * 500;
        fs_info[50].get_comments = "DragonScale";
        fs_info[50].stack_height = 50;
    }
}

void init() // initialize item data
{
    cout << "init start----------------------------------------------------------\n";
    // rings------------------------------------------------------------------------

    // crescent
    {
        Accessory q_acc;
        q_acc.acc_name = "Crescent";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 53.5, 132, 466, 1380, 4700, 43900 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
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

    // Ronaros
    {
        Accessory q_acc;
        q_acc.acc_name = "Ronaros";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 7.6, 20, 75.5, 284, 1330, 11300 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Ominous
    {
        Accessory q_acc;
        q_acc.acc_name = "Ominous";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 336, 468, 1600, 4560, 18500, 142000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Cadry
    /*
    {
        Accessory q_acc;
        q_acc.acc_name = "Cadry";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 28.7, 99, 347, 945, 4600, 35400 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }
    /**/

    // earrings------------------------------------------------------------------------------------------

    // narc
    {
        Accessory q_acc;
        q_acc.acc_name = "Narc";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 46, 108, 397, 1190, 5050, 41400 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
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

    // Ethereal
    {
        Accessory q_acc;
        q_acc.acc_name = "Ethereal";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 54.5, 179, 565, 1700, 6900, 55000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // belts-----------------------------------------------------------------------------------------

    // valtarra
    {
        Accessory q_acc;
        q_acc.acc_name = "Valtarra";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 51, 93.5, 376, 1050, 4750, 45000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Orkinrad
    {
        Accessory q_acc;
        q_acc.acc_name = "Orkinrad";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 18.8, 42.1, 178, 540, 2900, 22000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Tung belet
    {
        Accessory q_acc;
        q_acc.acc_name = "TungBelt";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 89, 264, 755, 2170, 7600, 65000 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    /**/
    // Basi
    {
        Accessory q_acc;
        q_acc.acc_name = "Basi";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 35.5, 67.5, 293, 930, 4080, 35800 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }
    /**/

    // collars---------------------------------------------------------------------

    // Serap
    {
        Accessory q_acc;
        q_acc.acc_name = "Serap";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 8.9, 36.6, 70, 228, 545, 2900 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Sicill
    {
        Accessory q_acc;
        q_acc.acc_name = "Sicill";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 9.7, 29.2, 119, 412, 2090, 18500 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // River
    {
        Accessory q_acc;
        q_acc.acc_name = "River";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 48.8, 160, 437, 1390, 6100, 45900 };
        for (int i = 0; i < 6; ++i)
            q_acc.mp_price[i] = q_pr[i] * MIL;
        acc_info[q_acc.item_id] = q_acc;
    }

    // Ogre
    {
        Accessory q_acc;
        q_acc.acc_name = "Ogre";
        q_acc.item_id = u_hash(q_acc.acc_name);
        double q_pr[] = { 86, 197, 600, 2000, 7600, 70000 };
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
        double chance = HARDCAP_CHANCE; // change this if CALC_LIMIT < 6990
        double BD5 = q.second.mp_price[5] * chance * MPTAX;
        int rounds = 1000000;
        double tries = 0;
        double q_r;
        for (int i = 0; i < rounds; ++i) {
            q_r = (double)rand() / (double)RAND_MAX;
            ++tries;
            while (q_r > chance) { // and this
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

    for (int i = 1; i < 10; ++i)
        fs_info[CALCULATION_LIMIT + i].value = fs_info[CALCULATION_LIMIT].value;

    return;
}

void downcrawl() // calc v(fs) from CALC_LIMIT
{
    cout << "downcrawl start--------------------------------------------------------------\n";
    unordered_map<int, Accessory>::iterator itr;
    for (int i = CALCULATION_LIMIT - 1; i >= 0; --i) {
        for (itr = acc_info.begin(); itr != acc_info.end(); ++itr) {
            pair<int, Accessory> q;
            q = *itr;
            for (int j = 0; j < 5; ++j) {
                if (q.second.ench_chance(i, j) * q.second.mp_price[j + 1] * MPTAX + (1 - q.second.ench_chance(i, j)) * fs_info[i + 1].value - q.second.mp_price[0] - q.second.mp_price[j] > fs_info[i].value) {
                    fs_info[i].value = q.second.ench_chance(i, j) * q.second.mp_price[j + 1] * MPTAX + (1 - q.second.ench_chance(i, j)) * fs_info[i + 1].value - q.second.mp_price[0] - q.second.mp_price[j];
                    fs_info[i].val_comments = "downcrawl with " + q.second.acc_name + " >" + to_string(j + 1);
                }
            }
        }
        /*
        cout << "fs value of " << i << " = ";
        zbicie_kijem(fs_info[i].value);
        cout << " via " << fs_info[i].val_comments;
        cout << " after get of ";
        zbicie_kijem(fs_info[i].get_profit);
        cout << " for a profit of ";
        zbicie_kijem(fs_info[i].value + fs_info[i].get_profit);
        cout << "\n";
        /**/
    }

    return;
}

void R14() // calc get prices of fs via reblath 14
{
    for (int i = 1; i <= CALCULATION_LIMIT; ++i) {
        double q_r = u_R14(i);
        //debug \
        cout << "R14 in " << i << " = " << q_r << "\n";
        if (q_r > fs_info[i].get_profit) {
            fs_info[i].get_profit = q_r;
            fs_info[i].get_comments = "R14";
        }
        // cin to stop \
        cin >> random_string;
    }
    return;
}

void summary() // summary on how to get certain fs /.how to earn on them
{
    for (int i = SUMMARY_LIMIT; i >= 0; --i) {
        cout << "fs = " << i << " profit of ";
        zbicie_kijem(fs_info[i].get_profit + fs_info[i].value);
        cout << " via " << fs_info[i].get_comments << "(";
        zbicie_kijem(fs_info[i].get_profit);
        cout << ") and " << fs_info[i].val_comments << "\n";
    }
    return;
}

// todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line todo line
/*
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
/**/

// this may maybe work slightly not bad
void R20_init_2()
{
    /* assuming + ench does not add value */

    /**/
}

// this may maybe work slightly not bad
void fs_update_2()
{
}

void R20_init() // uses current data to init pri+ rocaba prices, uses fs value and fs get profit
{
    /*
    u_R_value[4] = fs_info[70].value;
    cout << "ini R20 val = ";
    zbicie_kijem(u_R_value[4]);
    cout << "\n";
    /**/
    for (int i = 0; i < 5; ++i)
        u_R_costs[i] = -numeric_limits<double>::infinity();
    double q_p;

    for (int j = CALCULATION_LIMIT - 10; j >= 0; --j) {
        for (int i = 0; i < 5; ++i) {
            if (i == 0) {
                q_p = fs_info[j].get_profit - u_BSA * 99.8953 - mc_RX(j, 15) * (u_CBSA + u_R) + u_R;
                if (q_p > u_R_costs[i]) {
                    u_R_costs[i] = q_p;
                    u_R_comments[i] = to_string(j) + ">";
                }
            } else if (i == 1) {
                q_p = fs_info[j].get_profit + u_R_costs[0] - mc_RX(j, 16) * (u_CBSA + u_R) + u_R;
                if (q_p > u_R_costs[i]) {
                    u_R_costs[i] = q_p;
                    u_R_comments[i] = to_string(j) + ">";
                }
            } else if (i == 2 || i == 3 || i == 4) {
                q_p = fs_info[j].get_profit + mc_RX(j, 15 + i) * (u_R_costs[i - 1] - u_R_costs[i - 2] - u_R - u_CBSA) + u_R;
                if (q_p > u_R_costs[i]) {
                    u_R_costs[i] = q_p;
                    u_R_comments[i] = to_string(j) + ">";
                }
            }
        }
    }

    if (fs_info[70].get_profit < u_R_costs[4]) {
        fs_info[70].get_profit = u_R_costs[4];
        fs_info[70].get_comments = "from penR";
    }

    for (int i = 0; i < 5; ++i) {
        u_R_value[i] = -fs_info[DEVOUR_ARMOR[i + 16]].get_profit;
        cout << "u_R_val[" << i << "] = ";
        zbicie_kijem(-fs_info[DEVOUR_ARMOR[i + 16]].get_profit);
        cout << " from ";
        cout << DEVOUR_ARMOR[i + 16] << "fs\n";
    }

    for (int i = 0; i < 5; ++i) {
        cout << "R " << i + 16 << " value is ";
        zbicie_kijem(u_R_value[i]);
        cout << "\n";
    }
    /*
        double q_pq[4];


        for (int i = CALCULATION_LIMIT; i >= 0; --i) {
            q_pq[0] = min(0.9, .0769 + .00769 * min(82, i) + 0.00769 / 5.0 * max(i - 82, 0));
            q_pq[1] = min(0.9, .0625 + .00625 * min(102, i) + 0.00625 / 5.0 * max(i - 102, 0));
            q_pq[2] = min(0.9, .02 + .002 * min(340, i) + 0.002 / 5.0 * max(i - 340, 0));
            q_pq[3] = min(0.9, 0.003 + .0003 * min(2324, i) + 0.0003 / 5.0 * max(i - 2324, 0));

            for (int j = 3; j >= 0; --j) {
                if (j == 0) {
                    if ((1 - q_pq[j]) * fs_info[i + 3].value - fs_info[i].value - u_CBSA - u_R > u_R_value[j]) {
                        u_R_value[j] = (1 - q_pq[j]) * fs_info[i + 3].value - fs_info[i].value - u_CBSA - u_R;
                        u_R_comments[j] = "up at " + to_string(i);
                    }
                } else if (j == 1 || j == 2 || j == 3) {
                    if (q_pq[j] * u_R_value[j + 1] + (1 - q_pq[j]) * (u_R_value[j - 1] + fs_info[i + 3].value - u_R) - fs_info[i].value - u_CBSA > u_R_value[j]) {
                        u_R_value[j] = q_pq[j] * u_R_value[j + 1] + (1 - q_pq[j]) * (u_R_value[j - 1] + fs_info[i + 3].value - u_R) - fs_info[i].value - u_CBSA;
                        u_R_comments[j] = "up at " + to_string(i);
                    }
                }
            }
        }
        /**/

    for (int i = 0; i < 5; ++i) {
        cout << "R" << i + 16 << " costs " << u_R_comments[i] << " for ";
        zbicie_kijem(u_R_costs[i]);
        cout << "\n";
    }
    return;
}

void fs_update() // te koszty R15+ cos niezbyt dzialaja
{
    cout << "in fs update-----------------------------------------------------------------\n";
    double q_p;
    for (int i = 0; i <= CALCULATION_LIMIT; ++i)
        fs_info[i].get_profit = -numeric_limits<double>::infinity();
    fs_init();
    double q;
    double q_pq[5];

    for (int i = 0; i <= CALCULATION_LIMIT; ++i) {

        // enchance chances for 15->pri, pri->duo, duo->tri, tri->tet, tet->pen
        q_pq[0] = min(0.9, .1176 + .01176 * min(50, i) + .01176 / 5.0 * max(i - 50, 0));
        q_pq[1] = min(0.9, .0769 + .00769 * min(82, i) + 0.00769 / 5.0 * max(i - 82, 0));
        q_pq[2] = min(0.9, .0625 + .00625 * min(102, i) + 0.00625 / 5.0 * max(i - 102, 0));
        q_pq[3] = min(0.9, .02 + .002 * min(340, i) + 0.002 / 5.0 * max(i - 340, 0));
        q_pq[4] = min(0.9, 0.003 + .0003 * min(2324, i) + 0.0003 / 5.0 * max(i - 2324, 0));

        q = u_R14(i + 1);
        if (q > fs_info[i + 1].get_profit) {
            fs_info[i + 1].get_profit = q;
            fs_info[i + 1].get_comments = "R14";
        }
        // the not working \
        q = (fs_info[i].get_profit + u_R_value[1] * q_pq[0] - u_CBSA - (1 - q_pq[0]) * u_R) / (1 - q_pq[0]);

        // R15->pri not including enchant \
        q = (fs_info[i].get_profit - u_CBSA - (1 - q_pq[0]) * u_R) / (1 - q_pq[0]);

        // R15->pri includin enchant value
        q = min(0.0, (fs_info[i].get_profit - u_CBSA - (1 - q_pq[0]) * u_R - q_pq[0] * u_R_value[0]) / (1 - q_pq[0]));
        if (q > fs_info[i + 2].get_profit) {
            /**/
            if (i < SUMMARY_LIMIT) {
                cout << "fs cost " << i + 2 << " updated from ";
                zbicie_kijem(fs_info[i + 2].get_profit);
                cout << " via " << fs_info[i + 2].get_comments << " to ";
                zbicie_kijem(q);
                cout << " via R15>\n";
            }
            /**/
            fs_info[i + 2].get_profit = q;
            fs_info[i + 2].get_comments = "R15>";
        }

        /*debug R15
         if (i < SUMMARY_LIMIT) {
            cout << "R15  on " << i << "->" << i + 2 << " is ";
            zbicie_kijem(q);
            cout << "\n";
            // cout << "(" << fs_info[i].get_profit << " - " << u_CBSA << "- (1 - " << q_pq[0] << ") * " << u_R << "-" << q_pq[0] << "*" << u_R_costs[0] << ") / (1 - " << q_pq[0] << ")\n";
            cout << "(" << fs_info[i].get_profit << " - " << u_CBSA << " - (1 - " << q_pq[0] << ")/(1-" << q_pq[0] << ")\n";
        }
        /**/

        // pri->duo no ench val \
        q = (fs_info[i].get_profit - u_CBSA - (1 - q_pq[1]) * u_R) / (1 - q_pq[1]);

        // pri->duo ench val
        q = min(0.0, (fs_info[i].get_profit - u_CBSA - (1 - q_pq[1]) * u_R - q_pq[1] * u_R_value[1]) / (1 - q_pq[1]));
        if (q > fs_info[i + 3].get_profit) {

            /*
            if (i < SUMMARY_LIMIT) {
                cout << "fs cost " << i + 3 << " updated from ";
                zbicie_kijem(fs_info[i + 3].get_profit);
                cout << " via " << fs_info[i + 3].get_comments << " to ";
                zbicie_kijem(q);
                cout << " via R_PRI>\n";
            }
            /**/

            fs_info[i + 3].get_profit = q;
            fs_info[i + 3].get_comments = "R_PRI>";
        }
        for (int j = 2; j < 5; ++j) {
            // insert code for TRI,TET and PEN enchanting
             // ench no val \
            q = (fs_info[i].get_profit - u_CBSA - (1 - q_pq[j]) * (u_R - u_R_costs[j - 1] + u_R_costs[j - 2])) / (1 - q_pq[j]);

            // ench val
            // q = (fs_info[i].get_profit - u_CBSA - (1 - q_pq[j]) * (u_R )- q_pq[1] * u_R_value[1]) / (1 - q_pq[1]);
            q = min(0.0, (fs_info[i].get_profit - u_CBSA + (1 - q_pq[j]) * (u_R_costs[j] - u_R_costs[j - 1]) + q_pq[j] * u_R_value[j + 1]) / (q - q_pq[j]));

            if (q > fs_info[i + j + 2].get_profit) {
                fs_info[i + j + 2].get_profit = q;
                fs_info[i + j + 2].get_comments = (j == 2) ? "R_DUO>" : ((j == 3) ? "R_TRI>" : "R_TET>");
            }
        }
    }

    return;
}
