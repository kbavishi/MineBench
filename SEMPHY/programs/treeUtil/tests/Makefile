.PHONY: all clean tests
all: tests

TESTS= shortenTree1 shortenTree2 sametree1 sametree2 sametree3

tests: $(addsuffix .test,$(TESTS))

%.test: %.out.tmp %.out.standard
	diff $^

shortenTree1.out.tmp: ../shortenTree tree1.in
	$^ -m 1 -x 10000 -v 5 -t $(word 2,$^) > $@

shortenTree2.out.tmp: ../shortenTree tree1.in
	$^  -t $(word 2,$^) -o $@

sametree1.out.tmp: ../sametree tree1.in
	$< -t $(word 2,$^) -o  $(word 2,$^) > $@

sametree2.out.tmp: ../sametree tree1.in shortenTree2.out.standard
	$< -t $(word 2,$^) -o  $(word 3,$^) > $@

sametree3.out.tmp: ../sametree tree1.in tree2.in
	$< -t $(word 2,$^) -o  $(word 3,$^) > $@ || true


clean:
	rm -f *.out.tmp
