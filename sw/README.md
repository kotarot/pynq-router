# pynq-router (sw)

Vivado HLS 用 pynq-router


## メモ

* Vivado HLS 2016.1
* Vivado 2016.1

高位合成オプション:
* Part: xc7z020clg400-1
* Clock Period: 10.0 ns
* Clock Uncertainty: 3.0 ns, 3.5 ns // 2017/08/04

論理合成・配置配線のオプション:
* Synthesis strategy: Flow_PerfOptimized_high
* Implementation strategy: Performance_Explore

最適化記録：
* 2017/08/04: クリティカルパス遅延の最適化
  * (3.0) Estimated clock: 6.78, Max latency: 1,667,151,745 (1.00)
  * (3.5) Estimated clock: 6.38, Max latency: 1,692,438,002 (1.00)
* 2017/08/05: キューpop処理の動作改善
  * (3.0) Estimated clock: 6.78, Max latency: 1,530,952,491 (0.92)
  * (3.5) Estimated clock: 6.38, Max latency: 1,556,238,748 (0.92)
* 2017/08/07: 剰余演算の書き換え，隣接ノードの探索ループ展開
  * (3.0) Estimated clock: 6.78, Max latency: 1,423,592,713 (0.85)
  * (3.5) Estimated clock: 6.38, Max latency: 1,428,235,716 (0.84)
* 2017/08/07: 高位合成パラメータの調整 
  * (3.0) Estimated clock: 6.78, Max latency: 1,284,364,290 (0.77)
  * (3.5) Estimated clock: 6.38, Max latency: 1,289,003,291 (0.76)
* 2017/08/11: 隣接ノードの探索ループ展開を戻した (そうしないと論理合成・配置配線でタイミング満たさない)
  * (3.0) Estimated clock: 6.78, Max latency: 1,329,761,290 (0.80)
  * (3.5) Estimated clock: 6.38, Max latency: 1,355,035,291 (0.80)
* 2017/08/11: starts と goals を FF パーティションした
  * (3.0) Estimated clock: 6.78, Max latency: 1,329,245,034 (0.797)
  * (3.5) Estimated clock: 6.45, Max latency: 1,354,519,035 (0.800)
* 2017/08/16: MAX_LINES を 256 に増やした
  * (3.0) Estimated clock: 6.78, Max latency: 1,905,588,970 (1.143)
  * (3.5) Estimated clock: 6.38, Max latency: 1,933,166,972 (1.142)
* 2017/08/16: MAX_LINES を 128 に戻した (そうしないと論理合成・配置配線でタイミング満たさない)
  * (3.0) Estimated clock: 6.78, Max latency: 1,329,245,034 (0.797)
  * (3.5) Estimated clock: 6.45, Max latency: 1,354,519,035 (0.800)
* 2017/08/16: コスト関数の波形をのこぎり型にした
  * (3.0) Estimated clock: 6.78, Max latency: 1,329,245,034 (13.29 sec) (0.797)
  * (3.5) Estimated clock: 6.45, Max latency: 1,354,519,035 (13.55 sec) (0.800)


## 入出力

入力は boardstr という問題ファイルをコンパクトに記述したようなワンラインの文字列。

出力は char 型の配列で、各要素に解答ファイルに相当する数値が入っている。
