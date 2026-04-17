# rag_4pg

 Version: 0.9.1

 date    : 2026/04/15

 update :

***

C++ windows , RAG Search + PGvector DB

* model: gemma-4-E2B-it-Q4_K_S.gguf
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* llama.cpp , llama-server
* Visual studio Community 2026 use

***
### vector data add

https://github.com/kuc-arc-f/cpp_4ex/tree/main/rag_1

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
* front build React
```
npm i
npm run build
```
***
* start

```
x64\Debug\rag_4pg.exe
```
***
### blog

https://zenn.dev/knaka0209/scraps/5ffa8e1edc8a05

