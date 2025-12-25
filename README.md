# QEstoqueLoja

## Descrição

Software simples ERP para controlar o estoque de lojas pequenas.

Funções:
- **emissão, cancelamento e devolução de nota fiscal eletrônica (NF-e)
e NFC-e para regime simples nacional**
- Pesquisa de preço, registro de vendas
- Cadastro de produtos
- Relatório de lucro e valor vendido
- PDV
- Código de barras
- Vendas a prazo
- Gráficos e relatórios
- Impressão de etiquetas
- Orçamento
- Importação de produtos através de notas fiscais
- Envio de e-mail de notas fiscais
- E mais.

É feito para ser fácil de instalar e usar, sem necessidade de
servidores ou dependencias, atualmente, funciona em apenas um
computador, sem sincronização de dados. Apenas instale um executável e
está pronto.

Nós entendemos que os programas atuais de ERP são muito complexos
tentando cobrir todas as milhões de situações fiscais e financeiras de
todos os comerciantes ao mesmo tempo, o que faz com que os comercios
pequenos tenham dificuldade de integrar o sistema as suas necessidades
e acabam não usando 99% do que os programas oferecem. O QEstoqueLoja
tenta mudar essa situação com foco na automatização de tarefas e
simplicidade.

Feito em Qt framework e SQLite para o banco de dados.

## Instalação

Disponibilizamos instaladores para Windows e Linux (Debian) na página
de [releases do
repositório](https://github.com/GabR36/QEstoqueLoja/releases).

## Compilação

### Dependências

- cmake,
- qt6-declarative-dev,
- qt6-charts-dev,
- qt6-base-dev,
- qt6-tools-dev,
- libzint-dev,
- libquazip1-qt6-dev
- [QtRpt](https://qtrpt.sourceforge.io/) *versão biblioteca e sem  barcode
- [ACBrNFe](https://svn.code.sf.net/p/acbr/code/trunk2/) *compilada com Qt
- [ACBrConsultaCNPJConsoleMT](https://svn.code.sf.net/p/acbr/code/trunk2/)
- [ACBrMailConsoleMT](https://svn.code.sf.net/p/acbr/code/trunk2/)

### Cmake

```
mkdir build
cmake -S . -B build
cmake --build build
```

## Motivações e Futuro

O objetivo desse programa é atender as necessidades que vejo no dia a
dia como funcionario de uma pequena loja de produtos
diversos. Primeiro quero substituir as funções existentes em outros
programas ja usados, para, em seguida, colocar funções adicionais que
realmente possam tornar o trabalho mais eficiente.  Contribuiçoes
serão adicionadas se não interferirem nos objetivos ou irem de acordo
com os objetivos propostos.

O "Paradigma de programação" usado é o de "fazer as coisas
funcionarem", se algo estiver funcionando ele será aceito. A ideia é
que, se é algo bom, é adicionado, o código pode ser melhorado com o
tempo. Seguindo os ideiais da metodologia Ágil.

## Capturas de Tela

![Tela Principal](/Imagens/capturaPrincipal.png)

![Tela Vendas](/Imagens/capturaVendas.png)

![Tela Venda](/Imagens/capturaVenda.png)

![Tela Venda](/Imagens/capturaPagamento.png)

![Tela Relatórios](/Imagens/capturaGrafico.png)
