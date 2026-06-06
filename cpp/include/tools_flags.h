#ifndef _tools_flags_
#define _tools_flags_

// プログラム内部で使用し、外部には出力しないステータスコード
#define CODE_INIT 0            // 各処理の初期値。処理終了時もこの値の場合、管理対象の範囲内でエラーが発生しなかったことを表す
#define CODE_ERR_MAT_NULL -999 // OCRが読み込んだMatが空。dbNetTimeを使用して伝達する

// 外部へ出力するステータスコード

// 認識成功
#define CODE_OK 100      // 正常終了し、文字を認識した
#define CODE_OK_NONE 101 // 正常終了したが、文字を認識しなかった
#define MSG_OK_NONE "画像内に文字が見つかりませんでした。"

// パスからの画像読み込み失敗
#define CODE_ERR_PATH_EXIST 200 // 画像パスが存在しない
#define MSG_ERR_PATH_EXIST(p) "画像パスが存在しません。パス: \"" + p + "\""

#define CODE_ERR_PATH_CONV 201 // 画像パスをstringからwstringへ変換できない
#define MSG_ERR_PATH_CONV(p) "画像パスをUTF-16のwstringへ変換できませんでした。パス: \"" + p + "\""

#define CODE_ERR_PATH_READ 202 // 画像パスは存在するが、ファイルを開けない
#define MSG_ERR_PATH_READ(p) "画像ファイルを開けませんでした。パス: \"" + p + "\""

#define CODE_ERR_PATH_DECODE 203 // 画像ファイルは開けたが、内容をOpenCVでデコードできない
#define MSG_ERR_PATH_DECODE(p) "画像のデコードに失敗しました。パス: \"" + p + "\""

#define CODE_ERR_LOAD_JSON 204 // JSONの読み込みに失敗
#define MSG_ERR_LOAD_JSON(p) "文字列をJSONとして読み込めませんでした。入力値: \"" + p + "\""

#define CODE_ERR_JSON_NO_IMAGE 205 // JSON内に画像の指定が存在しない
#define MSG_ERR_JSON_NO_IMAGE(p) "JSON文字列内に'imagePath'要素がありません。入力値: \"" + p + "\""

// クリップボードからの画像読み込み失敗
#define CODE_ERR_CLIP_OPEN 210 // クリップボードを開けない（OpenClipboard）
#define MSG_ERR_CLIP_OPEN "クリップボードを開けませんでした。"

#define CODE_ERR_CLIP_EMPTY 211 // クリップボードが空（GetPriorityClipboardFormatがNULL）
#define MSG_ERR_CLIP_EMPTY "クリップボードが空です。"

#define CODE_ERR_CLIP_FORMAT 212 // クリップボードの形式に対応していない（GetPriorityClipboardFormatが-1）
#define MSG_ERR_CLIP_FORMAT "クリップボードのデータ形式が無効です。"

#define CODE_ERR_CLIP_DATA 213 // クリップボードのデータハンドルを取得できない。通常は他のプログラムがクリップボードを使用している場合に発生する（GetClipboardDataがNULL）
#define MSG_ERR_CLIP_DATA "クリップボードのデータハンドルを取得できませんでした。"

#define CODE_ERR_CLIP_FILES 214 // クリップボード内のファイル数が1ではない（DragQueryFile != 1）
#define MSG_ERR_CLIP_FILES(n) "クリップボード内のファイル数が不正です。ファイル数: " + std::to_string(n)

#define CODE_ERR_CLIP_GETOBJ 215 // クリップボード内のビットマップオブジェクト情報を取得できない（GetObjectがNULL）
#define MSG_ERR_CLIP_GETOBJ "クリップボード内のビットマップオブジェクトを取得できませんでした。"

#define CODE_ERR_CLIP_BITMAP 216 // クリップボードからビットマップデータを取得できない（GetBitmapBitsでコピーされたバイト数が0）
#define MSG_ERR_CLIP_BITMAP "クリップボードからビットマップデータを取得できませんでした。"

#define CODE_ERR_CLIP_CHANNEL 217 // クリップボード内のビットマップのチャンネル数に対応していない（nChannelsが1、3、4以外）
#define MSG_ERR_CLIP_CHANNEL(n) "クリップボード内の画像のチャンネル数が不正です。チャンネル数: " + std::to_string(n)

// Base64からの画像読み込み失敗
#define CODE_ERR_BASE64_DECODE 300 // Base64文字列のデコードに失敗
#define MSG_ERR_BASE64_DECODE "Base64のデコードに失敗しました。"

#define CODE_ERR_BASE64_IM_DECODE 301 // Base64文字列のデコードには成功したが、内容をOpenCVで画像としてデコードできない
#define MSG_ERR_BASE64_IM_DECODE "Base64データを画像としてデコードできませんでした。"

// 不明なエラー
#define CODE_ERR_UNKNOW 299 // 不明な例外
#define MSG_ERR_UNKNOW "不明なエラーが発生しました。"

#endif
