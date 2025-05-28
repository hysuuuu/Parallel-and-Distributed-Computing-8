LOG=runs.log
CSV=results.csv

: > "$LOG"
echo "k,run,time,balanced" > "$CSV"

for k in 5 10 100; do
  for run in $(seq 1 10); do
    out=$(./balance "$k")

    {
      echo "====== k=$k run=$run ======"
      echo "$out"
      echo
    } >> "$LOG"

    time=$(printf '%s\n' "$out" | grep -oP 'Finished at time: \K\d+')
    balanced=$(printf '%s\n' "$out" | grep 'System Balanced' | grep -oE '(YES|NO)')

    echo "$k,$run,$time,$balanced" >> "$CSV"

  done
done

{
  echo                   
  echo "k,average_time"
  awk -F, 'NR>1 {sum[$1]+=$3; cnt[$1]++}
           END {for (k in sum)
                    printf("%s,%.2f\n", k, sum[k]/cnt[k])}' "$CSV"
} >> "$CSV"

echo "All runs finished."
echo "Raw output   -> $LOG"
echo "Time summary -> $CSV"
