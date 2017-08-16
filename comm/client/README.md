DAS2017 ADC クライアントプログラム
===

DAS2017 アルゴリズムデザインコンテスト用クライアントプログラム

## Description

問題データをサーバから受信し，結果をサーバへ返すプログラム．

## Requirements

- Python 3.4以上(くらい)
- Flask

## Usage

```
python3 main.py [--port XXXX] [--host XXXX]
```

### Options

<dl>
    <dt>-H, --host</dt>
    <dd>サーバホストのアドレス (デフォルト：192.168.4.1:5000)</dd>
    
    <dt>-p, --port</dt>
    <dd>使用するポート (デフォルト：5000)</dd>
</dl>
