# 🐍 Documentação do Backend (Python/Flask)

O backend desta estação meteorológica foi construído seguindo o princípio de **Separação de Responsabilidades** (Separation of Concerns). Em vez de um único arquivo monolítico, o sistema foi dividido em três módulos lógicos para facilitar a manutenção, escalabilidade e testes.

Abaixo está a explicação técnica de cada módulo.

## Estrutura dos Arquivos

### 1. `app.py` (O Controlador / API Gateway)
Este é o ponto de entrada da aplicação. Ele é responsável **apenas** por gerenciar as rotas HTTP e orquestrar a comunicação entre o usuário, o banco de dados e as tarefas de fundo.

* **Função Principal:** Inicializar o servidor Flask e o Agendador (Scheduler).
* **Rotas Definidas:**
    * `POST /dados`: Endpoint que recebe o JSON do ESP32. Ele aciona o `tasks.py` para pegar o vento e o `database.py` para salvar tudo.
    * `GET /api/brutos`: Endpoint JSON usado pelo JavaScript do frontend para atualizar gráficos em tempo real.
    * `GET /`: Rota principal que renderiza o dashboard HTML, injetando as variáveis de previsão de chuva.

### 2. `tasks.py` (Regras de Negócio e Integrações)
Este arquivo contém a "inteligência" do sistema. Ele isola a lógica matemática e as dependências externas (APIs) do restante do código.

* **Integração Externa:** Contém a função `obter_vento_externo()` que consome a API da **OpenWeatherMap**. Inclui tratamento de erros e timeouts.
* **Machine Learning:** Implementa o algoritmo de **Regressão Logística** (função `calcular_probabilidade_chuva`). Os pesos ($w$) treinados previamente são aplicados aqui para calcular a chance de chuva.
* **Automação:** Define o *Job* que roda a cada hora para calcular a média dos sensores e compactar os dados históricos.

### 3. `database.py` (Camada de Persistência)
Responsável exclusivamente pela comunicação com o banco de dados **SQLite**. Nenhum comando SQL é executado fora deste arquivo.

* **Abstração:** As outras partes do código não precisam saber SQL. Elas apenas chamam funções como `inserir_medida()` ou `pegar_historico()`.
* **Modelagem de Dados:**
    * Tabela `medidas`: Armazena dados brutos a cada envio do ESP32.
    * Tabela `resumos`: Armazena os dados consolidados (médias horárias) para otimizar o armazenamento e a geração de gráficos de longo prazo.

---

## Fluxo de Dados (Data Flow)

Entenda como os arquivos interagem quando o ESP32 envia um dado:

1.  **ESP32** envia POST para `app.py`.
2.  `app.py` pede para `tasks.py` buscar o vento atual na internet.
3.  `app.py` recebe o vento e manda tudo para `database.py`.
4.  `database.py` executa o `INSERT` no arquivo `.db`.
5.  O servidor responde "OK" para o ESP32.

---

## Dependências

As bibliotecas Python necessárias para rodar este backend estão listadas abaixo (ou no `requirements.txt`):

* **Flask:** Framework web.
* **APScheduler:** Para rodar o robô de médias a cada hora.
* **Requests:** Para comunicação com a API de meteorologia externa.
