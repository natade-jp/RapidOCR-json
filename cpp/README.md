# RapidOCR-json ビルドガイド

このドキュメントでは、Windows x64環境でRapidOCR-jsonをビルドする方法を説明します。

このドキュメントは、RapidAI公式の[ビルド手順](https://github.com/RapidAI/RapidOcrOnnx/blob/main/BUILD.md)を参考にしています。

## 1. 事前準備

各リソースのリンクの後ろにある括弧内には、使用するバージョンを記載しています。間違えないように確認してください。

### 1.1 インストールが必要なツール

- [Visual Studio 2019](https://learn.microsoft.com/zh-cn/visualstudio/releases/2019/release-notes)（Community）
- [CMake](https://cmake.org/download/)（Windows x64 Installer）

## 2. プロジェクトのビルド

1. `lib.7z`を展開し、中にある次の2つのフォルダーを`cpp/`ディレクトリへ配置します。
    - `onnxruntime-static`
    - `opencv-static`
      `cpp/lib/opencv-static`ではなく、`cpp/opencv-static`へ直接配置する点に注意してください。
2. `generate-vs-project.bat`をクリックして実行し、ファイルの生成が完了するまで待ちます。
3. `build-win-vs2019-x64`を開き、Visual Studio 2019で`RapidOcrOnnx`を開きます。
4. Visual Studio 2019上部のツールバーで、構成を`Debug`から`Release`へ変更します。
5. ソリューション エクスプローラーで、`ALL_BUILD`のプロパティを開き、［全般］を設定します。
    - ［出力ディレクトリ］を`$(ProjectDir)/Release`へ変更します
    - ［ターゲット名］を`RapidOCR-json`または任意の名前へ変更します
6. ソリューション エクスプローラーで、`ALL_BUILD`のプロパティを開き、［デバッグ］を設定します。
    - ［作業ディレクトリ］を`$(ProjectDir)/Release`へ変更します
7. ソリューション エクスプローラーで、`ALL_BUILD`のプロパティを開き、［詳細設定］を設定します。
    - ［文字セット］を［Unicode文字セットを使用する］へ変更します
    - ［適用］をクリックし、プロパティ画面を閉じます
8. ソリューション エクスプローラーで、`RapidOcrOnnx`のプロパティを開き、［全般］を設定します。
    - ［ターゲット名］を`RapidOCR-json`または任意の名前へ変更します
9. ソリューション エクスプローラーで、`RapidOcrOnnx`のプロパティを開き、［詳細設定］を設定します。
    - ［文字セット］を［Unicode文字セットを使用する］へ変更します
    - ［適用］をクリックし、プロパティ画面を閉じます
10. F5キーを押してビルドを試します。
    `成功 ○、失敗 0`のように表示されれば、ビルドは成功です。
    黒いウィンドウが一瞬表示されてすぐ閉じる場合も、正常な動作です。
11. プロジェクトのルートディレクトリにある`models.7z`を展開し、`models`フォルダー全体を`Release`ディレクトリへ配置します。
12. 任意のテスト画像を用意し、`test.png`などの名前で`Release`ディレクトリへ配置します。
13. コマンドラインから次のコマンドを実行し、プログラムを起動して動作を確認します。

```text
RapidOCR-json.exe --models=models --det=ch_PP-OCRv3_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=ch_PP-OCRv3_rec_infer.onnx --keys=ppocr_keys_v1.txt --image=test.png
```

## 3. utf-8 オプションについて

C++コンパイラーへ`/utf-8`オプションを指定する必要があります。
ソリューション エクスプローラーで`RapidOcrOnnx`プロジェクトのプロパティを開き、次の場所へ移動します。

```text
構成プロパティ
  → C/C++
    → コマンド ライン
      → 追加のオプション
```

追加のオプションへ、次を指定します。

```text
/utf-8
```

`/utf-8`は、ソースコードと実行時文字列の文字コードをUTF-8として扱うコンパイルオプションです。

一方、［Unicode文字セットを使用する］は、主にWindows APIの`A`版と`W`版の切り替えに使用されます。

```text
Unicode文字セット
    Windows APIでUnicode版の関数を使用するための設定

/utf-8
    ソースコードとchar文字列リテラルをUTF-8として扱うための設定
```

そのため、日本語の`fprintf`や`std::cout`をUTF-8コンソールへ出力する場合は、`/utf-8`の設定が必要です。
