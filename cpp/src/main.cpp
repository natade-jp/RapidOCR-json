// このコードは https://github.com/RapidAI/RapidOcrOnnx を基にしています
// 二次開発：https://github.com/hiroi-sora

// バージョン情報
#define PROJECT_VER "v0.2.0"
#define PROJECT_NAME "RapidOCR-json " PROJECT_VER

#ifndef __JNI__
#ifndef __CLIB__

#include <cstdio>
#include <iostream>
#include <ctime> // タイムスタンプの記録に使用
#include "main.h"
#include "version.h"
#include "OcrLite.h"
#include "OcrUtils.h"
#include "nlohmann_json.hpp"
#include "tools.h"
#include "tools_flags.h" // ステータスコード

#ifdef _WIN32
#include <windows.h>
#endif

using namespace tool;

OcrLite* OCRLiteP = NULL; // OCRエンジンへのポインター

void printHelp(FILE* out, char* argv0) {
    fprintf(out, " ------- 使用方法 -------\n");
    fprintf(out, "%s\n\n", argv0);
    fprintf(out, " ------- オプションパラメーター -------\n");
    fprintf(out, "%s", optionalMsg);
    fprintf(out, " ------- その他のパラメーター -------\n");
    fprintf(out, "%s", otherMsg);
    fprintf(out, " ------- 使用例 -------\n");
    fprintf(out, example1Msg, argv0);
    fprintf(out, example2Msg, argv0);
}

// ==================== OCR処理を1回実行 ====================
void runOCR(const cv::Mat& mat)
{
    OcrLite& ocrLite = *OCRLiteP; // OCRエンジンオブジェクトを取得

    // OCR処理を1回実行
    OcrResult result = ocrLite.detect(
        mat,
        padding,
        maxSideLen,
        boxScoreThresh,
        boxThresh,
        unClipRatio,
        doAngle,
        mostAngle
    );

    // 結果を出力

    // 3.1. 認識失敗時の出力
    if (result.dbNetTime == CODE_ERR_MAT_NULL) { // エラーステータス
        tool::print_now_fail(); // エラーを出力
        return;
    }

    // 3.2. 認識結果を整理
    nlohmann::json outJ;
    outJ["code"] = 100;
    outJ["data"] = nlohmann::json::array();

    bool isEmpty = true; // 認識結果が空かどうか

    for (const auto& block : result.textBlocks) {
        if (block.text.empty()) { // 認識結果の妥当性を確認
            continue;
        }

        nlohmann::json j;

        // 認識した文字列を設定
        j["text"] = block.text;

        // 認識結果の信頼度を設定
        double score = 0; // 文字ごとの信頼度の平均値
        float charScoreSum = 0.0f;

        for (const auto& s : block.charScores) {
            score += s;
        }

        score /= static_cast<double>(block.charScores.size());
        j["score"] = score;

        // バウンディングボックスを設定
        std::vector<cv::Point> b = block.boxPoint;
        j["box"] = {
            {b[0].x, b[0].y},
            {b[1].x, b[1].y},
            {b[2].x, b[2].y},
            {b[3].x, b[3].y}
        };

        outJ["data"].push_back(j);
        isEmpty = false;
    }

    // 3.3. 認識には成功したが、文字が検出されなかった場合
    if (isEmpty) {
        print_ocr_fail(CODE_OK_NONE, MSG_OK_NONE);
        return;
    }

    // 3.4. 正常な認識結果を出力
    else {
        print_json(outJ);
    }
}

// 指定されたパスの画像を認識
void runPath(std::string imgPath) {
    cv::Mat img = tool::imread_utf8(imgPath, cv::IMREAD_COLOR);

    if (img.empty()) {
        tool::print_now_fail();
    }
    else {
        runOCR(img);
    }
}

// Base64形式の画像を認識
void runBase64(std::string imgBase64) {
    cv::Mat img = tool::imread_base64(imgBase64, cv::IMREAD_COLOR);

    if (img.empty()) {
        tool::print_now_fail();
    }
    else {
        runOCR(img);
    }
}

// ==================== OCRの受付処理を開始 ====================
void startOCR(std::string imgPath = "")
{
    if (!imgPath.empty()) { // 指定された画像に対して1回だけ実行
        runPath(imgPath);
    }
    else { // 標準入力を待ち受け、繰り返し実行
        std::string jsonIn;

        while (1) {
            jsonIn = "";
            getline(std::cin, jsonIn);

            int strLen = jsonIn.length();

            // JSON文字列の場合は内容を解析
            if (
                strLen > 2
                && jsonIn[0] == '{'
                && jsonIn[strLen - 1] == '}'
                ) {
                // JSONを解析し、画像の指定形式を取得
                std::string type = tool::load_json_str(jsonIn);

                if (jsonIn.empty()) { // 画像情報を取得できなかった場合
                    tool::print_now_fail(); // エラーを出力
                    continue;
                }

                if (type == "path") {
                    runPath(jsonIn);
                }
                else if (type == "base64") {
                    runBase64(jsonIn);
                }

                continue;
            }

            tool::print_now_fail(); // エラーを出力
        }
    }
}

int main(int argc, char** argv) {
    // ==================== パラメーターの読み込みと解析 ====================
    // if (argc <= 1) {
    //     printHelp(stderr, argv[0]);
    //     return -1;
    // }

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::string imgPath;
    int opt;
    int optionIndex = 0;

    // ==================== コマンドライン引数を解析 ====================

    // 短い形式のオプションは無効
    // 元の設定："d:1:2:3:4:i:t:p:s:b:o:u:a:A:G:v:h"
    while (
        (opt = getopt_long(
            argc,
            argv,
            "",
            long_options,
            &optionIndex
        )) != -1
        ) {
        switch (opt) {
        case 'd': // モデル格納ディレクトリ
            modelsDir = optarg;
            break;

        case '1': // 文字検出モデルのファイル名
            modelDetPath = optarg;
            break;

        case '2': // 文字方向分類モデルのファイル名
            modelClsPath = optarg;
            break;

        case '3': // 文字認識モデルのファイル名
            modelRecPath = optarg;
            break;

        case '4': // 辞書ファイル名
            keysPath = optarg;
            break;

        case 'i': // 初期表示する画像のパス
            imgPath.assign(optarg);
            // imgDir.assign(
            //     imgPath.substr(0, imgPath.find_last_of('/') + 1)
            // );
            // imgName.assign(
            //     imgPath.substr(imgPath.find_last_of('/') + 1)
            // );
            break;

        case 't': // スレッド数
            numThread = (int)strtol(optarg, NULL, 10);
            break;

        case 'p': // 画像の周囲へ追加する白い余白の幅
            padding = (int)strtol(optarg, NULL, 10);
            break;

        case 's': // 画像の長辺を縮小する際の最大値
            maxSideLen = (int)strtol(optarg, NULL, 10);
            break;

        case 'b': // 文字領域の信頼度しきい値
            boxScoreThresh = strtof(optarg, NULL);
            break;

        case 'o': // 文字領域の検出しきい値
            boxThresh = strtof(optarg, NULL);
            break;

        case 'u': // 文字領域の拡大倍率
            unClipRatio = strtof(optarg, NULL);
            break;

        case 'a': // 文字方向の検出
            doAngle =
                (int)strtol(optarg, NULL, 10) == 0
                ? false
                : true;
            break;

        case 'A': // 文字方向の多数決
            mostAngle =
                (int)strtol(optarg, NULL, 10) == 0
                ? false
                : true;
            break;

        case 'v': // バージョン情報
            return 0;

        case 'h': // ヘルプ
            printHelp(stdout, argv[0]);
            return 0;

        case 'G': // 使用するGPU番号
            flagGpu = (int)strtol(optarg, NULL, 10);
            break;

        case 'L': // ログおよび認識結果画像の出力
            ensureLogger =
                (int)strtol(optarg, NULL, 10) == 0
                ? false
                : true;
            break;

        case 'S': // 非ASCII文字のエスケープ
            ensureAscii =
                (int)strtol(optarg, NULL, 10) == 0
                ? false
                : true;
            break;

        default:
            printf(
                "その他のオプション %c：%s\n",
                opt,
                optarg
            );
        }
    }

    // ==================== モデルファイルの存在確認 ====================

    modelDetPath = modelsDir + "/" + modelDetPath;
    modelClsPath = modelsDir + "/" + modelClsPath;
    modelRecPath = modelsDir + "/" + modelRecPath;
    keysPath = modelsDir + "/" + keysPath;

    bool hasModelDetFile = isFileExists(modelDetPath);

    if (!hasModelDetFile) {
        fprintf(
            stderr,
            "文字検出モデルが見つかりません：%s\n",
            modelDetPath.c_str()
        );
        return -1;
    }

    bool hasModelClsFile = isFileExists(modelClsPath);

    if (!hasModelClsFile) {
        fprintf(
            stderr,
            "文字方向分類モデルが見つかりません：%s\n",
            modelClsPath.c_str()
        );
        return -1;
    }

    bool hasModelRecFile = isFileExists(modelRecPath);

    if (!hasModelRecFile) {
        fprintf(
            stderr,
            "文字認識モデルが見つかりません：%s\n",
            modelRecPath.c_str()
        );
        return -1;
    }

    bool hasKeysFile = isFileExists(keysPath);

    if (!hasKeysFile) {
        fprintf(
            stderr,
            "辞書ファイルが見つかりません：%s\n",
            keysPath.c_str()
        );
        return -1;
    }

    // ==================== OCRエンジンの初期化 ====================

    OcrLite ocrLite;

    // ログ出力を設定
    ocrLite.initLogger(
        ensureLogger, // コンソールへログを出力するか
        false,        // 切り出した画像を出力するか
        ensureLogger  // 認識結果画像を出力するか
    );

    ocrLite.setNumThread(numThread); // スレッド数を設定
    ocrLite.setGpuIndex(flagGpu);    // 使用するGPUを設定

    // OCRモデルを読み込み
    ocrLite.initModels(
        modelDetPath,
        modelClsPath,
        modelRecPath,
        keysPath
    );

    ocrLite.Logger("=====入力パラメーター=====\n");
    ocrLite.Logger(
        "スレッド数(%d)、余白(%d)、長辺の最大値(%d)、"
        "文字領域の信頼度しきい値(%f)、文字領域の検出しきい値(%f)、"
        "文字領域の拡大倍率(%f)、文字方向検出(%d)、"
        "角度の多数決(%d)、GPU(%d)\n",
        numThread,
        padding,
        maxSideLen,
        boxScoreThresh,
        boxThresh,
        unClipRatio,
        doAngle,
        mostAngle,
        flagGpu
    );

    OCRLiteP = &ocrLite;

    std::cout << PROJECT_NAME << std::endl; // バージョン情報を表示
    std::cout << "OCRの初期化が完了しました。" << std::endl;

    // ==================== OCRの受付処理を開始 ====================

    startOCR(imgPath);

    return 0;
}

#endif
#endif
