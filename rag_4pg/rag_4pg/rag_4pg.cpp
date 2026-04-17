#include "httplib.h"
#include <cmath>
#include <fcntl.h>       // _O_U16TEXT
#include <fstream>
#include <filesystem>
#include <iostream>
#include <io.h>          // _setmode
#include <iomanip>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp> // JSONライブラリ
#include <vector>
#include <random>
#include <shellapi.h>    // CommandLineToArgvW
#include <string>
#include <sstream>
#include <stdexcept>
#include <windows.h>

#pragma comment(lib, "shell32.lib")

#include "pgvector_client.h"
#include "HttpClient.h"

using json = nlohmann::json;


const std::string DB_HOST = "localhost";
const std::string DB_NAME = "mydb";
const std::string DB_USER = "root";
const std::string DB_PASSWORD = "admin";
// ─────────────────────────────────────────
// データ構造
// ─────────────────────────────────────────
struct Todo {
    int         id;
    std::string title;
    bool        done;
};

// インメモリストレージ
static std::vector<Todo> g_todos;
static int               g_next_id = 1;
static std::mutex        g_mutex;


struct QueryReq {
    std::string input;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryReq, input)

struct SearchReq {
    std::wstring input;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchReq, input)


// ファイルを文字列として読み込むユーティリティ
std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return "";
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}
//
std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), (int)str.size(),
        NULL, 0
    );

    std::wstring wstr(size_needed, 0);

    MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), (int)str.size(),
        &wstr[0], size_needed
    );

    return wstr;
}
// wstring ↔ string 変換ヘルパー (Windows ANSI 限定)
static std::string WStrToStr(const std::wstring& ws)
{
    if (ws.empty()) return {};
    int n = ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

// std::wstring を UTF-8 の std::string に変換するヘルパー
std::string to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// ─────────────────────────────────────────
// ヘルパー：Todo → JSON 文字列
// ─────────────────────────────────────────
std::string todo_to_json(const Todo& t) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << t.id << ","
        << "\"title\":\"" << t.title << "\","
        << "\"done\":" << (t.done ? "true" : "false")
        << "}";
    return oss.str();
}

std::string todos_to_json(const std::vector<Todo>& todos) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < todos.size(); ++i) {
        if (i > 0) oss << ",";
        oss << todo_to_json(todos[i]);
    }
    oss << "]";
    return oss.str();
}

// ─────────────────────────────────────────
// ヘルパー：JSON から値を取り出す（簡易版）
// ─────────────────────────────────────────
std::string extract_string(const std::string& json, const std::string& key) {
    // "key":"value" を探す
    std::string pattern = "\"" + key + "\":\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

bool extract_bool(const std::string& json, const std::string& key, bool def = false) {
    std::string pattern = "\"" + key + "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return def;
    pos += pattern.size();
    return json.substr(pos, 4) == "true";
}

std::string getStringResult(const std::vector<SearchResult>& results,
                  const std::string& title)
{
    std::string ret = "";
    int rank = 1;
    std::string matches = "";
    for (const auto& r : results) {
        std::cout << "r.id=" << r.id << "\n";
        std::cout << "r.distance=" << r.distance << "\n";
        if (r.distance < 0.5) {
            matches = r.label;
        }        
    }
    ret = matches;
    return ret;
}


struct SearchResponse {
    std::string ret;
    std::string text;
};   
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchResponse, ret, text)


struct ChatQuery {
    std::string role;
    std::string content;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChatQuery, role, content)

struct ChatRequest {
    std::string model;
    std::vector<ChatQuery> messages;
    double temperature;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChatRequest, model, messages, temperature)

const std::wstring API_URL_CHAT = L"http://localhost:8090/v1/chat/completions";

std::string extractContent(const std::string& jsonStr)
{
    try {
        auto j = nlohmann::json::parse(jsonStr);
        return j["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] JSON parse: " << e.what() << "\n";
        return "";
    }
}
//
std::string send_chat(std::string query) {
    std::string ret = "";
    ChatQuery req2;
    req2.role = "user";
    req2.content = query;
    json j2 = req2;
    std::string json_str2 = j2.dump();
    std::wstring w_str2 = StringToWString(json_str2);
    //std::wcout << L"json_str2:" << w_str2 << std::endl;
    std::vector<ChatQuery> chat_messages;
    chat_messages.push_back(req2);

    std::string target_msg = "[";
    target_msg.append(json_str2);
    target_msg.append("]");
    ChatRequest req3;
    req3.model = "local-model";
    req3.messages = chat_messages;
    req3.temperature = 0.7;
    json j3 = req3; // 構造体を代入するだけ！
    std::string json_str3 = j3.dump();
    std::wstring w_str3 = StringToWString(json_str3);
    //std::wcout << L"json_str3:" << w_str3 << std::endl;
    HttpClient client;

    auto resp2 = client.Post(
        API_URL_CHAT,
        json_str3,
        L"application/json");

    if (resp2.statusCode == 200) {
        std::wcout << L"resp.statusCode=200 \n\n";
        std::string reply = extractContent(resp2.body);
        //std::wstring w_str4 = StringToWString(reply);
        ret = reply;
        //std::wcout << L"Assistant: " << w_str4 << std::endl; 
        //std::wcout << L"\n" << std::endl;
    }
    return ret;
}

/**
*
* @param
*
* @return
*/
std::string rag_search(std::string query) {
    try {
        std::string ret = "";
        QueryReq req_data;
        req_data.input = query;

        // 2. JSON オブジェクトの作成
        json j1 = req_data;
        //j1["input"] = to_utf8(req_data.input); // UTF-8に変換して格納
        std::string json_str = j1.dump();
        std::wstring w_str = StringToWString(json_str);
        std::wcout << L"w_str : " << w_str << L"\n";
        HttpClient client;

        auto resp = client.Post(
            L"http://localhost:8080/embedding",
            json_str,
            L"application/json");

        if (resp.statusCode == 200) {
            std::wcout << L"resp.statusCode=200 \n";
            std::string str = resp.body;
            json j = json::parse(str);
            auto embedding = j[0]["embedding"];
            auto vec = embedding[0];
            int vlength = sizeof(vec) / sizeof(vec[0]);
            std::wcout << L"vlen=" << vec.size() << L"\n";

            //
            PGConnConfig cfg;
            cfg.host = DB_HOST;
            cfg.port = 5432;
            cfg.dbname = DB_NAME;
            cfg.user = DB_USER;
            cfg.password = DB_PASSWORD;

            PGVectorClient client(cfg);
            // =====================================================
            //  1. 接続
            // =====================================================
            client.connect();
            auto resultsCos   = client.searchCosine(vec, 1);
            std::string out =  getStringResult(resultsCos, "Cosine-similar");
            //std::wcout << L"out=" << StringToWString(out) << L"\n";

            std::string out_str = "日本語で、回答して欲しい。 \n要約して欲しい。\n\n";
            std::string resp_str = out;
            if(resp_str.empty()){
                out_str.append("user query: ");
                out_str.append(query);
                out_str.append(" \n");
            }else{
                out_str.append("context:");
                out_str.append(resp_str);
                out_str.append("\n user query: ");
                out_str.append(query);
                out_str.append(" \n");
            }
            //ret = out_str;
            std::wcout << StringToWString(out_str)  << std::endl;
            ret = send_chat(out_str);
        }        
        return ret;
    }
    catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        std::cerr << "Error , rag_search" << std::endl;
    }
}

// ─────────────────────────────────────────
// CORS ヘッダー（ブラウザからのアクセス用）
// ─────────────────────────────────────────
void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// ─────────────────────────────────────────
// main
// ─────────────────────────────────────────
int main() {
    httplib::Server svr;

    // ── OPTIONS（プリフライト） ──────────────
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 204;
        });

    // ── GET /todos ── 一覧取得 ───────────────
    svr.Get("/todos", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        set_cors(res);
        res.set_content(todos_to_json(g_todos), "application/json");
        });

    // ── POST /todos ── 新規作成 ──────────────
    svr.Post("/todos", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
       // 1. Content-Typeの確認
        if (req.get_header_value("Content-Type") != "application/json") {
            res.status = 400;
            res.set_content("Expected application/json", "text/plain");
            return;
        }
        try {
            // 2. JSONデコード (req.body をパース)
            json j = json::parse(req.body);

            // 3. データの取り出し (例: {"name": "Gopher", "id": 123})
            std::string title = j.at("title").get<std::string>();
            std::cout << "title=" << title << "\n";

            Todo t;
            t.id = g_next_id++;
            t.title = title;
            t.done = false;
            g_todos.push_back(t);
            res.status = 200;
            res.set_content(todo_to_json(t), "application/json");
            return;
        }
        catch (const std::exception& e) {
            // キーが存在しない場合など
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
            return;
        }
    });

    // ─── SSR HTMLページ (CSSを<link>タグで参照) ───
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <title>cpp-httplib SSR</title>
    <!-- CSSはサーバーから /style.css として配信 -->
    <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4"></script>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
<!-- 
    <h1>Hello from , cpp-httplib SSR!</h1>
    <p>このページはサーバーサイドでレンダリングされています。</p>
-->
    <div id="app"></div>
    <script type="module" src="/client.js"></script>
</body>
</html>)";
        res.set_content(html, "text/html; charset=utf-8");
    });
    // ─── CSSファイルの配信 ───
    svr.Get("/style.css", [](const httplib::Request&, httplib::Response& res) {
        std::string css = readFile("static/style.css");
        if (css.empty()) {
            res.status = 404;
            res.set_content("CSS not found", "text/plain");
            return;
        }
        res.set_content(css, "text/css; charset=utf-8");
    });
    svr.Get("/client.js", [](const httplib::Request&, httplib::Response& res) {
        std::string css = readFile("static/client.js");
        if (css.empty()) {
            res.status = 404;
            res.set_content("CSS not found", "text/plain");
            return;
        }
        res.set_content(css, "application/javascript");
    });

    svr.Post("/api/rag_search", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
       // 1. Content-Typeの確認
        if (req.get_header_value("Content-Type") != "application/json") {
            res.status = 400;
            res.set_content("Expected application/json", "text/plain");
            return;
        }
        try {
            json j = json::parse(req.body);
            std::string input = j.at("input").get<std::string>();
            std::cout << "input=" << input << "\n";

            std::string reply = rag_search(input);
            SearchResponse resp3;
            resp3.ret = "OK";
            resp3.text = reply;
            json j4 = resp3;
            std::string json_str4 = j4.dump();
            res.status = 200;
            res.set_content(json_str4, "application/json");
            return;
        }
        catch (const std::exception& e) {
            // キーが存在しない場合など
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
            return;
        }
    });

    // ── 起動 ────────────────────────────────
    int port_no = 8000;
    std::cout << "TODO Server running on http://localhost:8000\n";

    svr.listen("0.0.0.0", port_no);
    return 0;
}
