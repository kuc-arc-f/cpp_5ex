# rag_2

 Version: 0.9.1

 date    : 2026/04/15

 update :

***

C++ windows , RAG Search + Qdrant DB

* model: gemma-4-E2B-it-Q4_K_S.gguf
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* llama.cpp , llama-server
* Visual studio Community 2026 use

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

https://huggingface.co/unsloth/Qwen3.5-2B-GGUF

https://huggingface.co/Qwen/Qwen3-Embedding-0.6B-GGUF


***
* init
```
x64\Debug\rag_2.exe init
```

***
* vector data add

```
x64\Debug\rag_2.exe embed
```

***
* search
```
x64\Debug\rag_2.exe search hello
```
***
### blog


