# pynq-router

*pynq-router* is a simple 3D-Numberlink puzzle solver.


## Requirements

バージョンは我々が使用しているものなので、もっと古くても動くかもしれない。

* Python 3.6.1
* numpy 1.13.0 (for NLGenerator)
* matplotlib 2.0.2 (for NLGenerator)


## Usage

ランダム問題生成

```
cd NLGenerator
python NLGenerator -x 10 -y 10 -z 4 -l 20
```

ソフトウェアバージョン

```
cd sw
make (or make -f Makefile.cygwin)
./sim
```


## Publications

TODO


## License

This software is released under GPL v3 License, see [LICENSE](/LICENSE).
