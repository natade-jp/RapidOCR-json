#ifndef __MAIN_H__
#define __MAIN_H__

#include "getopt.h"

// コマンドライン引数の一覧
static const struct option long_options[] = {
        // コールドパラメーター：起動前に設定
        {"ensureAscii",    required_argument, NULL, 'S'}, // ASCIIエスケープ出力を有効化（1）／無効化（0）
        {"ensureLogger",   required_argument, NULL, 'L'}, // ログおよび可視化出力を有効化（1）／無効化（0）
        {"models",         required_argument, NULL, 'd'}, // モデル格納ディレクトリ。絶対パスまたは相対パスを指定
        {"det",            required_argument, NULL, '1'}, // 文字検出モデルのファイル名
        {"cls",            required_argument, NULL, '2'}, // 文字方向分類モデルのファイル名
        {"rec",            required_argument, NULL, '3'}, // 文字認識モデルのファイル名
        {"keys",           required_argument, NULL, '4'}, // 辞書ファイル名
        {"numThread",      required_argument, NULL, 't'}, // スレッド数
        {"GPU",            required_argument, NULL, 'G'}, // 使用するGPU番号。使用しない場合は-1

        // ホットパラメーター：起動後に変更可能
        {"image_path",     required_argument, NULL, 'i'}, // 初回に認識する画像のパス
        {"padding",        required_argument, NULL, 'p'}, // 画像の周囲へ追加する白い余白の幅
        {"maxSideLen",     required_argument, NULL, 's'}, // 画像の長辺を縮小する際の最大値
        {"boxScoreThresh", required_argument, NULL, 'b'}, // 文字領域の信頼度しきい値
        {"boxThresh",      required_argument, NULL, 'o'}, // 文字領域の検出しきい値
        {"unClipRatio",    required_argument, NULL, 'u'}, // 文字領域の拡大倍率。値が大きいほど領域も大きくなる。画像が大きい場合は、この値も大きくする
        {"doAngle",        required_argument, NULL, 'a'}, // 文字方向検出を有効化（1）／無効化（0）
        {"mostAngle",      required_argument, NULL, 'A'}, // 角度の多数決を有効化（1）／無効化（0）。無効化すると画像全体で最も可能性の高い文字方向を使用しない。文字方向検出が無効の場合、この設定も使用されない
        {"version",        no_argument,       NULL, 'v'}, // バージョン情報を表示
        {"help",           no_argument,       NULL, 'h'}, // ヘルプを表示
        {"loopCount",      required_argument, NULL, 'l'}, // 繰り返し回数
        {NULL,             no_argument,       NULL, 0}
};

const char *optionalMsg =
    "--models: モデル格納ディレクトリ\n"
    "--det: 文字検出モデルのファイル名\n"
    "--cls: 文字方向分類モデルのファイル名\n"
    "--rec: 文字認識モデルのファイル名\n"
    "--keys: 辞書ファイル名\n"
    "--image_path: 指定した場合は、その画像に対してOCRを1回実行します。"
    "省略した場合は、OCRの受付処理を繰り返し実行します。\n"
    "--ensureAscii: ASCIIコードに基づく文字のエスケープ出力を有効化（1）／無効化（0）\n"
    "--ensureLogger: ログ出力および認識結果画像の可視化出力を有効化（1）／無効化（0）\n"
    "--numThread: スレッド数（整数）。デフォルト値：4\n"
    "--padding: 画像の周囲へ追加する白い余白の幅（整数）。デフォルト値：50\n"
    "--maxSideLen: リサイズ後の画像の長辺の最大値（整数）。デフォルト値：1024\n"
    "--boxScoreThresh: 文字領域の信頼度しきい値（浮動小数点数）。デフォルト値：0.5\n"
    "--boxThresh: 文字領域の検出しきい値（浮動小数点数）。デフォルト値：0.3\n"
    "--unClipRatio: 文字領域の拡大倍率（浮動小数点数）。デフォルト値：1.6\n"
    "--doAngle: 文字方向検出を有効化（1）／無効化（0）。デフォルト：有効\n"
    "--mostAngle: 角度の多数決を有効化（1）／無効化（0）。デフォルト：有効\n"
    "--GPU: VulkanによるGPU高速化を使用します。"
    "無効（-1）／GPU 0（0）／GPU 1（1）／...。デフォルト：無効（-1）\n\n";

const char *otherMsg =
    "--version: バージョン情報を表示\n"
    "--help: このヘルプを表示\n\n";

const char *example1Msg =
    "使用例1：%s --image_path=\"D:/images/test(1).png\"\n\n";

const char *example2Msg =
    "使用例2：%s --models=models "
    "--det=ch_PP-OCRv3_det_infer.onnx "
    "--cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
    "--rec=ch_PP-OCRv3_rec_infer.onnx "
    "--keys=ppocr_keys_v1.txt "
    "--ensureAscii=1 "
    "--ensureLogger=1 "
    "--numThread=8 "
    "--padding=50 "
    "--maxSideLen=1024 "
    "--boxScoreThresh=0.5 "
    "--boxThresh=0.3 "
    "--unClipRatio=1.6 "
    "--doAngle=1 "
    "--mostAngle=1 "
    "--GPU=-1\n\n";

#endif // __MAIN_H__
