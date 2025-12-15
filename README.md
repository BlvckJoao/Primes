# LPII – Programação Concorrente (2025.2)

## Exercício Prático 001 — Contagem de Números Primos

**Aluno:** João Pedro
**Disciplina:** LPII – Programação Concorrente
**Professor:** Carlos Eduardo Batista

---

## 1. Objetivo

O objetivo deste trabalho é implementar e comparar duas versões de um programa que conta quantos números primos existem no intervalo **[2, N]**:

* Uma versão **sequencial** (baseline)
* Uma versão **concorrente**, utilizando **processos POSIX** e **IPC**

  * Comunicação via **pipes**
  * Comunicação via **memória compartilhada (shared memory)**

Além da implementação, o trabalho envolve a **medição de desempenho**, **consumo de recursos** e a **análise dos resultados observados**.

---

## 2. Compilação

O código foi desenvolvido em C e pode ser compilado com `gcc`:

```bash
gcc -O2 -Wall -Wextra exercicio.c -o exercicio -lm
```

> O uso de `-O2` melhora o desempenho, especialmente para valores grandes de `N`.

---

## 3. Execução

### Modo Sequencial

```bash
./exercicio seq N
```

Exemplo:

```bash
./exercicio seq 5000000
```

### Modo Paralelo

```bash
./exercicio par N P pipe
./exercicio par N P shm
```

Onde:

* `N` é o limite superior do intervalo
* `P` é o número de processos worker
* `pipe` ou `shm` define o mecanismo de IPC

Exemplos:

```bash
./exercicio par 5000000 4 pipe
./exercicio par 5000000 4 shm
```

---

## 4. Estrutura do Código

### 4.1 Teste de Primalidade

A função `is_prime` implementa um método básico e honesto:

* Trata casos especiais (`n < 2`, `n == 2`)
* Elimina números pares
* Testa divisores ímpares até ( \sqrt{n} )

Esse método atende exatamente ao que é exigido no enunciado.

---

### 4.2 Versão Sequencial

A função `count_primes_seq` percorre o intervalo `[2, N]` e conta os primos utilizando `is_prime`.

Essa versão serve como **baseline de corretude e desempenho** para comparação com a versão concorrente.

---

### 4.3 Versão Concorrente — Modelo Master/Worker

No modo paralelo, o programa utiliza o seguinte modelo:

* Um **processo master** divide o intervalo `[2, N]` em `P` subintervalos
* O master cria `P` processos **worker** com `fork()`
* Cada worker conta os primos em seu subintervalo
* O master agrega os resultados e imprime a saída final

A divisão do intervalo é feita pela função `partition_interval`, garantindo:

* Cobertura completa do intervalo
* Ausência de sobreposição
* Balanceamento aproximado da carga

---

### 4.4 IPC via Shared Memory (SHM)

* O master cria um array compartilhado usando `mmap` com `MAP_SHARED | MAP_ANONYMOUS`
* Cada worker escreve sua contagem parcial em uma posição exclusiva do array
* O master aguarda todos os processos (`waitpid`) e soma os valores

**Vantagens:**

* Menor overhead
* Menos chamadas de sistema
* Melhor desempenho geral

---

### 4.5 IPC via Pipes

* O master cria um pipe dedicado para cada worker
* Cada worker escreve sua contagem parcial no pipe
* O master lê os valores e soma

Cuidados importantes implementados:

* Fechamento correto das pontas de pipe não utilizadas
* Evita deadlocks e processos zumbis

**Desvantagens:**

* Maior overhead devido à comunicação via kernel
* Maior tempo em modo sistema (sys time)

---

## 5. Medição de Desempenho

### 5.1 Tempo Interno

O tempo de execução é medido internamente com:

```c
clock_gettime(CLOCK_MONOTONIC, ...)
```

O tempo reportado corresponde ao **tempo total de execução**, em milissegundos.

Cada configuração foi executada **3 vezes**, e o **menor tempo** foi considerado como valor representativo.

---

### 5.2 Consumo de Recursos

O consumo de recursos foi medido externamente utilizando:

```bash
/usr/bin/time -v ./exercicio ...
```

Os seguintes dados foram coletados:

* User time (seconds)
* System time (seconds)
* Elapsed (wall clock) time
* Maximum resident set size (kbytes)

---

## 6. Resultados Experimentais

### Ambiente

* Sistema: Linux
* Arquitetura: x86_64
* Número de núcleos: 4
* Compilador: GCC

### Execuções (N = 5.000.000)

| Modo | Processos | IPC  | Primos | Tempo (ms) |
| ---- | --------- | ---- | ------ | ---------- |
| Seq  | –         | –    | 348513 | 1016.870   |
| Par  | 4         | pipe | 348513 | 344.015    |
| Par  | 4         | shm  | 348513 | 345.665    |

---

## 7. Análise dos Resultados

### 7.1 Speedup

* O speedup com 4 processos foi de aproximadamente **3×** em relação à versão sequencial.
* O ganho não é linear devido a:

  * Overhead de criação de processos (`fork`)
  * Comunicação entre processos
  * Limitações do hardware (número de núcleos físicos)

---

### 7.2 Pipes vs Shared Memory

* A versão com **pipes** apresentou maior tempo em modo sistema, devido às chamadas de sistema necessárias para `read` e `write`.
* A versão com **shared memory** apresentou menor overhead, pois os processos escrevem diretamente na memória compartilhada.

Apesar disso, os tempos totais foram semelhantes, pois o custo dominante é o teste de primalidade.

---

### 7.3 Consumo de Memória

* O consumo de memória não variou significativamente entre as versões
* A versão paralela utiliza mais memória devido à criação de múltiplos processos, mas o impacto é pequeno

---

## 8. Conclusão

O programa atende integralmente às especificações do exercício, implementando corretamente as versões sequencial e concorrente.

Os resultados demonstram ganhos significativos de desempenho com paralelismo, além de evidenciar as diferenças práticas entre IPC via pipes e memória compartilhada.

O uso de processos POSIX, divisão correta do intervalo e tratamento adequado de IPC garantem corretude, robustez e boa performance.

---

## 9. Observações Finais

* O código evita processos zumbis
* Não há vazamento de memória
* A saída é facilmente comparável entre execuções
* O projeto está pronto para avaliação
