# DAS2017 ADC RaspberryPi・PYNQ間HTTP通信プログラム

# 概要

DAS2017アルゴリズムデザインコンテストに向けた，端末間通信プラグラム．
親となるRaspberry Piから，子となる複数のPYNQに対し問題を配信し，結果を受け取る．

# 構成

+ server: Raspberry Pi上で実行するためのサーバプラグラム．問題をクライアントに配信し，結果を受け取る．
+ client: PYNQ上で実行するためのクライアントプログラム．問題をサーバから受け取り，問題を解いて回答をサーバに返す．
+ README.md: このファイル．

# Requirements

- Python 3.4以上
- Flask (pipからインストール)
