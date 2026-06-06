#### オフラインOCRコンポーネント関連プロジェクト

- [PaddleOCR-json](https://github.com/hiroi-sora/PaddleOCR-json)
- **RapidOCR-json**

| 項目                         | PaddleOCR-json                                                              | RapidOCR-json                            |
| ---------------------------- | --------------------------------------------------------------------------- | ---------------------------------------- |
| CPU要件                      | CPUがAVX命令セットに対応している必要があります。次のCPUには対応していません | 特別な要件なし 👍                        |
|                              | Atom、Itanium、Celeron、Pentium                                             |                                          |
| 推論高速化ライブラリ         | mkldnn 👍                                                                   | なし                                     |
| 認識速度                     | 高速（mkldnn有効時）👍                                                      | 標準的                                   |
|                              | 非常に低速（mkldnn無効時）                                                  |                                          |
| 初期化時間                   | 約2秒で遅め                                                                 | 0.1秒以内で高速 👍                       |
| コンポーネント容量（圧縮時） | 52MB                                                                        | 15MB 👍                                  |
| コンポーネント容量（展開時） | 250MB                                                                       | 30MB 👍                                  |
| CPU使用率                    | 高く、ハードウェア性能を最大限使用                                          | 比較的低く、低スペック環境にも適している |
| メモリ使用量のピーク         | 2,000MB超（mkldnn有効時）                                                   | 約500MB 👍                               |
|                              | 約600MB（mkldnn無効時）                                                     |                                          |

# RapidOCR-json

これは、[RapidOcrOnnx](https://github.com/RapidAI/RapidOcrOnnx)をベースにした、オフライン画像OCR文字認識プログラムです。パイプなどを通してローカル画像のパスを入力し、認識結果をJSON文字列として出力します。`Windows 7 x64`以降のシステムに対応しています。

本プロジェクトは、パッケージ化されたOCRエンジンコンポーネントを提供することを目的としています。C++のプログラミング知識がない利用者でも、ほかの言語から簡単にOCRを呼び出せます。また、高速な実行性能と、より簡単なパッケージ化・配布方法を利用できます。

![](/readme_images/img-1.png)

## 準備

[RapidOCR-json v0.2.0](https://github.com/hiroi-sora/RapidOCR-json/releases/tag/v0.2.0)をダウンロードして展開するだけで使用できます。

### 簡単な使い方

方法1：

コンソールを開き、次のコマンドを入力します。

```text
path/RapidOCR-json.exe --image_path=path/test1.png
```

方法2：

`RapidOCR_json.exe`を直接ダブルクリックして起動します。プログラムの初期化が完了すると、`OCR init completed.`と出力されます。

JSON文字列で画像パスを入力します。ASCIIエスケープの使用を推奨します。

```json
{ "image_path": "D:/\u6d4b\u8bd5\u56fe\u7247.png" }
```

画像をBase64エンコードした文字列を渡すこともできます。

```json
{ "image_base64": "……" }
```

[Python API](api/python/)を直接使用することもできます。

## コマンドオプション

| キー名         | 説明                                                             | デフォルト値                            |
| -------------- | ---------------------------------------------------------------- | --------------------------------------- |
| ensureAscii    | ASCIIエスケープ出力を有効化（1）または無効化（0）                | 0                                       |
| models         | モデルディレクトリのパス。絶対パスまたは相対パスを指定可能       | `"models"`                              |
| det            | 文字検出モデルのファイル名                                       | `"ch_PP-OCRv3_det_infer.onnx"`          |
| cls            | 文字方向分類モデルのファイル名                                   | `"ch_ppocr_mobile_v2.0_cls_infer.onnx"` |
| rec            | 文字認識モデルのファイル名                                       | `"ch_PP-OCRv3_rec_infer.onnx"`          |
| keys           | 文字認識用辞書のファイル名                                       | `"ppocr_keys_v1.txt"`                   |
| doAngle        | 文字方向検出を有効化（1）または無効化（0）                       | 1                                       |
| mostAngle      | 角度投票を有効化（1）または無効化（0）                           | 1                                       |
| numThread      | スレッド数                                                       | 4                                       |
| padding        | 前処理で追加する白い余白の幅。幅の狭い画像の認識率改善に使用可能 | 50                                      |
| maxSideLen     | 画像の長辺を縮小する上限値。大きな画像の処理速度改善に使用可能   | 1024                                    |
| boxScoreThresh | 文字領域の信頼度しきい値                                         | 0.5                                     |
| boxThresh      | 文字領域検出のしきい値                                           | 0.3                                     |
| unClipRatio    | 1つの文字領域を拡張する倍率                                      | 1.6                                     |
| image_path     | 起動時に認識する画像のパス                                       | `""`                                    |

例1：起動時に画像パスを渡し、1回だけ認識してプログラムを終了します。

```text
RapidOCR_json.exe --image_path="D:/images/test(1).png"
出力：認識結果
```

例2：起動時に画像パスを渡さず、無限ループでJSON入力を受け付けます。

```text
RapidOCR_json.exe --ensureAscii=1
出力：OCR init completed.
{"image_path": "D:/images/test(1).png"}
出力：認識結果
```

例3：各パラメーターを手動で指定します。

```text
RapidOCR_json.exe --doAngle=0 --mostAngle=0 --numThread=12 --padding=100 --image_path="D:/images/test(1).png"
```

### 言語オプション例

**日本語**

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=rec_japan_PP-OCRv3_infer.onnx --keys=dict_japan.txt --image=test.png
```

**簡体字**

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=ch_PP-OCRv3_rec_infer.onnx --keys=dict_chinese.txt --image=test.png
```

**繁体字**

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=rec_chinese_cht_PP-OCRv3_infer.onnx --keys=dict_chinese_cht.txt --image=test.png
```

**韓国語**

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=rec_korean_PP-OCRv3_infer.onnx --keys=dict_korean.txt --image=test.png
```

**英語**

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=rec_en_PP-OCRv3_infer.onnx --keys=dict_chinese.txt --image=test.png
```

## 戻り値

APIからOCRを1回呼び出すたびに、成功・失敗にかかわらずオブジェクトが返されます。

ルートには、状態コードを表す`code`と、内容を表す`data`の2つの要素があります。

`code`は整数で、値ごとに状態が定義されています。

##### `100` 文字を認識した

- `data`は配列です
- 配列の各要素は、次の3つの要素を持つオブジェクトです
    - `text`：認識した文字列
    - `box`：文字領域を囲む四角形の座標。左上、右上、右下、左下の順に、4つの`[x, y]`を格納
    - `score`：認識結果の信頼度を表す浮動小数点数

例：

```json
{
	"code": 100,
	"data": [
		{
			"box": [
				[13, 5],
				[161, 5],
				[161, 27],
				[13, 27]
			],
			"score": 0.9996442794799805,
			"text": "飞舞的因果交流"
		}
	]
}
```

##### `101` 文字を認識できなかった

- `data`には次の形式の文字列が格納されます

```text
No text found in image. Path:"画像パス"
```

例：

```json
{
	"code": 101,
	"data": "No text found in image. Path: \"D:\\空白.png\""
}
```

これは正常な結果です。文字が含まれていない空白画像を認識した場合などに返されます。

##### `200` 画像パスが存在しない

- `data`には次の形式の文字列が格納されます

```text
Image path dose not exist. Path:"画像パス".
```

例：

```json
{
	"code": 200,
	"data": "Image path dose not exist. Path: \"D:\\不存在.png\""
}
```

システムでUTF-8サポートの「ワールドワイド言語サポートでUnicode UTF-8を使用」が有効になっていない場合、`😀.png`のように絵文字などの特殊文字を含むパスは読み込めません。

一般的な中国語やその他のUnicode文字を含むパスは、システムの地域設定や既定の文字コードにかかわらず使用できます。

##### `201` 画像パスの文字列を`wstring`へ変換できなかった

- `data`には次の形式の文字列が格納されます

```text
Image path failed to convert to utf-16 wstring. Path: "画像パス".
```

APIを通常どおり使用している場合、基本的には発生しません。

APIの開発時に、不正な文字コードの文字列を渡した場合に発生する可能性があります。

##### `202` 画像パスは存在するが、ファイルを開けなかった

- `data`には次の形式の文字列が格納されます

```text
Image open failed. Path: "画像パス".
```

システムのアクセス権限などが原因で発生する可能性があります。

##### `203` 画像を開けたが、読み込んだ内容をOpenCVでデコードできなかった

- `data`には次の形式の文字列が格納されます

```text
Image decode failed. Path: "画像パス".
```

このエンジンは、ファイルの拡張子で画像形式を判定しません。指定されたパスからファイルのバイト列を読み込み、画像としてデコードを試みます。

そのため、指定したファイルが画像ではない場合や、画像が破損している場合にこのエラーが返されます。

一方、正常な画像の拡張子を`.png`から`.jpg`や`.exe`などに変更しても、画像としてデコードできれば正常に認識されます。

##### `210` クリップボードを開けなかった

- `data`には次の文字列が格納されます

```text
Clipboard open failed.
```

ほかのプログラムがクリップボードを使用している場合などに発生する可能性があります。

##### `211` クリップボードが空だった

- `data`には次の文字列が格納されます

```text
Clipboard is empty.
```

##### `212` クリップボードの形式に対応していない

- `data`には次の文字列が格納されます

```text
Clipboard format is not valid.
```

エンジンが認識できるクリップボードの内容は、ビットマップ画像またはファイルのみです。コピーしたテキストなど、これら以外の形式ではエラーになります。

##### `213` クリップボードのデータハンドルを取得できなかった

- `data`には次の文字列が格納されます

```text
Getting clipboard data handle failed.
```

ほかのプログラムがクリップボードを使用している場合などに発生する可能性があります。

##### `214` クリップボード内のファイル数が1件ではなかった

- `data`には次の形式の文字列が格納されます

```text
Clipboard number of query files is not valid. Number: ファイル数
```

一度にコピーできるファイルは1件だけです。複数のファイルをコピーした状態でOCRを呼び出すと、このエラーが返されます。

##### `215` クリップボード内のビットマップ情報を取得できなかった

- `data`には次の文字列が格納されます

```text
Clipboard get bitmap object failed.
```

クリップボード内にビットマップは存在しますが、その情報を取得できなかった場合に返されます。ほかのプログラムがクリップボードを使用している場合などに発生する可能性があります。

##### `216` クリップボード内のビットマップデータを取得できなかった

- `data`には次の文字列が格納されます

```text
Getting clipboard bitmap bits failed.
```

クリップボード内のビットマップ情報は取得できたものの、バッファーへの読み込みに失敗した場合に返されます。ほかのプログラムがクリップボードを使用している場合などに発生する可能性があります。

##### `217` クリップボード内のビットマップのチャンネル数に対応していない

- `data`には次の形式の文字列が格納されます

```text
Clipboard number of image channels is not valid. Number: チャンネル数
```

このエンジンが読み込める画像のチャンネル数は、1（白黒）、3（RGB）、4（RGBA）のいずれかです。それ以外のチャンネル数ではエラーになります。

##### `299` 不明な例外

- `data`には次の文字列が格納されます

```text
An unknown error has occurred.
```

通常は発生しない状態コードです。発生した場合はIssueを作成してください。

##### `300` 戻り値をJSON文字列へ変換できなかった

- `data`には次の文字列が格納されます

```text
JSON dump failed. Coding error.
```

起動引数`-image_dir`に、不正な文字コードを含むパスを渡した場合に発生します。

中国語などを含むパスを使用する場合は、起動引数として渡すのではなく、プログラム起動後にJSONで入力してください。

## APIからの呼び出し

### 1. Python API

[リソースディレクトリ](api/python)

使用例：

```python
import os
import sys

from RapidOCR_api import OcrAPI

ocrPath = 'エンジンのパス/RapidOCR_json.exe'
ocr = OcrAPI(ocrPath)
res = ocr.run('サンプル.png')

print('OCR認識結果：\n', res)
ocr.stop()
```

その他のAPIは今後追加予定です。

## [プロジェクトのビルドガイド](cpp)

ソースコードを変更する場合はこちらを参照してください。

## 謝辞

[RapidAI/RapidOcrOnnx](https://github.com/RapidAI/RapidOcrOnnx)に感謝します。本プロジェクトは、このライブラリなしには実現できませんでした。

本プロジェクトでは、[nlohmann/json](https://github.com/nlohmann/json)を使用しています。

> JSON for Modern C++

## 更新履歴

#### v0.2.0 `2023-09-25`

- 画像パスを指定するキーを`imagePath`から`image_path`へ変更
- Base64画像認識機能を追加。キーは`image_base64`

#### v0.1.0 `2023-04-29`
