#! /bin/tcsh -f

echo "Iterative NJ, posterior-distance, 4 seqs"
../distanceBasedSeqs2TreeSA -s 1blsA.a1.0.croped4.chopped500.seq.fasta -o 1blsA.a1.0.croped4.chopped500.seq.iterativeNJ.posterior.out -T 1blsA.a1.0.croped4.chopped500.seq.iterativeNJ.posterior.tree -O -l 1blsA.a1.0.croped4.chopped500.seq.iterativeNJ.posterior.log -v 6 -E 0.01 --epsilonLikelihoodImprovement4alphaOptimiz 0.03 --epsilonLikelihoodImprovement4BBL 0.03 --posterior

cat 1blsA.a1.0.croped4.chopped500.seq.iterativeNJ.posterior.out 1blsA.a1.0.croped4.chopped500.seq.iterativeNJ.posterior.log 
