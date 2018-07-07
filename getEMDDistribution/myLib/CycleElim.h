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
#include"..\myLib\localOptimization.h"
#include"..\myLib\irregularLDPC.h"


class CycleElim {
    vector<vector<vector<int> > > mtr;
    vector<vector<int> > proto;
    int circ;
    int var, check;
    vector<int> varDeg, checkDeg;
    int splittingFactor;
public:
    CycleElim(string filename, int _splittingFactor = 2) {
        splittingFactor = _splittingFactor;
        //FILE* in = fopen(filename.c_str(), "r");
        freopen(filename.c_str(), "r", stdin);
        //fscanf(in, "%d%d%d", &var, &check, &circ);
        cin >> var >> check >> circ;
        mtr.assign(check, vector<vector<int> >(var));
        for (int i = 0; i < check; ++i) {
            for (int j = 0; j < var; ++j) {
                string s;
                //fscanf(in, "%s", &s);
                cin >> s;
                mtr[i][j] = parse(s);
            }
        }
        fclose(stdin);
        evProtoAndDeg();
    }
    CycleElim(const vector<vector<int> >& _mtr, int _circ, int _splittingFactor = 2) {
        splittingFactor = _splittingFactor;
        circ = _circ;
        check = _mtr.size();
        var = _mtr[0].size();
        mtr.resize(check);
        for (int i = 0; i < check; ++i) {
            mtr[i].resize(var);
            for (int j = 0; j < var; ++j) {
                if (_mtr[i][j] != -1)
                    mtr[i][j].push_back(_mtr[i][j]);
            }
        }
        evProtoAndDeg();
    }
    CycleElim(const vector<vector<vector<int> > >& _mtr, int _circ, int _splittingFactor = 2) {
        splittingFactor = _splittingFactor;
        circ = _circ;
        mtr = _mtr;
        evProtoAndDeg();
    }
    void eliminate(int targetGirth, int targetAce = 0) {
        vector<vector<entry> > cycles = getCycles(targetGirth, targetAce);
        while (!cycles.empty()) {
            vector<int> edgesByRows(mtr.size(), 0);
            for (int i = 0; i < cycles.size(); ++i) {
                for (int j = 0; j < cycles[i].size(); j += 2) {
                    ++edgesByRows[cycles[i][j].r];
                }
            }
            int maxInd = getMaxInd(edgesByRows);
            splitRow(cycles, maxInd, splittingFactor);
            evProtoAndDeg();
            cycles = getCycles(targetGirth, targetAce);
        }

    }

    void print(ostream& out = cout) {
        out << var << "\t" << check << "\t" << circ << endl;
        for (int i = 0; i < check; ++i) {
            for (int j = 0; j < var; ++j) {
                if (mtr[i][j].empty()) {
                    out << -1 << "\t";
                    continue;
                }
                out << mtr[i][j][0];
                for (int id = 1; id < mtr[i][j].size(); ++id) {
                    out << "&" << mtr[i][j][id];
                }
                out << "\t";
            }
            out << endl;
        }
    }
    pii getGirthAndAce() {
        for (int g = 4;; g += 2) {
            int ace = -1;
            cerr << "start to check girth = " << g << endl;
            CycleEnum enumerator(g, proto);
            if (!enumerator.init())
                continue;
            do {
                vector<entry> cycle = enumerator.cycle;
                ll sum = 0;
                for (int i = 0; i < cycle.size(); ++i) {
                    ll add = mtr[cycle[i].r][cycle[i].c][cycle[i].id];
                    if (i & 1)
                        sum += add;
                    else
                        sum -= add;
                }
                sum = ((sum % circ) + circ) % circ;
                if (sum == 0) {
                    int curAce = 0;
                    for (int i = 0; i < cycle.size(); ++i) {
                        curAce += varDeg[cycle[i].c] - 4;
                    }
                    curAce /= 2;
                    if ((ace == -1) || (curAce < ace)) {
                        ace = curAce;
                    }
                }
            } while (enumerator.next());
            if (ace != -1)
                return pii(g, ace);
        }
    }


private:
    vector<int> parse(const string& s) {
        int cur = 0;
        vector<int> res;
        for (int k = 0; k < s.size(); ++k) {
            if (s[k] == '&') {
                res.push_back(cur);
                cur = 0;
                continue;
            }
            if ((s[k] >= '0') && (s[k] <= '9')) {
                cur = cur * 10 + s[k] - '0';
                continue;
            }
            return res;
        }
        res.push_back(cur);
        return res;
    }

    void evProtoAndDeg() {
        check = mtr.size();
        var = mtr[0].size();
        proto.resize(check);
        for (int i = 0; i < check; ++i) {
            proto[i].resize(var);
            for (int j = 0; j < var; ++j) {
                proto[i][j] = mtr[i][j].size();
            }
        }
        evDeg();
    }
    void evDeg() {
        varDeg.assign(var, 0);
        checkDeg.assign(check, 0);
        for (int i = 0; i < proto.size(); ++i) {
            for (int j = 0; j < proto[i].size(); ++j) {
                varDeg[j] += proto[i][j];
                checkDeg[i] += proto[i][j];
            }
        }

    }



    


    vector<vector<entry> > getCycles(int targetGirth, int targetACE) {
        vector<vector<entry> > res;
        int upperGirth = targetGirth;
        if (targetACE == 0)
            --upperGirth;
        for (int g = 4; g <= upperGirth; g += 2) {
            //cerr << "start to check girth = " << g << endl;
            CycleEnum enumerator(g, proto);
            if (!enumerator.init())
                continue;
            do {
                vector<entry> cycle = enumerator.cycle;
                ll sum = 0;
                for (int i = 0; i < cycle.size(); ++i) {
                    ll add = mtr[cycle[i].r][cycle[i].c][cycle[i].id];
                    if (i & 1)
                        sum += add;
                    else
                        sum -= add;
                }
                sum = ((sum % circ) + circ) % circ;
                if (sum != 0)
                    continue;
                if (g < targetGirth) {
                    res.push_back(cycle);
                    continue;
                }

                if (sum == 0) {
                    int curAce = 0;
                    for (int i = 0; i < cycle.size(); ++i) {
                        curAce += varDeg[cycle[i].c] - 4;
                    }
                    curAce /= 2;
                    if (curAce < targetACE)
                        res.push_back(cycle);
                }
            } while (enumerator.next());
        }
        return res;
    }

    int getMaxInd(const vector<int>& a) {
        int maxi = a[0];
        int indMax = 0;
        for (int i = 1; i < a.size(); ++i) {
            if (maxi < a[i]) {
                maxi = a[i];
                indMax = i;
            }
        }
        return indMax;
    }

    bool nextMask(vector<int>& a, int k) {
        int n = a.size();
        int ind = n - 1;
        while ((ind >= 0) && (a[ind] == k - 1))
            --ind;
        if (ind < 0)
            return 0;
        ++a[ind];
        for (int i = ind + 1; i < n; ++i) {
            a[i] = 0;
        }
        return 1;
    }

    int getNumOfEdges(const vector<vector<int> >& g, const vector<int>& mask) {
        int res = 0;
        for (int i = 0; i < mask.size(); ++i) {
            for (int j = 0; j < i; ++j) {
                if (mask[i] == mask[j]) {
                    res += g[i][j];
                }
            }
        }
        return res;
    }

    
    void splitRow(int maxInd, const vector<int>& bestMask, int splittingFactor) {
        int newCheck = check + splittingFactor - 1;
        mtr.resize(newCheck);
        for (int i = check; i < newCheck; ++i)
            mtr[i].assign(var, vector<int>());
        for (int i = check - 1; i > maxInd; --i) {
            for (int j = 0; j < var; ++j)
                mtr[i + newCheck - check][j] = mtr[i][j];
        }
        vector<vector<int> > row = mtr[maxInd];
        for (int i = maxInd; i < maxInd + splittingFactor; ++i)
            mtr[i].assign(var, vector<int>());
        for (int i = 0, ind = 0; i < row.size(); ++i) {
            for (int id = 0; id < row[i].size(); ++id) {
                mtr[maxInd + bestMask[ind]][i].push_back(row[i][id]);
                ++ind;
            }
        }
        for (int i = 0; i < mtr.size(); ++i) {
            mtr[i].resize(var + splittingFactor - 1);
            for (int j = var; j < mtr[i].size(); ++j)
                mtr[i][j] = vector<int>();

        }
        for (int i = 0; i < splittingFactor - 1; ++i) {
            mtr[i + maxInd][i + var].push_back(0);
            mtr[i + maxInd + 1][i + var].push_back(0);
        }
    }

    void splitRow(const vector<vector<entry> >& cycles, int maxInd, int splittingFactor = 2) {
        int numCirc = 0;
        map<pii, int> indexByColAndId;
        vector<pii> colAndIdByIndex;
        for (int i = 0; i < mtr[maxInd].size(); ++i) {
            for (int id = 0; id < mtr[maxInd][i].size(); ++id) {
                colAndIdByIndex.push_back(pii(i, id));
                ++numCirc;
            }
        }

        for (int i = 0; i < colAndIdByIndex.size(); ++i)
            indexByColAndId[colAndIdByIndex[i]] = i;
        vector<vector<int> > g(numCirc, vector<int>(numCirc, 0));
        for (int i = 0; i < cycles.size(); ++i) {
            for (int j = 0; j < cycles[i].size(); ++j) {
                int j2 = j + 1;
                if (j2 == cycles[i].size())
                    j2 = 0;
                if ((cycles[i][j].r == cycles[i][j2].r) && (cycles[i][j].r == maxInd)) {
                    ++g[indexByColAndId[pii(cycles[i][j].c, cycles[i][j].id)]][indexByColAndId[pii(cycles[i][j2].c, cycles[i][j2].id)]];
                    ++g[indexByColAndId[pii(cycles[i][j2].c, cycles[i][j2].id)]][indexByColAndId[pii(cycles[i][j].c, cycles[i][j].id)]];
                }
            }
        }
        vector<int> mask(numCirc, 0);
        int res = -1;
        vector<int> bestMask;
        do {
            int cur = getNumOfEdges(g, mask);
            if ((res == -1) || (cur < res)) {
                res = cur;
                bestMask = mask;
            }
        } while (nextMask(mask, splittingFactor));
        splitRow(maxInd, bestMask, splittingFactor);
    }

    

};