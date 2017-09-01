# pynq-router

*pynq-router* is a simple 3D-Numberlink puzzle solver.


## Requirements

バージョンは我々が使用しているものなので、もっと古くても動くかもしれない。

* Python 3.6.1
* numpy 1.13.0 (for NLGenerator)
* matplotlib 2.0.2 (for NLGenerator)


## Setups

To setup the Raspberry Pi and PYNQs,
see [wiki](https://github.com/kotarot/pynq-router/wiki) pages.

各 PYNQ でこのリポジトリをクローンしておく。

```
cd /home/xilinx
git clone https://github.com/kotarot/pynq-router.git
```


## Usage

### ランダム問題生成

```
cd NLGenerator
python NLGenerator -x 10 -y 10 -z 4 -l 20 -m 50
```

`Q-10x10x4_20_50.txt` (問題ファイル) と `A-10x10x4_20_50.txt` (解答ファイル) が生成される。

### ソフトウェアバージョン (コンパイル)

```
cd sw
make (or make -f Makefile.cygwin)
```

### ソフトウェアバージョン (実行)

```
./sim
```

でサンプル問題を実行する。

NLGenerator で生成した問題を解くには、次のように `gen_boardstr.py` で1行形式の問題に変換してコマンドライン引数で与える。

```
python gen_boardstr.py ../NLGenerator/Q-10x10x4_20_50.txt
./sim X10Y10Z4L0808101044L0801408012L0301400053L0907200034L0909108061L0107401033L0108103081L0608206071L0300106003L0506105053L0006301051L0101402081L0000300022L0109402094L0908109051L0102101031L0200303033L0802209042L0707405064
```

### PYNQ (FPGA) 上で実行する

まず、Vivado HLS 2016.1 で
`sw/router.cpp` と `sw/router.hpp` をソースファイル、
`sw/main.cpp` をテストベンチとして高位合成して、pynqrouter IP を作成する。

次に、Vivado 2016.1 で pynqrouter IP を接続して論理合成する。
`pynqrouter.tcl` と `pynqrouter.bit` を生成する。

`pynqrouter.tcl` と `pynqrouter.bit` を PYNQ 上の
`/home/xilinx/pynq/bitstream` 内にコピーする。

PYNQ 上で次のコマンドでソルバ実行する。

```
cd /home/xilinx/solver
/opt/python3.6/bin/python3.6 pynqrouter.py -i problem.txt -o output.txt
```

`/opt/python3.6/bin/python3.6 pynqrouter.py -h` で詳細オプションを見よう。

### Raspberry Pi と複数の PYNQ で実行する

#### Raspberry Pi (サーバ) 側

```
/opt/python3.6/bin/python3.6 comm/server/main.py -c servers.txt -q ./problems -o ./answers --debug
```

#### PYNQ (クライアント) 側

```
/opt/python3.6/bin/python3.6 comm/client/main.py --debug
```


## Publications

1. 長谷川健人, 石川遼太, 寺田晃太朗, 川村一志, 多和田雅師, 戸川望, "組込みデバイスとFPGAを用いたナンバーリンクソルバの設計と実装," 情報処理学会DAシンポジウム2017ポスター発表, Aug. 2017.


## Awards

1. 情報処理学会 DAシンポジウム2017 アルゴリズムデザインコンテスト 最優秀賞


## Our Previous Work

* [nl-solver](https://github.com/kotarot/nl-solver)


## License

This software is released under GPL v3 License, see [LICENSE](/LICENSE).
