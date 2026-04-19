# rag_7sql

 Version: 0.9.1

 date    : 2026/04/18

 update :

***

C++ windows , Server RAG + SQLite

* model: gemma-4-E2B-it-Q4_K_S.gguf
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* llama.cpp , llama-server
* Visual studio Community 2026 use

***
### vector data add

https://github.com/kuc-arc-f/cpp_5ex/tree/main/rag_6sql

***
### related

https://www.sqlite.org/download.html

* sqlite-amalgamation-*.zip , download
* sqlite3.h , sqlite3.c

***
### setup
* llama-server start
* port 8080: Qwen3-Embedding-0.6B
* port 8090: gemma-4-E2B

```
#Qwen3-Embedding-0.6B

/home/user123/llama-server -m /var/lm_data/Qwen3-Embedding-0.6B-Q8_0.gguf --embedding  -c 1024 --port 8080

#gemma-4-E2B

/usr/local/llama-b8642/llama-server -m /var/lm_data/unsloth/gemma-4-E2B-it-Q4_K_S.gguf \
 --chat-template-kwargs '{"enable_thinking": false}' --port 8090 

```

***
### related

https://huggingface.co/unsloth/gemma-4-E2B-it-GGUF

https://huggingface.co/Qwen/Qwen3-Embedding-0.6B-GGUF


***
* front-build , React

```
npm i
npm run build
```

***

* start
```
x64\Debug\rag_7sql.exe
```

***



