# 檔案
LOG=runs.log
CSV=results.csv

# 重新建立檔案
: > "$LOG"
echo "k,run,cycles" > "$CSV"

# 三種 k
for k in 5 10 100; do
  for run in $(seq 1 10); do
    # -------- 1. 執行程式並保留完整輸出 --------
    out=$(./balance "$k")

    # 寫到 log，方便日後檢查
    {
      echo "====== k=$k run=$run ======"
      echo "$out"
      echo
    } >> "$LOG"

    # -------- 2. 從輸出中抓 cycles --------
    cycles=$(printf '%s\n' "$out" | grep -oE 'ran [0-9]+' | awk '{print $2}')

    # -------- 3. 寫入 CSV --------
    echo "$k,$run,$cycles" >> "$CSV"
  done
done

# -------- 4. 在 CSV 末尾計算平均 cycles --------
{
  echo                    # 空行
  echo "k,average_cycles"
  awk -F, 'NR>1 {sum[$1]+=$3; cnt[$1]++}
           END {for (k in sum)
                    printf("%s,%.2f\n", k, sum[k]/cnt[k])}' "$CSV"
} >> "$CSV"

echo "All runs finished."
echo "Raw output   -> $LOG"
echo "Cycle summary -> $CSV"