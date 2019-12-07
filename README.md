# EMD-Spectrum-LDPC
Enumerate LDPC Codes extrinsic message degree (EMD) Spectrum for cycles analysis (indirect Trapping sets analysis with sublinear and  linear sizes)

Papers:

K. Deka, A. Rajesh and P. K. Bora, "On the equivalence of the ACE and the EMD of a cycle for the ACE spectrum constrained LDPC codes," 2014 8th International Symposium on Turbo Codes and Iterative Information Processing (ISTC), Bremen, 2014, pp. 67-71.



Tao Tian, C. R. Jones, J. D. Villasenor and R. D. Wesel, "Selective avoidance of cycles in irregular LDPC code construction," in IEEE Transactions on Communications, vol. 52, no. 8, pp. 1242-1247, Aug. 2004.

Example of EMD Spectrum Estimation from paper Tao at al.

Parity-check Matrix with 9 columns, 3 rows and circulant size 1 (not quasi-cyclic codes)


9 3 1


1	0	1	0	1	0	0	0	1


0	1	1	0	1	1	1	0	0


0	0	0	1	0	1	0	0	0


0	0	0	1	1	0	1	1	0


0	1	1	1	0	1	0	0	1


1	1	0	0	0	0	1	1	1


after enumerating cycles up to 8 using command


getEMDDistribution.exe -file 9_3.txt -out out.txt -upperGirth 8


Get EMD Spectrum

girth = 4	108 cycles
EMD	Number
0	108
girth = 6	504 cycles
EMD	Number
0	504
girth = 8	9234 cycles
EMD	Number
0	9234
