// excel_1.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <xlsxwriter.h>

int main() {
    // Excelファイル作成
    lxw_workbook  *workbook  = workbook_new("sample.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    // データ書き込み
    worksheet_write_string(worksheet, 0, 0, "商品名", nullptr);
    worksheet_write_string(worksheet, 0, 1, "価格", nullptr);
    worksheet_write_string(worksheet, 1, 0, "りんご", nullptr);
    worksheet_write_number(worksheet, 1, 1, 120, nullptr);
    worksheet_write_string(worksheet, 2, 0, "ばなな", nullptr);
    worksheet_write_number(worksheet, 2, 1, 80, nullptr);

    worksheet_write_string(worksheet, 3, 0, "tea", nullptr);
    worksheet_write_number(worksheet, 3, 1, 110, nullptr);

    // ファイル保存
    workbook_close(workbook);

    return 0;
}
// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
