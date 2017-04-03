#!/bin/sh

data=$1
class=$2
folds=${3:-3}

tsplit -xc$class $data.tab -t$folds 2>/dev/null

rm -rf xval.tmp
for (( i = 0; i < folds; i++ )); do
  list=""
  for (( k = 0; k < folds; k++ )); do
    if (( k != i )); then list="$list $k.tab"; fi
  done
  tmerge $list - 2>/dev/null | \
    bci $data.dom - - 2>/dev/null | \
    bcx - $i.tab 2>&1 | \
    gawk '($2 ~ "error[(]s[)]") {
      printf("%s %s\n", $1, substr($3, 2, length($3)-3)) }' \
    >> xval.tmp
done

gawk '{ cnt += $1; sum += $2; }
END { printf("%d error(s) (%.2f%%)\n", cnt, sum/NR) }' xval.tmp
rm -rf $list $(( folds -1 )).tab xval.tmp
