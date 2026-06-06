
#ifndef _tools_
#define _tools_

#include "tools_flags.h" // フラグ
#include "nlohmann_json.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

namespace tool {
    // ==================== パラメータ ====================

    // コールドパラメータ：起動前に設定
    extern bool ensureAscii;              // 出力するJSONをASCIIへ変換するか
    extern bool ensureLogger;             // ログ出力を有効にするか
    extern std::string modelsDir;         // モデル格納ディレクトリ
    extern std::string modelDetPath;      // デフォルトの文字検出モデル
    extern std::string modelClsPath;      // デフォルトの文字方向分類モデル
    extern std::string modelRecPath;      // デフォルトの文字認識モデル
    extern std::string keysPath;          // デフォルトの辞書ファイル
    extern int numThread;                 // スレッド数
    extern int flagGpu;                   // 使用するGPU番号（使用しない場合は-1）

    // ホットパラメータ：起動後に変更可能
    extern int padding;                   // 前処理で追加する余白の幅
    extern int maxSideLen;                // 長辺をこの値まで縮放する
    extern float boxScoreThresh;          // 文字領域の信頼度しきい値
    extern float boxThresh;
    extern float unClipRatio;             // 文字領域の拡大倍率（値が大きいほど領域も大きくなる）
    extern bool doAngle;                   // 文字方向の検出を有効にするか
    extern bool mostAngle;                 // 角度の多数決を有効にするか
    extern bool isEnsureAsci;              // ASCIIへ変換するか

    // ==================== 関数 ====================
    void get_state(int&, std::string&);
    void set_state(int code = CODE_INIT, std::string msg = "");

    void load_congif_file();

    void print_json(const nlohmann::json&);
    void print_ocr_fail(int, const std::string&);
    void print_now_fail();
    std::string load_json_str(std::string&);

    cv::Mat imread_utf8(std::string, int flags = cv::IMREAD_COLOR);
    cv::Mat imread_base64(std::string, int flags = cv::IMREAD_COLOR);
}

#endif
