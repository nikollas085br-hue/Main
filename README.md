# M5Stack Cardputer ADV — Central de Estudos

Este pacote é **somente o firmware/projeto para GitHub/PlatformIO**.

## Importante
- Não existe pasta `data/` neste projeto.
- O conteúdo de estudos fica no pacote separado do SD.
- O firmware fornece ferramentas; o SD fornece conteúdo.
- A navegação de menus não depende de Fn: as teclas físicas marcadas com as setas (;, , . /) são interpretadas diretamente como ↑ ← ↓ → fora dos campos de texto. WASD também funciona como alternativa direta. Dentro da calculadora, pontuação continua sendo digitável.
- `M5Cardputer` é fixado em 1.1.1 para manter a API usada pelo projeto.
- O botão/entrada de π é tratado como token matemático `pi`, e não como o texto `3.14`.

## GitHub Actions
Faça upload deste diretório para um repositório GitHub. A ação em `.github/workflows/build.yml` compila automaticamente e publica `firmware.bin` como artefato.

## SD
O SD é entregue em outro ZIP. Copie o conteúdo dele para o cartão microSD. O firmware cria/usa `/anotacoes` e `/conteudos`.

## Módulos
- Matemática: expressão, π, e, potência `^`, raiz, trigonometria, log, funções, equação linear/quadrática e base para exponenciais.
- Física: módulo de “Suposição de cálculo” para escolher a grandeza procurada e relacioná-la aos dados fornecidos.
- Química: tabela de elementos, distribuição eletrônica e classificação de ligações.
- Tabela periódica: consulta interativa.
- Biologia: conteúdo principalmente no SD.
- Arquivos: armazenamento e consulta de materiais.
- Conexão: AP Wi‑Fi `Cardputer-Estudos`, página em `192.168.4.1` para envio de arquivos.
- Anotações: espaço separado para textos.

## Observação sobre PDF/PPT/PPTX
O sistema aceita esses arquivos para **transferência e armazenamento no SD**. Renderizar qualquer PDF/PPTX moderno integralmente no ESP32-S3 seria outro subsistema; o projeto não finge ter um leitor de desktop completo.

## Rede
Ao iniciar, o firmware cria o ponto de acesso `Cardputer-Estudos`. A página de upload fica em `http://192.168.4.1/`.
