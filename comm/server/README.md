DAS2017 ADC サーバプログラム
===

DAS2017 アルゴリズムデザインコンテスト用サーバプログラム

## Description

問題データをクライアントへ配信し，結果を受け取るプログラム．

## Requirements

- Python 3.4以上(くらい)
- Flask

## Usage

```
python3 main.py [--question XXXX] [--port XXXX] [--clients XXXX]
```

### Options

<dl>
    <dt>-c, --client</dt>
    <dd>クライアントを定義したテキストファイル．1行ずつホスト名を記述する(必要ならばポート番号も記述する)，必須</dd>
    
    <dt>-q, --question</dt>
    <dd>問題ファイルのパス (デフォルト：./)</dd>

    <dt>-o, --out</dt>
    <dd>回答を出力するパス (デフォルト：./)</dd>

    <dt>-p, --port</dt>
    <dd>サーバのポート (デフォルト：5000)</dd>

    <dt>-l, --line-num-th</dt>
    <dd>処理を分岐させるライン数の閾値</dd>

    <dt>-t, --timeout</dt>
    <dd>PYNQで処理させるタイムアウト(秒)</dd>
</dl>

## Comments

This project uses some libraries: jQuery, Bootstrap
