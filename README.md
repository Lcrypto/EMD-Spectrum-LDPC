# EMD-Spectrum-LDPC
The GitHub repository contains a fast and efficient tool for enumerating Low-Density Parity-Check (LDPC) Codes' Extrinsic Message Degree (EMD) Spectrum for cycle analysis. The EMD spectrum can be used to indirectly analyze trapping sets with sublinear and linear sizes.

This tool allows the user to analyze the effect of EMD on trapping sets under Belief Propagation decoding, and to approximate the waterfall region for linear size trapping sets and error-floor for sublinear size trapping sets.

The tool is based on research published in two papers: "On the equivalence of the ACE and the EMD of a cycle for the ACE spectrum constrained LDPC codes" by K. Deka, A. Rajesh, and P. K. Bora, presented at the 2014 8th International Symposium on Turbo Codes and Iterative Information Processing in Bremen; and "Selective avoidance of cycles in irregular LDPC code construction" by Tao Tian, C. R. Jones, J. D. Villasenor, and R. D. Wesel, published in IEEE Transactions on Communications in 2004.

The repository provides an example of EMD Spectrum Estimation from the paper by Tao et al.

Overall, this repository provides a valuable tool for analyzing the EMD Spectrum of LDPC codes and understanding the impact of EMD on trapping sets under Belief Propagation decoding.



Parity-check Matrix with 9 columns, 3 rows and circulant size 1 (not quasi-cyclic codes) but we can rewrite it in QC for


1	0	1	0	1	0	0	0	1


0	1	1	0	1	1	1	0	0


0	0	0	1	0	1	0	0	0


0	0	0	1	1	0	1	1	0


0	1	1	1	0	1	0	0	1


1	1	0	0	0	0	1	1	1

became

9 6 1


0  -1	 0	-1	0	-1	-1	-1	0

-1  0	 0 -1	0	 0	 0	-1	-1

-1 -1	 -1	  0	-1 0	-1	-1 -1

-1 -1  -1	  0	 0 -1	 0	0	-1

-1	0	  0	  0	-1  0	-1 -1	 0

 0	0	 -1	 -1	 -1	-1 0	0	 0


after enumerating cycles up to 8 using command


getEMDDistribution.exe -file 9_3.txt -out out.txt -upperGirth 8

result at out.txt


girth = 4	11 cycles


EMD	Number


1	2


2	9


girth = 6	51 cycles


EMD	Number


0	8


1	19


2	18


3	6


girth = 8	170 cycles


EMD	Number


0	50


1	57


2	54


3	9
