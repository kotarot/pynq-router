# pynq-router (sw)

Vivado HLS 用 pynq-router


## メモ

* Vivado HLS 2016.1
* Vivado 2016.1

高位合成オプション:
* Part: xc7z020clg400-1
* Clock Period: 10.0 ns
* Clock Uncertainty: デフォルト

論理合成のオプション:
* Synthesis strategy: --
* Implementation strategy: --


## 入出力

入力は boardstr という問題ファイルをコンパクトに記述したようなワンラインの文字列。

出力は char 型の配列で、各要素に解答ファイルに相当する数値が入っている。
