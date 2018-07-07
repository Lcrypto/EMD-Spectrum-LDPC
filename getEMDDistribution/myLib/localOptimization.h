/*
Copyright(c) 2012, Ilya Vorobyev und Vasiliy Usatyuk
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :
*Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and / or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#include"..\myLib\CycleEnum.h"
#include<iostream>

struct Tiii {
    int first, second, third;
    Tiii() {}
    Tiii(int _first, int _second, int _third) : first(_first), second(_second), third(_third) {}
    Tiii(const entry& e) : first(e.r), second(e.c), third(e.id) {}
    bool operator==(const Tiii& rhs) const {
        return (first == rhs.first) && (second == rhs.second) && (third == rhs.third);
    }
    bool operator!=(const Tiii& rhs) const {
        return (first != rhs.first) || (second != rhs.second) || (third != rhs.third);
    }
    bool operator<(const Tiii& rhs) const {
        return (first < rhs.first) ||
            ((first == rhs.first) && (second < rhs.second)) ||
            ((first == rhs.first) && (second == rhs.second) && (third == rhs.third));
    }
};

class LocalOpt {
    struct Cycle {
        vector<Tiii> cycle;
        int uniqueNodes = 1;
        bool oneEntry = true;
        Cycle(const vector<entry>& _cycle) {
            cycle.resize(_cycle.size());
            for (size_t i = 0; i < cycle.size(); ++i)
                cycle[i] = _cycle[i];
            for (int i = 1; i < cycle.size(); ++i) {
                if (cycle[i] != cycle[i - 1]) {
                    oneEntry = false;
                    break;
                }
            }
            for (int i = 1; i < cycle.size(); ++i) {
                ++uniqueNodes;
                for (int j = 0; j < i; ++j)
                    if (cycle[i] == cycle[j]) {
                        --uniqueNodes;
                        break;
                    }
            }
        }
    };
    int checkNodes, variableNodes;
    vector<vector<int> > protograph;
    long long circulant, targetGirth; 
    vector<vector<vector<int> > > mtr;
    bool noCycles = false;
public:

    LocalOpt(long long _circulant, long long _targetGirth, const vector<vector<vector<int> > >& _mtr) {
        circulant = _circulant, targetGirth = _targetGirth, mtr = _mtr;
        checkNodes = mtr.size(), variableNodes = mtr[0].size();
        protograph.assign(checkNodes, vector<int>(variableNodes));
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                protograph[r][c] = mtr[r][c].size();
            }
        }
    }
    LocalOpt(long long _circulant, long long _targetGirth, const vector<vector<int> >& _mtr) {
        circulant = _circulant, targetGirth = _targetGirth;
        checkNodes = _mtr.size(), variableNodes = _mtr[0].size();
        mtr.assign(checkNodes, vector<vector<int> >(variableNodes));
        protograph.assign(checkNodes, vector<int>(variableNodes));
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                if (_mtr[r][c] != -1)
                    mtr[r][c].push_back(_mtr[r][c]);
                protograph[r][c] = mtr[r][c].size();
            }
        }
    }
    bool optimize() {
        vector<vector<vector<vector<int> > > > numberOfCyclesWithThisValues(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<vector<int> > > > numberOfCyclesOfFixedLenContainsThisEntry(checkNodes, vector<vector<vector<int> > >(variableNodes));
        long long numberOfValuesToAssign = 0;
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                numberOfValuesToAssign += protograph[r][c];
                numberOfCyclesWithThisValues[r][c].assign(protograph[r][c], vector<int>(circulant));
                numberOfCyclesOfFixedLenContainsThisEntry[r][c].assign(protograph[r][c], vector<int>(targetGirth - 1, 0));
            }
        }
        for (int girth = 4; girth < targetGirth; girth += 2) {
            CycleEnum enumerator(girth, protograph);
            if (!enumerator.init()) {
                continue;
            }
            do {
                ++numberOfCyclesOfFixedLenContainsThisEntry[enumerator.cycle[0].r][enumerator.cycle[0].c][enumerator.cycle[0].id][girth];
            } while (enumerator.next());
        }
        int movesWithoutChange = 0;
        bool cycleExist = 0;
        long long moves = 0;
        while (true) {
            for (int r = 0; r < checkNodes; ++r) {
                for (int c = 0; c < variableNodes; ++c) {
                    for (int id = 0; id < protograph[r][c]; ++id) {
                        ++movesWithoutChange;
                        ++moves;
                        if (movesWithoutChange > numberOfValuesToAssign) {
                            noCycles = !cycleExist;
                            return noCycles;
                        }
                        numberOfCyclesWithThisValues[r][c][id].assign(circulant, 0);
                        for (int girth = 4; girth < targetGirth; girth += 2) {
                            int numberOfCyclesToProcess = numberOfCyclesOfFixedLenContainsThisEntry[r][c][id][girth];
                            if (numberOfCyclesToProcess == 0)
                                continue;
                            CycleEnum enumerator(girth, protograph);
                            enumerator.init(r, c, id);
                            for (int cyclesCounter = 0; cyclesCounter < numberOfCyclesToProcess; ++cyclesCounter) {
                                Cycle cycle(enumerator.cycle);
                                processCycle(cycle, numberOfCyclesWithThisValues[r][c][id], mtr, circulant);
                                enumerator.next();
                            }
                        }
                        for (int i = 0; i < circulant; ++i) {
                            if (i == mtr[r][c][id])
                                continue;
                            if (numberOfCyclesWithThisValues[r][c][id][i] < numberOfCyclesWithThisValues[r][c][id][mtr[r][c][id]]) {
                                movesWithoutChange = 0;
                                mtr[r][c][id] = i;
                                cycleExist = (numberOfCyclesWithThisValues[r][c][id][mtr[r][c][id]] > 0);
                            }
                        }
                        cycleExist = cycleExist || (numberOfCyclesWithThisValues[r][c][id][mtr[r][c][id]] > 0);
                    }
                }
            }
        }
    }
    vector<vector<vector<int> > > getMatrix() {
        return mtr;
    }
    vector<vector<int> > getRegMatrix() {
        vector<vector<int> > res(mtr.size(), vector<int>(mtr[0].size()));
        for (int i = 0; i < res.size(); ++i) {
            for (int j = 0; j < res[i].size(); ++j) {
                if (mtr[i][j].empty())
                    res[i][j] = -1;
                else
                    res[i][j] = mtr[i][j][0];
            }
        }
        return res;
    }
    bool anneal(vector<Tiii> order = vector<Tiii>()) {
        vector<vector<vector<vector<int> > > > numberOfCyclesWithTheseValues(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<vector<int> > > > numberOfCyclesOfFixedLenContainsThisEntry(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<int> > > isInQueue(checkNodes, vector<vector<int> >(variableNodes));
        long long numberOfValuesToAssign = 0;
        long long totalNumberOfCycles = 0;
        long long moves = numberOfValuesToAssign - order.size();
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                isInQueue[r][c].assign(protograph[r][c], 0);
                numberOfValuesToAssign += protograph[r][c];
                numberOfCyclesWithTheseValues[r][c].assign(protograph[r][c], vector<int>(circulant));
                numberOfCyclesOfFixedLenContainsThisEntry[r][c].assign(protograph[r][c], vector<int>(targetGirth - 1, 0));
            }
        }
        for (int i = 0; i < order.size(); ++i) {
            isInQueue[order[i].first][order[i].second][order[i].third] = 1;
        }
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                for (int i = 0; i < protograph[r][c]; ++i) {
                    if (isInQueue[r][c][i])
                        continue;
                    order.push_back(Tiii(r, c, i));
                }
            }
        }

        for (int girth = 4; girth < targetGirth; girth += 2) {
            CycleEnum enumerator(girth, protograph);
            if (!enumerator.init()) {
                continue;
            }
            do {
                ++numberOfCyclesOfFixedLenContainsThisEntry[enumerator.cycle[0].r][enumerator.cycle[0].c][enumerator.cycle[0].id][girth];
                ++totalNumberOfCycles;
            } while (enumerator.next());
        }
        int movesWithoutChange = 0;
        bool cycleExist = 0;
        while (true) {
            for (int it = 0; it < order.size(); ++it) {
                ++moves;
                int r = order[it].first, c = order[it].second, id = order[it].third;
                double temperature = 1.0 * totalNumberOfCycles / moves / moves;
                ++movesWithoutChange;
                if (movesWithoutChange > 20 * numberOfValuesToAssign) {
                    return !cycleExist;
                }
                numberOfCyclesWithTheseValues[r][c][id].assign(circulant, 0);
                for (int girth = 4; girth < targetGirth; girth += 2) {
                    int numberOfCyclesToProcess = numberOfCyclesOfFixedLenContainsThisEntry[r][c][id][girth];
                    if (numberOfCyclesToProcess == 0)
                        continue;
                    CycleEnum enumerator(girth, protograph);
                    enumerator.init(r, c, id);
                    for (int cyclesCounter = 0; cyclesCounter < numberOfCyclesToProcess; ++cyclesCounter) {
                        Cycle cycle(enumerator.cycle);
                        processCycle(cycle, numberOfCyclesWithTheseValues[r][c][id], mtr, circulant);
                        enumerator.next();
                    }
                }
                vector<int> used(circulant, 0);
                for (int i = 0; i < protograph[r][c]; ++i) {
                    if (i == id)
                        continue;
                    used[mtr[r][c][i]] = 1;
                }
                for (int i = 0; i < circulant; ++i) {
                    if (i == mtr[r][c][id])
                        continue;
                    if (used[i])
                        continue;
                    if (numberOfCyclesWithTheseValues[r][c][id][i] < numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) {
                        movesWithoutChange = 0;
                        //print(mtr);
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                    }
                }
                cycleExist = cycleExist || (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                if (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0)
                    continue;
                vector<double> prob(circulant);
                double sumProb = 0;
                for (int i = 0; i < circulant; ++i) {
                    if (used[i])
                        continue;
                    prob[i] = exp(-(numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) / temperature);
                    /*if (i != mtr[r][c][id])
                        prob[i] /= circulant;*/
                    /*if ((numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0) && (i != mtr[r][c][id]))
                        prob[i] = exp(-0.5 / temperature);*/
                    sumProb += prob[i];
                }
                double randMove = sumProb * rand() / RAND_MAX;
                double sum = 0;
                for (int i = 0; i < circulant; ++i) {
                    sum += prob[i];
                    if (sum > randMove) {
                        if (mtr[r][c][id] == i)
                            break;
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        if (numberOfCyclesWithTheseValues[r][c][id][i] != numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) 
                            movesWithoutChange = 0;
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                        break;
                    }
                }
            }           
        }
    }


    bool annealWithFixed(const vector<vector<vector<bool> > >& fixed, vector<Tiii> order = vector<Tiii>()) {
        vector<vector<vector<vector<int> > > > numberOfCyclesWithTheseValues(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<vector<int> > > > numberOfCyclesOfFixedLenContainsThisEntry(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<int> > > isInQueue(checkNodes, vector<vector<int> >(variableNodes));
        long long numberOfValuesToAssign = 0;
        long long totalNumberOfCycles = 0;
        long long moves = numberOfValuesToAssign - order.size();
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                isInQueue[r][c].assign(protograph[r][c], 0);
                numberOfValuesToAssign += protograph[r][c];
                numberOfCyclesWithTheseValues[r][c].assign(protograph[r][c], vector<int>(circulant));
                numberOfCyclesOfFixedLenContainsThisEntry[r][c].assign(protograph[r][c], vector<int>(targetGirth - 1, 0));
            }
        }
        for (int i = 0; i < order.size(); ++i) {
            isInQueue[order[i].first][order[i].second][order[i].third] = 1;
        }
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                for (int i = 0; i < protograph[r][c]; ++i) {
                    if (isInQueue[r][c][i])
                        continue;
                    order.push_back(Tiii(r, c, i));
                }
            }
        }

        for (int girth = 4; girth < targetGirth; girth += 2) {
            CycleEnum enumerator(girth, protograph);
            if (!enumerator.init()) {
                continue;
            }
            do {
                ++numberOfCyclesOfFixedLenContainsThisEntry[enumerator.cycle[0].r][enumerator.cycle[0].c][enumerator.cycle[0].id][girth];
                ++totalNumberOfCycles;
            } while (enumerator.next());
        }
        int movesWithoutChange = 0;
        bool cycleExist = 0;
        while (true) {
            for (int it = 0; it < order.size(); ++it) {
                
                /*for (int i = 0; i < mtr.size(); ++i) {
                    for (int j = 0; j < mtr[i].size(); ++j) {
                        if (mtr[i][j].empty())
                            cerr << -1 << " ";
                        else
                            cerr << mtr[i][j][0] << " ";
                    }
                    cerr << endl;
                }
                cerr << endl;*/



                ++moves;
                int r = order[it].first, c = order[it].second, id = order[it].third;
                if (fixed[r][c][id])
                    continue;
                double temperature = 1.0 * totalNumberOfCycles / moves / moves;
                ++movesWithoutChange;
                if (movesWithoutChange > 20 * numberOfValuesToAssign) {
                    return !cycleExist;
                }
                numberOfCyclesWithTheseValues[r][c][id].assign(circulant, 0);
                for (int girth = 4; girth < targetGirth; girth += 2) {
                    int numberOfCyclesToProcess = numberOfCyclesOfFixedLenContainsThisEntry[r][c][id][girth];
                    if (numberOfCyclesToProcess == 0)
                        continue;
                    CycleEnum enumerator(girth, protograph);
                    enumerator.init(r, c, id);
                    for (int cyclesCounter = 0; cyclesCounter < numberOfCyclesToProcess; ++cyclesCounter) {
                        Cycle cycle(enumerator.cycle);
                        processCycle(cycle, numberOfCyclesWithTheseValues[r][c][id], mtr, circulant);
                        enumerator.next();
                    }
                }
                for (int i = 0; i < circulant; ++i) {
                    if (i == mtr[r][c][id])
                        continue;
                    if (numberOfCyclesWithTheseValues[r][c][id][i] < numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) {
                        movesWithoutChange = 0;
                        //print(mtr);
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                    }
                }
                cycleExist = cycleExist || (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                if (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0)
                    continue;
                vector<double> prob(circulant);
                double sumProb = 0;
                for (int i = 0; i < circulant; ++i) {
                    prob[i] = exp(-(numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) / temperature);
                    /*if (i != mtr[r][c][id])
                    prob[i] /= circulant;*/
                    /*if ((numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0) && (i != mtr[r][c][id]))
                    prob[i] = exp(-0.5 / temperature);*/
                    sumProb += prob[i];
                }
                double randMove = sumProb * rand() / RAND_MAX;
                double sum = 0;
                for (int i = 0; i < circulant; ++i) {
                    sum += prob[i];
                    if (sum > randMove) {
                        if (mtr[r][c][id] == i)
                            break;
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        if (numberOfCyclesWithTheseValues[r][c][id][i] != numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]])
                            movesWithoutChange = 0;
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                        break;
                    }
                }
            }
        }
    }




    long long annealOptimizeNumberOfCycles(vector<Tiii> order = vector<Tiii>()) {
        vector<vector<vector<vector<int> > > > numberOfCyclesWithTheseValues(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<vector<int> > > > numberOfCyclesOfFixedLenContainsThisEntry(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<int> > > isInQueue(checkNodes, vector<vector<int> >(variableNodes));
        long long numberOfValuesToAssign = 0;
        long long totalNumberOfCycles = 0;
        long long moves = numberOfValuesToAssign - order.size();
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                isInQueue[r][c].assign(protograph[r][c], 0);
                numberOfValuesToAssign += protograph[r][c];
                numberOfCyclesWithTheseValues[r][c].assign(protograph[r][c], vector<int>(circulant));
                numberOfCyclesOfFixedLenContainsThisEntry[r][c].assign(protograph[r][c], vector<int>(targetGirth - 1, 0));
            }
        }
        for (int i = 0; i < order.size(); ++i) {
            isInQueue[order[i].first][order[i].second][order[i].third] = 1;
        }
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                for (int i = 0; i < protograph[r][c]; ++i) {
                    if (isInQueue[r][c][i])
                        continue;
                    order.push_back(Tiii(r, c, i));
                }
            }
        }

        for (int girth = 4; girth < targetGirth; girth += 2) {
            CycleEnum enumerator(girth, protograph);
            if (!enumerator.init()) {
                continue;
            }
            do {
                ++numberOfCyclesOfFixedLenContainsThisEntry[enumerator.cycle[0].r][enumerator.cycle[0].c][enumerator.cycle[0].id][girth];
                ++totalNumberOfCycles;
            } while (enumerator.next());
        }
        int movesWithoutChange = 0;
        bool cycleExist = 0;
        while (true) {
            for (int it = 0; it < order.size(); ++it) {
                int r = order[it].first, c = order[it].second, id = order[it].third;
                
                ++moves;

                double temperature = 1.0 * totalNumberOfCycles / moves / moves;
                ++movesWithoutChange;
                if (movesWithoutChange > 20 * numberOfValuesToAssign) {
                    long long res = 0;
                    for (int r = 0; r < checkNodes; ++r) {
                        for (int c = 0; c < variableNodes; ++c) {
                            for (int i = 0; i < protograph[r][c]; ++i) {
                                res += numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]];
                            }
                        }
                    }
                    return res;
                }
                numberOfCyclesWithTheseValues[r][c][id].assign(circulant, 0);
                for (int girth = 4; girth < targetGirth; girth += 2) {
                    int numberOfCyclesToProcess = numberOfCyclesOfFixedLenContainsThisEntry[r][c][id][girth];
                    if (numberOfCyclesToProcess == 0)
                        continue;
                    CycleEnum enumerator(girth, protograph);
                    enumerator.init(r, c, id);
                    for (int cyclesCounter = 0; cyclesCounter < numberOfCyclesToProcess; ++cyclesCounter) {
                        Cycle cycle(enumerator.cycle);
                        processCycle(cycle, numberOfCyclesWithTheseValues[r][c][id], mtr, circulant, (1LL << (targetGirth / girth)));
                        enumerator.next();
                    }
                }
                for (int i = 0; i < circulant; ++i) {
                    if (i == mtr[r][c][id])
                        continue;
                    if (numberOfCyclesWithTheseValues[r][c][id][i] < numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) {
                        movesWithoutChange = 0;
                        //print(mtr);
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                    }
                }
                cycleExist = cycleExist || (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                if (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0)
                    continue;
                vector<double> prob(circulant);
                double sumProb = 0;
                for (int i = 0; i < circulant; ++i) {
                    prob[i] = exp(-(numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) / temperature);
                    /*if (i != mtr[r][c][id])
                    prob[i] /= circulant;*/
                    /*if ((numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0) && (i != mtr[r][c][id]))
                    prob[i] = exp(-0.5 / temperature);*/
                    sumProb += prob[i];
                }
                double randMove = sumProb * rand() / RAND_MAX;
                double sum = 0;
                for (int i = 0; i < circulant; ++i) {
                    sum += prob[i];
                    if (sum > randMove) {
                        if (mtr[r][c][id] == i)
                            break;
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        if (numberOfCyclesWithTheseValues[r][c][id][i] != numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]])
                            movesWithoutChange = 0;
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                        break;
                    }
                }
            }
        }
    }



    long long annealOptimizeNumberOfCyclesWithFixed(const vector<vector<vector<bool> > >& fixed, vector<Tiii> order = vector<Tiii>()) {
        vector<vector<vector<vector<int> > > > numberOfCyclesWithTheseValues(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<vector<int> > > > numberOfCyclesOfFixedLenContainsThisEntry(checkNodes, vector<vector<vector<int> > >(variableNodes));
        vector<vector<vector<int> > > isInQueue(checkNodes, vector<vector<int> >(variableNodes));
        long long numberOfValuesToAssign = 0;
        long long totalNumberOfCycles = 0;
        long long moves = numberOfValuesToAssign - order.size();
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                isInQueue[r][c].assign(protograph[r][c], 0);
                numberOfValuesToAssign += protograph[r][c];
                numberOfCyclesWithTheseValues[r][c].assign(protograph[r][c], vector<int>(circulant));
                numberOfCyclesOfFixedLenContainsThisEntry[r][c].assign(protograph[r][c], vector<int>(targetGirth - 1, 0));

            }
        }
        for (int i = 0; i < order.size(); ++i) {
            isInQueue[order[i].first][order[i].second][order[i].third] = 1;
        }
        for (int r = 0; r < checkNodes; ++r) {
            for (int c = 0; c < variableNodes; ++c) {
                for (int i = 0; i < protograph[r][c]; ++i) {
                    if (isInQueue[r][c][i])
                        continue;
                    order.push_back(Tiii(r, c, i));
                }
            }
        }

        for (int girth = 4; girth < targetGirth; girth += 2) {

            CycleEnum enumerator(girth, protograph);
            if (!enumerator.init()) {
                continue;
            }
            do {
                ++numberOfCyclesOfFixedLenContainsThisEntry[enumerator.cycle[0].r][enumerator.cycle[0].c][enumerator.cycle[0].id][girth];
                ++totalNumberOfCycles;
            } while (enumerator.next());
        }

        int movesWithoutChange = 0;
        bool cycleExist = 0;
        while (true) {

            for (int it = 0; it < order.size(); ++it) {

                int r = order[it].first, c = order[it].second, id = order[it].third;
                if (fixed[r][c][id])
                    continue;

                ++moves;

                double temperature = 1.0 * totalNumberOfCycles / moves / moves;
                //changed for fast debug
                //for debug//double temperature = 1.0 / moves / moves;



                /*cerr.precision(18);
                cerr << temperature << endl;*/
                ++movesWithoutChange;
                if (movesWithoutChange > 20 * numberOfValuesToAssign) {
                    long long res = 0;
                    for (int r = 0; r < checkNodes; ++r) {
                        for (int c = 0; c < variableNodes; ++c) {
                            for (int i = 0; i < protograph[r][c]; ++i) {
                                res += numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]];
                            }
                        }
                    }
                    return res;
                }

                numberOfCyclesWithTheseValues[r][c][id].assign(circulant, 0);
                for (int girth = 4; girth < targetGirth; girth += 2) {
                    int numberOfCyclesToProcess = numberOfCyclesOfFixedLenContainsThisEntry[r][c][id][girth];
                    if (numberOfCyclesToProcess == 0)
                        continue;
                    CycleEnum enumerator(girth, protograph);
                    enumerator.init(r, c, id);
                    for (int cyclesCounter = 0; cyclesCounter < numberOfCyclesToProcess; ++cyclesCounter) {
                        Cycle cycle(enumerator.cycle);
                        processCycle(cycle, numberOfCyclesWithTheseValues[r][c][id], mtr, circulant, (1LL << (targetGirth / girth)));
                        enumerator.next();
                    }
                }
                for (int i = 0; i < circulant; ++i) {
                    if (i == mtr[r][c][id])
                        continue;
                    if (numberOfCyclesWithTheseValues[r][c][id][i] < numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) {
                        movesWithoutChange = 0;
                        //print(mtr);
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                    }
                }
                cycleExist = cycleExist || (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                if (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0)
                    continue;
                vector<double> prob(circulant);
                double sumProb = 0;
                for (int i = 0; i < circulant; ++i) {
                    prob[i] = exp(-(numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]]) / temperature);
                    /*if (i != mtr[r][c][id])
                    prob[i] /= circulant;*/
                    /*if ((numberOfCyclesWithTheseValues[r][c][id][i] - numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] == 0) && (i != mtr[r][c][id]))
                    prob[i] = exp(-0.5 / temperature);*/
                    sumProb += prob[i];
                }
                double randMove = sumProb * rand() / RAND_MAX;
                double sum = 0;
                for (int i = 0; i < circulant; ++i) {
                    sum += prob[i];
                    if (sum > randMove) {
                        if (mtr[r][c][id] == i)
                            break;
                        /*freopen("err_log.txt", "a", stderr);
                        eprint(mtr);
                        cerr << endl;
                        fclose(stderr);*/
                        if (numberOfCyclesWithTheseValues[r][c][id][i] != numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]])
                            movesWithoutChange = 0;
                        mtr[r][c][id] = i;
                        cycleExist = (numberOfCyclesWithTheseValues[r][c][id][mtr[r][c][id]] > 0);
                        break;
                    }
                }
            }
        }
    }
private:


    long long gcd(long long a, long long b) {
        return b ? gcd(b, a % b) : a;
    }

    void gcd(long long a, long long b, long long& x, long long& y) {
        if (b == 0) {
            x = 1, y = 0;
            return;
        }
        gcd(b, a % b, x, y);
        long long q = a / b;
        long long xx = y;
        y = x - y * q;
        x = xx;
    }

    long long inverse(long long a, long long m) {
        long long x, y;
        gcd(a, m, x, y);
        return ((x % m) + m) % m;
    }

    bool lexMin(const Cycle& cycle) {
        int len = cycle.cycle.size();
        for (int i = 1; i < len; ++i) {
            if (cycle.cycle[i] != cycle.cycle[0])
                continue;
            if ((i & 1) == 0) {
                for (int j = i + 1, k = 1; k < len; ++j, ++k) {
                    if (j == len)
                        j = 0;
                    if (cycle.cycle[k] < cycle.cycle[j])
                        break;
                    if (cycle.cycle[j] < cycle.cycle[k])
                        return false;

                }
            }
            else {
                for (int j = i - 1, k = 1; k < len; --j, ++k) {
                    if (j == -1)
                        j = len - 1;
                    if (cycle.cycle[j] < cycle.cycle[k])
                        return false;
                    if (cycle.cycle[k] < cycle.cycle[j])
                        break;
                }
            }
        }
        return true;
    }

    void processCycle(const Cycle& cycle, vector<int>& numberOfCycles, const vector<vector<vector<int> > >& a, long long circulant, long long mult = 1) {
        mult = cycle.uniqueNodes;
        Tiii cur = cycle.cycle[0];
        long long c = 0, b = 0;//cx=b mod circulant
        bool one = true;
        for (int i = 0; i < cycle.cycle.size(); ++i) {
            if (cur == cycle.cycle[i]) {
                if (i & 1)
                    ++c;
                else
                    --c;
                if (i)
                    one = false;
            }
            else {
                if (i & 1)
                    b -= a[cycle.cycle[i].first][cycle.cycle[i].second][cycle.cycle[i].third];
                else
                    b += a[cycle.cycle[i].first][cycle.cycle[i].second][cycle.cycle[i].third];
            }
        }
        if (!one) {
            if (!lexMin(cycle))
                return;
        }
        c = ((c % circulant) + circulant) % circulant;
        b = ((b % circulant) + circulant) % circulant;
        if (c == 0) {
            if (b != 0)
                return;
            for (int i = 0; i < circulant; ++i)
                numberOfCycles[i] += mult;
            return;
        }
        long long d1 = gcd(c, circulant);
        if ((b % d1) != 0)
            return;
        c /= d1, b /= d1, circulant /= d1;
        long long x = (inverse(c, circulant) * b) % circulant;
        for (int i = 0; i < d1; ++i) {
            numberOfCycles[x + circulant * i] += mult;
        }
    }


    
};






