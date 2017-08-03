# pynq-router

*pynq-router* is a simple 3D-Numberlink puzzle solver.


## Requirements

バージョンは我々が使用しているものなので、もっと古くても動くかもしれない。

* Python 3.6.1
* numpy 1.13.0 (for NLGenerator)
* matplotlib 2.0.2 (for NLGenerator)


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


## Publications

TODO


## License

This software is released under GPL v3 License, see [LICENSE](/LICENSE).
