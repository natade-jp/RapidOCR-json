#include <string>
#include <iostream>
#include <fstream> // ファイルの読み込み
#include <windows.h> // クリップボード関連
#include <filesystem> // 設定ファイルの存在確認

#include "tools_flags.h" // ステータスコード
#include "tools.h"
#include "opencv2/imgproc.hpp" // 色空間の変換
#include "base64.h" // Base64ライブラリ

// 文字コード変換
#include <codecvt>

// UTF-8のstringとUTF-16のwstringを相互変換するコンバーター
std::wstring_convert<std::codecvt_utf8<wchar_t>> conv_Ustr_Wstr;

using namespace std;
using namespace nlohmann;

namespace tool {
    // ==================== OCRエンジンのパラメーター ====================

    // コールドパラメーター：起動前に設定
    bool ensureAscii = false; // 出力するJSONをASCIIへ変換するか
    bool ensureLogger = false; // ログ出力を有効にするか
    string modelsDir = "models"; // モデル格納ディレクトリ
    string modelDetPath = "ch_PP-OCRv3_det_infer.onnx"; // デフォルトの文字検出モデル
    string modelClsPath = "ch_ppocr_mobile_v2.0_cls_infer.onnx"; // デフォルトの文字方向分類モデル
    string modelRecPath = "ch_PP-OCRv3_rec_infer.onnx"; // デフォルトの文字認識モデル
    string keysPath = "ppocr_keys_v1.txt"; // デフォルトの辞書ファイル
    int numThread = 4; // スレッド数
    int flagGpu = -1; // 使用するGPU番号（使用しない場合は-1）

    // ホットパラメーター：起動後に変更可能
    int padding = 50; // 前処理で追加する白い余白の幅
    int maxSideLen = 1024; // 長辺をこの値まで縮小
    float boxScoreThresh = 0.5f; // 文字領域の信頼度しきい値
    float boxThresh = 0.3f;
    float unClipRatio = 1.6f; // 文字領域の拡大倍率。値が大きいほど領域も大きくなる
    bool doAngle = true; // 文字方向の検出を有効にするか
    bool mostAngle = true; // 角度の多数決を有効にするか

    // ==================== 実行状態 ====================

    int ToolCode = 0; // 現在の処理におけるエラーコード
    string ToolMsg = ""; // 現在の処理におけるエラーメッセージ

    // 現在の状態を取得
    void get_state(int& code, string& msg) {
        code = ToolCode;
        msg = ToolMsg;
    }

    // 現在の状態を設定
    void set_state(int code, string msg) {
        ToolCode = code;
        ToolMsg = msg;
    }

    // メッセージ用のwstringをstringへ変換
    // 変換に失敗した場合はデフォルトのエラーメッセージを返す
    string msg_wstr_2_ustr(wstring& msg) {
        try {
            string msgU8 = conv_Ustr_Wstr.to_bytes(msg); // UTF-8へ変換
            return msgU8;
        }
        catch (...) {
            return "wstringからUTF-8のstringへの変換に失敗しました。";
        }
    }

    // マルチバイトANSI文字列をワイド文字列へ変換
    wchar_t* char_2_wchar(char* c) {
        setlocale(LC_ALL, ""); // ロケールをWindowsシステムの現在の設定へ変更
        size_t lenWchar = mbstowcs(NULL, c, 0); // ワイド文字列へ変換した場合の長さを取得
        wchar_t* wc = new wchar_t[lenWchar + 1]; // ファイル名のワイド文字列を格納
        int n = mbstowcs(wc, c, lenWchar + 1); // マルチバイト文字列をワイド文字列へ変換
        setlocale(LC_ALL, "C"); // ロケールをデフォルトへ戻す
        return wc;
    }

    // ==================== JSON関連 ====================

    // JSONを出力
    void print_json(const json& j) {
        try {
            cout << j.dump(-1, ' ', ensureAscii) << endl;
        }
        catch (...) {
            json j2;
            j2["code"] = 300;
            j2["data"] = "JSONの出力に失敗しました。文字コードに問題があります。";
            cout << j2.dump(-1, ' ', ensureAscii) << endl;
        }
    }

    // エラー情報を出力
    void print_ocr_fail(int code, const string& msg) {
        json j;
        j["code"] = code;
        j["data"] = msg;
        print_json(j);
    }

    // 現在のエラー情報を出力
    void print_now_fail()
    {
        int code;
        std::string msg;

        get_state(code, msg);

        if (code != CODE_INIT) { // エラーが登録されている場合
            print_ocr_fail(code, msg); // JSON形式で出力
            return;
        }

        print_ocr_fail(CODE_ERR_UNKNOW, MSG_ERR_UNKNOW); // 不明なエラー
    }

    // 入力例：{"image_path":"D:\Test\Test.png"}
    // JSON文字列を読み込み、str_inを画像パスまたはBase64文字列へ書き換える
    // 戻り値として画像の指定形式を返す
    std::string load_json_str(string& str_in) {
        set_state(); // 状態を初期化

        string origin_str = str_in;

        try {
            auto j = json::parse(str_in); // JSONオブジェクトへ変換

            for (auto& el : j.items()) { // キーと値を順番に確認
                string value = to_string(el.value());
                int vallen = value.length();

                if (
                    vallen > 2
                    && value[0] == '\"'
                    && value[vallen - 1] == '\"'
                    ) {
                    // nlohmann::jsonによって付与された両端の引用符を削除
                    value = value.substr(1, vallen - 2);
                }

                if (el.key() == "image_path") { // 画像パス
                    str_in = value;
                    return "path";
                }
                else if (el.key() == "image_base64") { // Base64形式の画像
                    str_in = value;
                    return "base64";
                }

                // その他のパラメーターは現在未対応
            }

            // 状態を設定：画像の指定が見つからない
            set_state(
                CODE_ERR_JSON_NO_IMAGE,
                MSG_ERR_JSON_NO_IMAGE(origin_str)
            );

            str_in = "";
        }
        catch (...) {
            // 状態を設定：JSONの解析に失敗
            set_state(
                CODE_ERR_LOAD_JSON,
                MSG_ERR_LOAD_JSON(origin_str)
            );

            str_in = "";
        }

        return "";
    }

    // ==================== 画像の読み込み ====================

    // pathWがファイルとして存在する場合はtrueを返す
    bool is_exists_wstr(wstring pathW) {
        struct _stat buf;
        int result = _wstat((wchar_t*)pathW.c_str(), &buf);

        if (result != 0) { // エラーが発生
            return false;
        }

        if (S_IFREG & buf.st_mode) { // ファイルの場合
            return true;
        }

        // else if (S_IFDIR & buf.st_mode) { // ディレクトリの場合
        //     return false;
        // }

        return false;
    }

    // cv::imreadの代替処理
    // Unicode形式のwstringで指定されたパスから画像を読み込む
    cv::Mat imread_wstr(
        wstring pathW,
        int flags = cv::IMREAD_COLOR
    ) {
        // エラー出力で使用するため、UTF-8のstringへ戻す
        string pathU8 = msg_wstr_2_ustr(pathW);

        // この関数はクリップボードのCF_UNICODETEXTなどからも使用されるため、
        // 呼び出し元がwstringしか渡せない場合を考慮し、ここで再変換している

        if (!is_exists_wstr(pathW)) { // パスが存在しない
            // 状態を設定：画像パスが存在しない
            set_state(
                CODE_ERR_PATH_EXIST,
                MSG_ERR_PATH_EXIST(pathU8)
            );

            return cv::Mat();
        }

        // ワイド文字列のパスを使用してファイルを開く
        FILE* fp = _wfopen(
            (wchar_t*)pathW.c_str(),
            L"rb"
        );

        if (!fp) { // ファイルを開けなかった場合
            // 状態を設定：画像ファイルを読み込めない
            set_state(
                CODE_ERR_PATH_READ,
                MSG_ERR_PATH_READ(pathU8)
            );

            return cv::Mat();
        }

        // ファイルの内容をメモリへ読み込む
        fseek(fp, 0, SEEK_END); // ファイル位置を末尾へ移動
        long sz = ftell(fp); // ファイル全体のサイズを取得
        char* buf = new char[sz]; // ファイルの内容を格納する領域
        fseek(fp, 0, SEEK_SET); // ファイル位置を先頭へ戻す

        // ファイルの内容をbufへ読み込む
        long n = fread(buf, 1, sz, fp);

        // OpenCVの入力配列へ変換
        cv::_InputArray arr(buf, sz);

        // メモリ上の画像データをcv::Matへデコード
        cv::Mat img = cv::imdecode(arr, flags);

        delete[] buf; // bufのメモリを解放
        fclose(fp); // ファイルを閉じる

        if (!img.data) {
            // 状態を設定：画像のデコードに失敗
            set_state(
                CODE_ERR_PATH_DECODE,
                MSG_ERR_PATH_DECODE(pathU8)
            );
        }

        return img;
    }

    // クリップボードから画像を読み込む
    cv::Mat imread_clipboard(
        int flags = cv::IMREAD_COLOR
    ) {
        // 参考：
        // https://docs.microsoft.com/zh-cn/windows/win32/dataxchg/using-the-clipboard

        // クリップボードを開き、他のアプリケーションによる変更を防止
        if (!OpenClipboard(NULL)) {
            // 状態を設定：クリップボードを開けない
            set_state(
                CODE_ERR_CLIP_OPEN,
                MSG_ERR_CLIP_OPEN
            );
        }
        else {
            // 読み込みを許可するクリップボード形式
            static UINT auPriorityList[] = {
                CF_BITMAP, // ビットマップ
                CF_HDROP   // ファイル一覧のハンドル
            };

            // 対応形式の数
            int auPriorityLen =
                sizeof(auPriorityList)
                / sizeof(auPriorityList[0]);

            // 現在のクリップボード内容に対応する形式を取得
            int uFormat = GetPriorityClipboardFormat(
                auPriorityList,
                auPriorityLen
            );

            // 形式に応じて処理を分岐
            //
            // 処理に成功した場合：
            //   使用したリソースを解放してクリップボードを閉じ、
            //   画像のcv::Matを返す
            //
            // 処理に失敗した場合：
            //   開いているリソースやロックを解放し、
            //   エラー状態を設定してswitchを終了する
            //
            // switch終了後にクリップボードを閉じ、
            // 空のcv::Matを返す

            switch (uFormat)
            {
            case CF_BITMAP: {
                // 1. ビットマップ形式
                // ==================================================

                // 1.1. クリップボードからビットマップのハンドルを取得
                HBITMAP hbm =
                    (HBITMAP)GetClipboardData(uFormat);

                if (hbm) {
                    // GlobalLock(hbm);
                    //
                    // ビットマップの読み込みではロックが不要らしく、
                    // GlobalLockの戻り値は常に無効になる
                    //
                    // 参考：
                    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/d2a6aa71-68d7-4db0-8b1f-5d1920f9c4ce/globallock-and-dib-transform-into-hbitmap-issue?forum=vcgeneral

                    // 指定したグラフィックオブジェクトの情報を格納
                    BITMAP bmp;

                    // 1.2. グラフィックオブジェクトの情報を取得
                    // 画像データそのものは含まれない
                    GetObject(
                        hbm,
                        sizeof(BITMAP),
                        &bmp
                    );

                    if (!hbm) {
                        // 状態を設定：グラフィックオブジェクトの取得に失敗
                        set_state(
                            CODE_ERR_CLIP_GETOBJ,
                            MSG_ERR_CLIP_GETOBJ
                        );

                        break;
                    }

                    // 色深度からチャンネル数を計算
                    // 32bitの場合は4、24bitの場合は3
                    int nChannels =
                        bmp.bmBitsPixel == 1
                        ? 1
                        : bmp.bmBitsPixel / 8;

                    // 1.3. ビットマップの内容をバッファへコピー

                    // 画像全体のサイズを計算
                    long sz =
                        bmp.bmHeight
                        * bmp.bmWidth
                        * nChannels;

                    // ビットマップの大きさとチャンネル数を指定し、
                    // 空のcv::Matを作成
                    cv::Mat mat(
                        cv::Size(
                            bmp.bmWidth,
                            bmp.bmHeight
                        ),
                        CV_MAKETYPE(CV_8U, nChannels)
                    );

                    // ビットマップハンドルから画像データをコピー
                    long getsz = GetBitmapBits(
                        hbm,
                        sz,
                        mat.data
                    );

                    if (!getsz) {
                        // 状態を設定：ビットマップデータの取得に失敗
                        set_state(
                            CODE_ERR_CLIP_BITMAP,
                            MSG_ERR_CLIP_BITMAP
                        );

                        break;
                    }

                    CloseClipboard(); // クリップボードを閉じる

                    // 1.4. 適切なチャンネル数の画像を返す
                    if (mat.data) {
                        if (
                            nChannels == 1
                            || nChannels == 3
                            ) {
                            // 1チャンネルまたは3チャンネルは
                            // PPOCRで認識可能なため、そのまま返す
                            return mat;
                        }
                        else if (nChannels == 4) {
                            // 4チャンネルはPPOCRで認識できないため、
                            // アルファチャンネルを削除し、
                            // 3チャンネルへ変換して返す
                            cv::Mat mat_c3;

                            cv::cvtColor(
                                mat,
                                mat_c3,
                                cv::COLOR_BGRA2BGR
                            );

                            return mat_c3;
                        }

                        // 状態を設定：画像のチャンネル数が不正
                        set_state(
                            CODE_ERR_CLIP_CHANNEL,
                            MSG_ERR_CLIP_CHANNEL(nChannels)
                        );

                        break;
                    }

                    // 通常は上の!getszで処理が終了するため、
                    // ここには到達しないが、念のためエラーを設定する
                    set_state(
                        CODE_ERR_CLIP_BITMAP,
                        MSG_ERR_CLIP_BITMAP
                    );

                    break;
                }

                // 状態を設定：クリップボードのデータ取得に失敗
                set_state(
                    CODE_ERR_CLIP_DATA,
                    MSG_ERR_CLIP_DATA
                );

                break;
            }

            case CF_HDROP: {
                // 2. ファイル一覧形式
                // ==================================================

                // 2.1. ファイル一覧のハンドルを取得
                HDROP hClip =
                    (HDROP)GetClipboardData(uFormat);

                if (hClip) {
                    // 参考：
                    // https://docs.microsoft.com/zh-CN/windows/win32/api/shellapi/nf-shellapi-dragqueryfilea

                    // 2.2. グローバルメモリオブジェクトをロック
                    GlobalLock(hClip);

                    // 2.3. クリップボード内のファイル数を取得
                    int iFiles = DragQueryFile(
                        hClip,
                        0xFFFFFFFF,
                        NULL,
                        0
                    );

                    if (iFiles != 1) { // ファイルは1つのみ許可
                        GlobalUnlock(hClip);

                        // 状態を設定：ファイル数が1ではない
                        set_state(
                            CODE_ERR_CLIP_FILES,
                            MSG_ERR_CLIP_FILES(iFiles)
                        );

                        break;
                    }

                    // for (int i = 0; i < iFiles; i++) {

                    int i = 0; // 先頭のファイルのみ取得

                    // 2.4. ファイル名の読み込みに必要なサイズを取得
                    UINT lenChar = DragQueryFile(
                        hClip,
                        i,
                        NULL,
                        0
                    );

                    // ファイル名を格納する領域
                    char* nameC =
                        new char[lenChar + 1];

                    // 2.5. ファイル名を取得
                    DragQueryFileA(
                        hClip,
                        i,
                        nameC,
                        lenChar + 1
                    );

                    // 2.6. ファイル名をワイド文字列へ変換
                    wchar_t* nameW =
                        char_2_wchar(nameC);

                    // 2.7. 画像ファイルを読み込む
                    cv::Mat mat =
                        imread_wstr(nameW);

                    // 使用したリソースを解放
                    delete[] nameC;
                    delete[] nameW;

                    // 2.x.1. ファイル一覧のハンドルを解放
                    GlobalUnlock(hClip);

                    // 2.x.2. クリップボードを閉じる
                    CloseClipboard();

                    return mat;
                }

                // 状態を設定：クリップボードのデータ取得に失敗
                set_state(
                    CODE_ERR_CLIP_DATA,
                    MSG_ERR_CLIP_DATA
                );

                break;
            }

            case NULL:
                // クリップボードが空
                set_state(
                    CODE_ERR_CLIP_EMPTY,
                    MSG_ERR_CLIP_EMPTY
                );

                break;

            case -1:
                // 対応していない形式

            default:
                // 不明な形式
                set_state(
                    CODE_ERR_CLIP_FORMAT,
                    MSG_ERR_CLIP_FORMAT
                );

                break;
            }

            // breakでswitchを終了した場合は、
            // 他のアプリケーションが再びアクセスできるよう、
            // ここでクリップボードを閉じる
            CloseClipboard();
        }

        return cv::Mat();
    }

    // cv::imreadの代替処理
    // UTF-8形式のstringで指定されたパスから画像を読み込む
    cv::Mat imread_utf8(
        string pathU8,
        int flags
    ) {
        set_state(); // 状態を初期化

        if (pathU8 == u8"clipboard") {
            // クリップボードから画像を読み込む
            return imread_clipboard(flags);
        }

        // UTF-8のstringをwstringへ変換
        std::wstring wpath;

        try {
            // コンバーターを使用して変換
            wpath = conv_Ustr_Wstr.from_bytes(pathU8);
        }
        catch (...) {
            // 状態を設定：wstringへの変換に失敗
            set_state(
                CODE_ERR_PATH_CONV,
                MSG_ERR_PATH_CONV(pathU8)
            );

            return cv::Mat();
        }

        return imread_wstr(wpath);
    }

    // Base64形式の文字列を入力し、cv::Matを返す
    cv::Mat imread_base64(
        string b64str,
        int flag
    ) {
        std::string decoded_string;

        try {
            decoded_string =
                base64_decode(b64str);
        }
        catch (...) {
            // 状態を設定：Base64の解析に失敗
            set_state(
                CODE_ERR_BASE64_DECODE,
                MSG_ERR_BASE64_DECODE
            );

            return cv::Mat();
        }

        try {
            // デコード後の文字列をバイト配列へ変換
            std::vector<uchar> data(
                decoded_string.begin(),
                decoded_string.end()
            );

            // バイト配列を画像へ変換
            cv::Mat img =
                cv::imdecode(data, flag);

            if (img.empty()) {
                // 状態を設定：cv::Matへの変換に失敗
                set_state(
                    CODE_ERR_BASE64_IM_DECODE,
                    MSG_ERR_BASE64_IM_DECODE
                );
            }

            return img;
        }
        catch (...) {
            // 状態を設定：cv::Matへの変換に失敗
            set_state(
                CODE_ERR_BASE64_IM_DECODE,
                MSG_ERR_BASE64_IM_DECODE
            );

            return cv::Mat();
        }
    }
}

