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
#include".\myLib\irregularLDPC.h"
#include".\myLib\emdOptimization.h"
#include".\myLib\ME.h"



bool gen(int checkNodes, int variableNodes, const vector<vector<int> >& protograph, ll circulant, ll targetGirth, vector<vector<vector<int> > >& mtr) {
    mtr.assign(checkNodes, vector<vector<int> >(variableNodes));
    for (int r = 0; r < checkNodes; ++r) {
        for (int c = 0; c < variableNodes; ++c) {
            mtr[r][c].resize(protograph[r][c]);
            for (int id = 0; id < protograph[r][c]; ++id) {
                mtr[r][c][id] = getRand(circulant);
            }
        }
    }
    emdOpt opt(circulant, targetGirth, mtr);
    if (opt.annealEmd()) {
        mtr = opt.getMatrix();
        return 1;
    }
    return 0;
}

vector<vector<pair<int, int> > > getEMDDistr(const vector<vector<vector<int> > > & mtr, int circ, int upperGirth) {
    emdOpt e(circ, upperGirth, mtr);
    return e.getEMDDistr();
}


int main(int argc, char* argv[]) {
    int upperGirth = 6;
    string INPUT_FILENAME = "";
    string OUTPUT_FILENAME = "out.txt";
    cerr << "Usage:\ngetEMDDistribution.exe -file in.txt -out out.txt(default) -upperGirth 6(default)\n";
    for (int i = 1; i + 1 < argc; ++i) {
        if (string(argv[i]) == "-file") {
            INPUT_FILENAME = argv[i + 1];
            ++i;
            continue;
        }
        if (string(argv[i]) == "-out") {
            OUTPUT_FILENAME = argv[i + 1];
            ++i;
            continue;
        }
        if (string(argv[i]) == "-upperGirth") {
            string strDep = argv[i + 1];
            stringstream sstrDep(strDep);
            sstrDep >> upperGirth;
            ++i;
            continue;
        }
    }
    ll varNodes, checkNodes, circulant;
    freopen(INPUT_FILENAME.c_str(), "r", stdin);
    cin >> varNodes >> checkNodes >> circulant;
    vector<vector<vector<int> > > mtr(checkNodes, vector<vector<int> >(varNodes));
    for (int i = 0; i < checkNodes; ++i) {
        for (int j = 0; j < varNodes; ++j) {
            string toParse;
            cin >> toParse;
            mtr[i][j] = parse(toParse);
        }
    }
    //vector<vector<int> > emd(depth / 2 + 1);
    vector<vector<pair<int, int> > > res = getEMDDistr(mtr, circulant, upperGirth);
    vector<int> sum(res.size(), 0);
    FILE* out = fopen(OUTPUT_FILENAME.c_str(), "w");
    for (int i = 0; i < res.size(); ++i) {
        for (int j = 0; j < res[i].size(); ++j) {
            res[i][j].second = (res[i][j].second + i - 1) / i * circulant;
            sum[i] += res[i][j].second;
        }
        if (sum[i] == 0)
            continue;
        cerr << "girth =  " << i << "\t" << sum[i] << " cycles" << endl;
        cerr << "EMD\tNumber" << endl;
        for (int j = 0; j < res[i].size(); ++j) {
            cerr << res[i][j].first << "\t" << res[i][j].second << endl;
        }
        fprintf(out, "girth = %d\t%d cycles\n", i, sum[i]);
        fprintf(out, "EMD\tNumber\n");
        for (int j = 0; j < res[i].size(); ++j) {
            fprintf(out, "%d\t%d\n", res[i][j].first, res[i][j].second);
        }
    }
    fclose(out);
    return 0;
}